/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "BuildRevision.h"
#include "ConfigurationSettingsImp.h"
#include "DataDescriptor.h"
#include "DateTimeImp.h"
#include "DynamicObjectAdapter.h"
#include "Filename.h"
#include "ImportDescriptorImp.h"
#include "MessageLogMgrImp.h"
#include "ModelServices.h"
#include "MruFile.h"
#include "ObjectResource.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "StringUtilities.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include "yaml.h"

#include <iostream>
#include <fstream>
#if defined(WIN_API)
#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

using namespace std;

ConfigurationSettingsImp* ConfigurationSettingsImp::spInstance = NULL;
bool ConfigurationSettingsImp::mDestroyed = false;

ConfigurationSettingsImp* ConfigurationSettingsImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use ConfigurationSettings after "
            "destroying it.");
      }
      spInstance = new ConfigurationSettingsImp;
   }

   return spInstance;
}

void ConfigurationSettingsImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy ConfigurationSettings after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

ConfigurationSettingsImp::ConfigurationSettingsImp() :
   mCreator(APP_SPONSOR),
   mProduct(APP_NAME),
   mVersion(APP_VERSION_NUMBER),
   mpReleaseDate(new DateTimeImp()),
#if defined(_DEBUG)
   mProductionRelease(false),
#else
   mProductionRelease(APP_IS_PRODUCTION_RELEASE),
#endif
   mReleaseType(RT_NORMAL),
   mNeedToLoadMruFiles(true),
   mUserDocs(locateUserDocs()),
   mIsInitialized(false)
{
   mpReleaseDate->set(APP_RELEASE_DATE_YEAR, APP_RELEASE_DATE_MONTH, APP_RELEASE_DATE_DAY);

   string errorMessage;
   if (!parseDeploymentFile(errorMessage, mDeploymentDebugMsg))
   {
      mIsInitialized = false;
      mInitializationErrorMsg = errorMessage;
      return;
   }

   Service<ApplicationServices> pApp;
   pApp->attach(SIGNAL_NAME(ApplicationServices, ApplicationClosed),
      Slot(this, &ConfigurationSettingsImp::applicationClosed));

   // Load the values into config settings
   string settingErrorMsg;
   if (!loadSettings(settingErrorMsg))
   {
      mIsInitialized = false;
      mInitializationErrorMsg = settingErrorMsg + " The application will be shut down.\n\n" + mDeploymentDebugMsg;
      return;
   }

   // Reset Any Error Codes and mark as initialized
   mIsInitialized = true;
   mInitializationErrorMsg = "";
}

void ConfigurationSettingsImp::validateInitialization()
{
   //check for support files
   string supportFilesPath;
   const Filename* pFilename = getSettingSupportFilesPath();
   if (pFilename != NULL)
   {
      supportFilesPath = pFilename->getFullPathAndName();
   }
   else
   {
      mIsInitialized = false;
      mInitializationErrorMsg = "The Support Files setting could not be found. "
         "The application will be shut down.\n\n" + mDeploymentDebugMsg;
      return;
   }
   if (!QFile::exists(QString::fromStdString(supportFilesPath)))
   {
      mIsInitialized = false;
      mInitializationErrorMsg = "The Support Files directory of " + supportFilesPath + 
         " does not exist. The application will be shut down.\n\n" + mDeploymentDebugMsg;
      return;
   }

   mReleaseType = getSettingReleaseType();

   // Set Internal Path -  Note that this only affects the environment
   // variable of the current process. 
   const Filename* pInternalPath = getSettingInternalPath();
   if (pInternalPath != NULL)
   {
      string internalPath = pInternalPath->getFullPathAndName();
      if (!internalPath.empty())
      {
         char* pPath = getenv("PATH");

         string path = "PATH=";
         if (pPath != NULL)
         {
            path = path + string(pPath);
         }

   #if defined(WIN_API)
         internalPath = path + ";" + internalPath;
         _putenv(internalPath.c_str());
   #else
         internalPath = path + ":" + internalPath;
         putenv(const_cast<char*>(internalPath.c_str()));
   #endif
      }
   }

   if (ArgumentList::instance()->getOption("debugDeployment") == "true")
   {
      mIsInitialized = true;
      mInitializationErrorMsg = mDeploymentDebugMsg;
      return;
   }

   // Reset Any Error Codes and mark as initialized
   mIsInitialized = true;
   mInitializationErrorMsg = "";
}

ConfigurationSettingsImp::~ConfigurationSettingsImp()
{
   notify(SIGNAL_NAME(Subject, Deleted));

   Service<ApplicationServices> pApp;
   pApp->detach(SIGNAL_NAME(ApplicationServices, ApplicationClosed),
      Slot(this, &ConfigurationSettingsImp::applicationClosed));

   // Delete the data descriptors in the MRU file vector
   vector<MruFile>::iterator iter;
   for (iter = mMruFiles.begin(); iter != mMruFiles.end(); ++iter)
   {
      MruFile mruFile = *iter;

      vector<ImportDescriptor*>::iterator iter2;
      for (iter2 = mruFile.mDescriptors.begin(); iter2 != mruFile.mDescriptors.end(); ++iter2)
      {
         ImportDescriptorImp* pImportDescriptor = dynamic_cast<ImportDescriptorImp*>(*iter2);
         if (pImportDescriptor != NULL)
         {
            delete pImportDescriptor;
         }
      }
   }
}

void ConfigurationSettingsImp::initDeploymentValues()
{
   QDir binaryDir(QCoreApplication::applicationDirPath());
   binaryDir.makeAbsolute();
   QString binaryDirPath = binaryDir.canonicalPath();
   QString newHome = binaryDirPath + QString::fromStdString("/../");
   QDir homeDir(newHome);
   homeDir.makeAbsolute();
   mHomePath = homeDir.canonicalPath().toStdString();

   mPlugInPath = mHomePath + SLASH + "PlugIns";
   {
      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(getPlugInPath());
      mPlugInPathVariant = DataVariant(*(pFilename.get()));
   }
   mAdditionalDefaultDir = "";
   mUserConfigPath = "";

   QString userSettingsPath;
   QString appNamePath;
#if defined(WIN_API)
   char path[MAX_PATH];
   HRESULT retValue = SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
   if (SUCCEEDED(retValue))
   {
      userSettingsPath = QString::fromAscii(path);
   }
   appNamePath = "Opticks";
#else
   char* pPath = getenv("HOME");
   if (pPath != NULL)
   {
      userSettingsPath = QString::fromAscii(pPath);
   }
   appNamePath = ".opticks";
#endif
   if (!userSettingsPath.isEmpty())
   {
      userSettingsPath.replace("\\", "/");
      QDir userSettingsDir(userSettingsPath);
      if (userSettingsDir.exists())
      {
         QString appUserSettingsPath = userSettingsDir.absoluteFilePath(appNamePath);
         QDir appSettingsDir(appUserSettingsPath);
         if (!appSettingsDir.exists())
         {
            userSettingsDir.mkdir(appNamePath);
         }
         mUserConfigPath = appSettingsDir.absolutePath().toStdString();
      }
   }
}

string expandDeploymentPathVar(const string& originalValue, const string& relativeRoot, string& errorMessage)
{
   errorMessage.clear();
   vector<string> ignoredExpansions;
   ignoredExpansions.push_back("C");
   ignoredExpansions.push_back("V");
   string expandedStr = StringUtilities::expandVariables(originalValue, ignoredExpansions);
   QString expandedPath = QString::fromStdString(expandedStr).replace("\\", "/");
   if (QDir::isRelativePath(expandedPath))
   {
      expandedPath = QString::fromStdString(relativeRoot) + "/" + expandedPath;
   }
   QFileInfo dirFileInfo(expandedPath);
   if (!dirFileInfo.exists() || !dirFileInfo.isDir())
   {
      errorMessage = QString("The provided directory %1 "
         "does not exist.").arg(expandedPath).toStdString();
   }
   QDir dirInfo(expandedPath);
   return dirInfo.canonicalPath().toStdString();
}

bool ConfigurationSettingsImp::parseDeploymentFile(string& errorMessage, string& deploymentDebugMsg)
{
   errorMessage.clear();

   initDeploymentValues();

   //determine which .dep file that we are loading.
   bool depMustExist = false;
   string deploymentFile = ArgumentList::instance()->getOption("deployment");
   if (deploymentFile.empty())
   {
      QDir binaryDir(QCoreApplication::applicationDirPath());
      binaryDir.makeAbsolute();
      QString binaryDirPath = binaryDir.canonicalPath();
      deploymentFile = binaryDirPath.toStdString() + SLASH + "opticks.dep";
   }
   else
   {
      depMustExist = true;
   }

   //if provided as arg, .dep file must exist
   bool needToParse = true;
   QFileInfo depFileInfo(QString::fromStdString(deploymentFile));
   if (!depFileInfo.exists() || !depFileInfo.isFile())
   {
      if (depMustExist)
      {
         errorMessage = QString("The %1 deployment file does not exist.  "
            "The application will be shut down.").arg(depFileInfo.absoluteFilePath()).toStdString();
         return false;
      }
      else
      {
         needToParse = false; //the .dep doesn't exist, but it doesn't need to
         //and all the hard-coded defaults have already been configured.
      }
   }

   //parse data out of .dep file here.
   map<string, string> parsedDepMap;
   bool appHomePathReadFromDep = false;
   bool plugInPathReadFromDep = false;
   bool userConfigPathReadFromDep = false;
   bool addDefaultsPathReadFromDep = false;
   if (needToParse)
   {
      try
      {
         ifstream depStream(deploymentFile.c_str());
         YAML::Parser parser(depStream);
         YAML::Node doc;
         if (!parser)
         {
            errorMessage = QString("Unknown error while parsing %1 deployment file.  "
               "The application will be shut down.").arg(depFileInfo.absoluteFilePath()).toStdString();
            return false;
         }
         parser.GetNextDocument(doc);
         string tag = doc.GetTag();
         if (tag != "!depV1")
         {
            errorMessage = QString("Error while parsing %1 deployment file.  The initial YAML map "
               "must be tagged with a local type of depV1, i.e. !depV1.  The application will be "
               "shut down.").arg(depFileInfo.absoluteFilePath()).toStdString();
            return false;
         }
         if (doc.GetType() != YAML::CT_MAP || doc["deployment"].GetType() != YAML::CT_MAP)
         {
            errorMessage = QString("Error while parsing %1 deployment file. "
               "The deployment file must contain a YAML map with a key of 'deployment'. "
               "The application will be shut down.").arg(depFileInfo.absoluteFilePath()).toStdString();
            return false;
         }
         const YAML::Node& depMap = doc["deployment"];
         for (YAML::Iterator iter = depMap.begin(); iter != depMap.end(); ++iter)
         {
            if (iter.second().GetType() == YAML::CT_SCALAR)
            {
               string key;
               string value;
               iter.first() >> key;
               iter.second() >> value;
               parsedDepMap.insert(make_pair(key, value));
            }
         }
         depStream.close();
      }
      catch (YAML::ParserException& ex)
      {
         errorMessage = QString("Error while parsing %1 deployment file. "
            "Found '%2' problem at line %3 and column %4. "
            "The application will be shut down.").arg(depFileInfo.absoluteFilePath()).arg(
            QString::fromStdString(ex.msg)).arg(ex.line).arg(ex.column).toStdString();
         return false;
      }
      catch (YAML::Exception&)
      {
         errorMessage = QString("Unknown error while parsing %1 deployment file.  "
            "The application will be shut down.").arg(depFileInfo.absoluteFilePath()).toStdString();
         return false;
      }

      string pathErrorMsg;
      string relativeRoot = depFileInfo.absolutePath().toStdString();
      //set app home first from the .dep file
      if (!parsedDepMap["AppHomePath"].empty())
      {
         appHomePathReadFromDep = true;
         mHomePath = expandDeploymentPathVar(parsedDepMap["AppHomePath"], relativeRoot, pathErrorMsg);
         if (!pathErrorMsg.empty())
         {
            errorMessage = QString("Error while parsing %1 deployment file. "
               "The value for AppHomePath is invalid. %2"
               "The application will be shut "
               "down.").arg(depFileInfo.absoluteFilePath()).arg(QString::fromStdString(pathErrorMsg)).toStdString();
            return false;
         }
      }
      //allow the other three variables to reference each other in any order.
      if (!parsedDepMap["PlugInPath"].empty())
      {
         plugInPathReadFromDep = true;
         mPlugInPath = expandDeploymentPathVar(parsedDepMap["PlugInPath"], relativeRoot, pathErrorMsg);
         if (!pathErrorMsg.empty())
         {
            errorMessage = QString("Error while parsing %1 deployment file. "
               "The value for PlugInPath is invalid. %2"
               "The application will be shut "
               "down").arg(depFileInfo.absoluteFilePath()).arg(QString::fromStdString(pathErrorMsg)).toStdString();
            return false;
         }
         FactoryResource<Filename> pFilename;
         pFilename->setFullPathAndName(getPlugInPath());
         mPlugInPathVariant = DataVariant(*(pFilename.get()));
      }
      if (!parsedDepMap["UserConfigPath"].empty())
      {
         userConfigPathReadFromDep = true;
         mUserConfigPath = expandDeploymentPathVar(parsedDepMap["UserConfigPath"], relativeRoot, pathErrorMsg);
         if (!pathErrorMsg.empty())
         {
            errorMessage = QString("Error while parsing %1 deployment file. "
               "The value for UserConfigPath is invalid. %2"
               "The application will be shut "
               "down").arg(depFileInfo.absoluteFilePath()).arg(QString::fromStdString(pathErrorMsg)).toStdString();
            return false;
         }
      }
      if (!parsedDepMap["AdditionalDefaultPath"].empty())
      {
         addDefaultsPathReadFromDep = true;
         mAdditionalDefaultDir = expandDeploymentPathVar(parsedDepMap["AdditionalDefaultPath"], relativeRoot,
            pathErrorMsg);
         if (!pathErrorMsg.empty())
         {
            errorMessage = QString("Error while parsing %1 deployment file. "
               "The value for AdditionalDefaultPath is invalid. %2"
               "The application will be shut "
               "down").arg(depFileInfo.absoluteFilePath()).arg(QString::fromStdString(pathErrorMsg)).toStdString();
            return false;
         }
      }
   }

   QString deploymentInfoMsg;
   deploymentInfoMsg += QString("The DefaultSettings will be loaded from the following directories:\n");
   deploymentInfoMsg += QString("     1) [AppHomePath]/DefaultSettings\n");
   deploymentInfoMsg += QString("     2) [AdditionalDefaultPath]\n");
   deploymentInfoMsg += QString("The plug-ins will be loaded from the [PlugInPath] directory\n");
   deploymentInfoMsg += QString("The user settings file (%1) will be "
      "loaded from the [UserConfigPath] directory\n\n").arg(QString::fromStdString(getUserSettingsFileName()));
   if (depMustExist || needToParse)
   {
      if (depMustExist)
      {
         deploymentInfoMsg += QString("The settings below were loaded from the %1 "
            "deployment file that was specified using the /deployment: "
            "command-line argument.\n").arg(depFileInfo.absoluteFilePath());

      }
      else if (needToParse)
      {
         deploymentInfoMsg += QString("The settings below were loaded from the %1 "
            "deployment file because an opticks.dep file was present in the same directory "
            "as the Opticks executable and the /deployment: command-line argument "
            "was NOT used.\n").arg(depFileInfo.absoluteFilePath());
      }
      if (appHomePathReadFromDep)
      {
         deploymentInfoMsg += QString("     AppHomePath = %1\n").arg(QString::fromStdString(mHomePath));
      }
      if (plugInPathReadFromDep)
      {
         deploymentInfoMsg += QString("     PlugInPath = %1\n").arg(QString::fromStdString(mPlugInPath));
      }
      if (userConfigPathReadFromDep)
      {
         deploymentInfoMsg += QString("     UserConfigPath = %1\n").arg(QString::fromStdString(mUserConfigPath));
      }
      if (addDefaultsPathReadFromDep)
      {
         deploymentInfoMsg +=
            QString("     AdditionalDefaultPath = %1\n").arg(QString::fromStdString(mAdditionalDefaultDir));
      }
      deploymentInfoMsg += QString("\n");
   }
   else
   {
      deploymentInfoMsg += QString("All of the settings were resolved using the "
         "application defaults because the the /deployment: command-line argument "
         "was not used and an opticks.dep file was not present in the same directory "
         "as the Opticks executable.\n");
   }

   if (!appHomePathReadFromDep || !plugInPathReadFromDep || !userConfigPathReadFromDep || !addDefaultsPathReadFromDep)
   {
      deploymentInfoMsg += QString("The settings below were resolved using the application defaults.\n");
      if (!appHomePathReadFromDep)
      {
         deploymentInfoMsg += QString("     AppHomePath - This was defaulted as one directory "
            "up from the directory where the Opticks "
            "executable resides on the filesystem. In this case "
            "%1.\n").arg(QString::fromStdString(mHomePath));
      }
      if (!plugInPathReadFromDep)
      {
         deploymentInfoMsg += QString("     PlugInPath - This was defaulted as ..\\PlugIns from the "
            "directory where the Opticks executable resides on the filesystem. In this case "
            "%1\n").arg(QString::fromStdString(mPlugInPath));
      }
      if (!userConfigPathReadFromDep)
      {
         deploymentInfoMsg += QString("     UserConfigPath - This was defaulted as the users "
            "setting directory from the specific platform.  In this case "
            "%1\n").arg(QString::fromStdString(mUserConfigPath));
      }
      if (!addDefaultsPathReadFromDep)
      {
         deploymentInfoMsg += QString("     AdditionalDefaultPath - This was defaulted "
            "as empty, i.e. default settings "
            "will not be searched for in this directory.\n");
      }
   }
   deploymentDebugMsg = deploymentInfoMsg.toStdString();

   return true;
}

string ConfigurationSettingsImp::getHome() const
{
   return mHomePath;
}

string ConfigurationSettingsImp::getPlugInPath() const
{
   return mPlugInPath;
}

string ConfigurationSettingsImp::getUserDocs() const
{
   return mUserDocs;
}

string ConfigurationSettingsImp::getCreator() const
{
   return mCreator;
}

string ConfigurationSettingsImp::getProduct() const
{
   return mProduct;
}

string ConfigurationSettingsImp::getVersion() const
{
   return mVersion;
}

string ConfigurationSettingsImp::getBuildRevision() const
{
   return string(BUILD_REVISION);
}

string ConfigurationSettingsImp::getUserName() const
{
   const int BUFFER_SIZE = 8096;
   string username;
#if defined(WIN_API)
   char* pUserName = new char[BUFFER_SIZE];
   DWORD size = BUFFER_SIZE;

   if (GetUserName(pUserName, &size) != false)
   {
      username = pUserName;
   }

   delete [] pUserName;
#else
   struct passwd* userEntry = getpwuid(getuid());
   if (userEntry != NULL)
   {
      if (userEntry->pw_name != NULL)
      {
         username = userEntry->pw_name;
      }
   }
#endif
   return username;
}

string ConfigurationSettingsImp::getOperatingSystemName() const
{
   string os;
#if defined(WIN_API)
   os = "Windows";
#elif defined(SOLARIS)
   os = "Solaris";
#elif defined(LINUX)
   os = "Linux";
#else
   os = "Unkown"
#endif
   return os;
}

string ConfigurationSettingsImp::getArchitectureName() const
{
   string arch = "Unknown";
#if defined(WIN_API)
#if PTR_SIZE == 8
   arch = "x86-64";
#elif PTR_SIZE == 4
   arch = "x86-32";
#endif
#elif defined(SOLARIS)
#if PTR_SIZE == 8
   arch = "sparcv9";
#endif
#elif defined(LINUX)
#if PTR_SIZE == 8
   arch = "x86_64";
#endif
#endif
   return arch;
}

const DateTime* ConfigurationSettingsImp::getReleaseDate() const
{
   return dynamic_cast<DateTime*>(mpReleaseDate.get());
}

bool ConfigurationSettingsImp::isProductionRelease() const
{
   return mProductionRelease;
}

ReleaseType ConfigurationSettingsImp::getReleaseType() const
{
   return mReleaseType;
}

bool ConfigurationSettingsImp::isInitialized()
{
   return mIsInitialized;
}

const char* ConfigurationSettingsImp::getInitializationErrorMsg()
{
   if (mInitializationErrorMsg.empty())
   {
      return NULL;
   }

   const char* pErrorMsg = mInitializationErrorMsg.c_str();
   return pErrorMsg;
}

bool ConfigurationSettingsImp::adoptSetting(const std::string& key, DataVariant& var, bool setIfSame)
{
   return setSetting(key, var, setIfSame, true);
}

bool ConfigurationSettingsImp::setSetting(const string& key, DataVariant& var, bool setIfSame, bool adopt)
{
   bool same = false;

   if (!setIfSame)
   {
      DataVariant curValue = getSetting(key);
      try
      {
         //Need try-catch because DataVariant comparison can
         //throw UnsupportedOperation if the held type doesn't support comparision
         same = (var == curValue);
      }
      catch (DataVariant::UnsupportedOperation) {}
   }
   bool success = true;
   if (!same)
   {
      if (adopt)
      {
         success = mpUserSettings->adoptAttributeByPath(key, var);
      }
      else
      {
         success = mpUserSettings->setAttributeByPath(key, var);
      }
      if (success)
      {
         deleteSessionSetting(key);
         notify(SIGNAL_NAME(ConfigurationSettings, SettingModified), boost::any(key));
      }
   }
   return success;
}

const DataVariant& ConfigurationSettingsImp::getSetting(const string& key) const
{
   static DataVariant sEmpty;

   const DataVariant& sessionValue = mpSessionSettings->getAttributeByPath(key);
   if (sessionValue.isValid())
   {
      return sessionValue;
   }
   const DataVariant& userValue = mpUserSettings->getAttributeByPath(key);
   if (userValue.isValid())
   {
      return userValue;
   }
   const DataVariant& defaultValue = mpDefaultSettings->getAttributeByPath(key);
   if (defaultValue.isValid())
   {
      return defaultValue;
   }

   if (key == "FileLocations/PlugInPath")
   {
      return mPlugInPathVariant;
   }

   return sEmpty;
}

bool ConfigurationSettingsImp::isUserSetting(const string& key) const
{
   if (isSessionSetting(key))
   {
      return false;
   }
   const DataVariant& userValue = mpUserSettings->getAttributeByPath(key);
   return userValue.isValid();
}

bool ConfigurationSettingsImp::isSessionSetting(const string& key) const
{
   return mpSessionSettings->getAttributeByPath(key).isValid();
}

bool ConfigurationSettingsImp::isDefaultSetting(const string& key) const
{
   const DataVariant& defaultValue = mpDefaultSettings->getAttributeByPath(key);
   if (!defaultValue.isValid())
   {
      return false;
   }
   const DataVariant& userValue = mpUserSettings->getAttributeByPath(key);
   if (userValue.isValid())
   {
      return false;
   }
   const DataVariant& sessionValue = mpSessionSettings->getAttributeByPath(key);
   if (sessionValue.isValid())
   {
      return false;
   }
   return true;
}

void ConfigurationSettingsImp::deleteUserSetting(const string& key)
{
   bool success = mpUserSettings->removeAttributeByPath(key);
   if (success)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void ConfigurationSettingsImp::deleteSessionSetting(const string& key)
{
   bool success = mpSessionSettings->removeAttributeByPath(key);
   if (success)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool ConfigurationSettingsImp::adoptSessionSetting(const std::string& key, DataVariant& var)
{
   return setSessionSetting(key, var, true);
}

bool ConfigurationSettingsImp::setSessionSetting(const string& key, DataVariant& var, bool adopt)
{
   bool success = false;
   if (adopt)
   {
      success = mpSessionSettings->adoptAttributeByPath(key, var);
   }
   else
   {
      success = mpSessionSettings->setAttributeByPath(key, var);
   }
   if (success)
   {
      notify(SIGNAL_NAME(ConfigurationSettings, SettingModified), boost::any(key));
   }
   return success;
}

void ConfigurationSettingsImp::copySetting(const string& key, DynamicObject* pObject) const
{
   const DataVariant& dv = getSetting(key);
   if (dv.isValid())
   {
      pObject->setAttributeByPath(key, dv);
   }
}

void ConfigurationSettingsImp::setMruFiles(const vector<MruFile>& mruFiles)
{
   mMruFiles = mruFiles;
}

void ConfigurationSettingsImp::removeMruFile(const string& filename)
{
   if (filename.empty() == true)
   {
      return;
   }

   // Make sure the MRU files have been deserialized before trying to remove the file
   deserializeMruFiles();

   Service<ModelServices> pModel;

   QString strFilename = QString::fromStdString(filename).toLower();
   strFilename.replace(QRegExp("\\\\"), "/");

   for (vector<MruFile>::iterator iter = mMruFiles.begin(); iter != mMruFiles.end(); ++iter)
   {
      MruFile mruFile = *iter;

      QString strMruFilename = QString::fromStdString(mruFile.mName).toLower();
      strMruFilename.replace(QRegExp("\\\\"), "/");

      if (strMruFilename == strFilename)
      {
         // Destroy the import descriptors in this MRU file
         vector<ImportDescriptor*>::iterator descriptorIter;
         for (descriptorIter = mruFile.mDescriptors.begin();
              descriptorIter != mruFile.mDescriptors.end();
              ++descriptorIter)
         {
            ImportDescriptor* pImportDescriptor = *descriptorIter;
            if (pImportDescriptor != NULL)
            {
               pModel->destroyImportDescriptor(pImportDescriptor);
            }
         }

         mMruFiles.erase(iter);
         break;
      }
   }
}

const vector<MruFile>& ConfigurationSettingsImp::getMruFiles() const
{
   const_cast<ConfigurationSettingsImp*>(this)->deserializeMruFiles();
   return mMruFiles;
}

void ConfigurationSettingsImp::deserializeMruFiles()
{
   if (!mNeedToLoadMruFiles)
   {
      return;
   }

   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(getUserSettingsFilePath());
   XmlReader xmlReader(NULL, false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDomDoc(NULL);
   pDomDoc = xmlReader.parse(pFilename.get());

   if (pDomDoc == NULL)
   {
      mNeedToLoadMruFiles = false;
      return;
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pConfList(NULL);
   pConfList = pDomDoc->getElementsByTagName(X("ConfigurationSettings"));
   if (pConfList == NULL || pConfList->getLength() != 1)
   {
      mNeedToLoadMruFiles = false;
      return;
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pSettingsNodes(NULL);
   pSettingsNodes = (pConfList->item(0))->getChildNodes();
   for (unsigned int i = 0; i < pSettingsNodes->getLength(); i++)
   {
      string name;
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pSettingsNode = pSettingsNodes->item(i);
      int nodeType = pSettingsNode->getNodeType();
      string elementName = A(pSettingsNode->getNodeName());
      if ((nodeType == XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) &&
          (elementName == "group") && (pSettingsNode->hasAttributes()))
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* pAttr = pSettingsNode->getAttributes();
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNameAttr = pAttr->getNamedItem(X("name"));
         if (pNameAttr != NULL)
         {
            name = A(pNameAttr->getNodeValue());
         }
      }
      else
      {
         continue;
      }
      if (name == "MRUFiles")
      {
         Service<ModelServices> pModel;
         for (XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pMruFileNode = pSettingsNode->getFirstChild();
            pMruFileNode != NULL;
            pMruFileNode = pMruFileNode->getNextSibling())
         {
            if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pMruFileNode->getNodeName(), X("attribute")))
            {
               unsigned maxNumFiles = ConfigurationSettings::getSettingNumberOfMruFiles();
               if (mMruFiles.size() == maxNumFiles)
               {
                  break;
               }

               if (!static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pMruFileNode)->hasAttribute(X("name")))
               {
                  continue;
               }

               vector<ImportDescriptor*> descriptors;
               for (XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pDescriptorNode = pMruFileNode->getFirstChild();
                  pDescriptorNode != NULL;
                  pDescriptorNode = pDescriptorNode->getNextSibling())
               {
                  if (XERCES_CPP_NAMESPACE_QUALIFIER XMLString::equals(pDescriptorNode->getNodeName(),
                     X("DataDescriptor")))
                  {
                     XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
                        static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pDescriptorNode);
                     if (!(pElement->hasAttribute(X("type")) && pElement->hasAttribute(X("version"))))
                     {
                        continue;
                     }
                     string mruName = A(pElement->getAttribute(X("name")));
                     string type = A(pElement->getAttribute(X("type")));
                     unsigned int version = atoi(A(pElement->getAttribute(X("version"))));
                     DataDescriptor* pDescriptor = pModel->createDataDescriptor(mruName, type, NULL);
                     if (pDescriptor == NULL)
                     {
                        continue;
                     }
                     try
                     {
                        if (pDescriptor->fromXml(pElement, version))
                        {
                           ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(pDescriptor);
                           if (pImportDescriptor != NULL)
                           {
                              descriptors.push_back(pImportDescriptor);
                           }
                        }
                     }
                     catch (XmlBase::XmlException&)
                     {
                        // do nothing
                     }
                  }
               }
               string mruName = A(static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(
                                                pMruFileNode)->getAttribute(X("name")));
               string mruImporter = A(static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(
                                                pMruFileNode)->getAttribute(X("importer")));
               string mruModification = A(static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(
                                                pMruFileNode)->getAttribute(X("modification_time")));
               if ((mruName.empty() == false) && (mruImporter.empty() == false) && (mruModification.empty() == false))
               {
                  MruFile mruFile(mruName, mruImporter, descriptors, DateTimeImp(mruModification));
                  mMruFiles.push_back(mruFile);
               }
            }
         }
      }
   }

   mNeedToLoadMruFiles = false;
}

void ConfigurationSettingsImp::applicationClosed(Subject& subject, const string& signal, const boost::any& args)
{
   serialize();
}

string ConfigurationSettingsImp::getUserSettingsFileName() const
{
   string os = getOperatingSystemName();
   string arch = getArchitectureName();
   string mode = "";
   #if defined(DEBUG)
      mode = "Debug";
   #endif
   return "UserSettings-" + mVersion + "-" + os + "-" + arch + mode + ".cfg";
}

string ConfigurationSettingsImp::getUserSettingsFilePath() const
{
   QDir configDirectory = QDir(QString::fromStdString(mUserConfigPath));
   if (!configDirectory.exists())
   {
      return "";
   }

   QString cfgFilePath = configDirectory.absoluteFilePath(QString::fromStdString(getUserSettingsFileName()));
   return QDir::toNativeSeparators(cfgFilePath).toStdString();
}

string ConfigurationSettingsImp::locateUserDocs()
{
   string userDirectory;
   QString userFolderPath;
   QString appNamePath;
#if defined(WIN_API)
   char path[MAX_PATH];
   HRESULT retValue = SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
   if (SUCCEEDED(retValue))
   {
      userFolderPath = QString::fromAscii(path);
   }
#else
   char* pPath = getenv("HOME");
   if (pPath != NULL)
   {
      userFolderPath = QString::fromAscii(pPath);
   }
#endif
   appNamePath = APP_NAME;
   if (!userFolderPath.isEmpty())
   {
      userFolderPath.replace("\\", "/");
      QDir userFolderDir(userFolderPath);
      if (!userFolderDir.exists())
      {
         return "";
      }
      QString appUserPath = userFolderDir.absoluteFilePath(appNamePath);
      QDir appUserDir(appUserPath);
      if (!appUserDir.exists())
      {
         userFolderDir.mkdir(appNamePath);
      }
      userDirectory = appUserDir.absolutePath().toStdString();
   }

   return userDirectory;
}

bool ConfigurationSettingsImp::serialize() const
{
   return serializeSettings(getUserSettingsFilePath(), mpUserSettings.get(), true);
}

bool ConfigurationSettingsImp::serializeAsDefaults(const Filename* pFilename, const DynamicObject* pObject) const
{
   if (pFilename == NULL)
   {
      return false;
   }
   return serializeSettings(pFilename->getFullPathAndName(), pObject, false);
}

bool ConfigurationSettingsImp::loadSettings(string& errorMessage)
{
   string defaultDir = mHomePath + SLASH + "DefaultSettings";
   string customDefaultDir = mAdditionalDefaultDir;

   QStringList defaultDirectories;
   defaultDirectories.append(QString::fromStdString(defaultDir));
   if (!customDefaultDir.empty())
   {
      defaultDirectories.append(QString::fromStdString(customDefaultDir));
   }

   //locate all .cfg files found in all default directories and sort them into
   //a map based on load order
   multimap<unsigned int, string> configFileMap;
   QStringList::iterator dirIter;
   QStringList cfgExtensions;
   cfgExtensions.append("*.cfg");
   for (dirIter = defaultDirectories.begin(); dirIter != defaultDirectories.end(); ++dirIter)
   {
      QDir curDir(*dirIter);
      if (curDir.exists())
      {
         QFileInfoList cfgFiles = curDir.entryInfoList(cfgExtensions, QDir::Files, QDir::Name);
         QFileInfoList::iterator cfgIter;
         for (cfgIter = cfgFiles.begin(); cfgIter != cfgFiles.end(); ++cfgIter)
         {
            QFileInfo cfgFile = *cfgIter;
            QStringList entries = cfgFile.fileName().split("-");
            if (entries.size() >= 2) //Require .cfg files to be named like "1-foobar.cfg"
            {
               QString loadOrderStr = entries[0];
               bool success;
               unsigned int loadOrder = loadOrderStr.toUInt(&success);
               if (success)
               {
                  configFileMap.insert(make_pair(loadOrder, cfgFile.absoluteFilePath().toStdString()));
               }
            }
         }
      }
   }

   //we must at least have "1-ApplicationDefaults.cfg" which contains the application default settings
   bool missingAppDefaults = false;
   if (configFileMap.count(1) <= 0)
   {
      missingAppDefaults = true;
   }
   else if (configFileMap.count(1) == 1)
   {
      QFileInfo appDefaultConfigFileName(QString::fromStdString(configFileMap.find(1)->second));
      if (appDefaultConfigFileName.fileName() != "1-ApplicationDefaults.cfg")
      {
         missingAppDefaults = true;
      }
   }
   else 
   {
      errorMessage = "More than 1 default settings file was found with a load priority of 1. "
         "Only 1 default settings file can have a priority of 1.";
      return false;
   }

   if (missingAppDefaults)
   {
      errorMessage = "The 1-ApplicationDefaults.cfg could not be located in any of the default "
         "settings directories that were searched.";
      return false;
   }

   mpUserSettings->clear();
   mpSessionSettings->clear();
   mpDefaultSettings->clear();
   mNeedToLoadMruFiles = true;

   //parse all .cfg's files into mpDefaultSettings using the
   //load order
   multimap<unsigned int, string>::const_iterator configFileIter;
   for (configFileIter = configFileMap.begin(); configFileIter != configFileMap.end(); ++configFileIter)
   {
      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(configFileIter->second);
      FactoryResource<DynamicObject> pObj(deserialize(pFilename.get()));
      mpDefaultSettings->adoptiveMerge(pObj.get());
   }

   //parse the user's config file
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(getUserSettingsFilePath());
   FactoryResource<DynamicObject> pObj(deserialize(pFilename.get()));
   if (pObj.get() != NULL)
   {
      mpUserSettings->adoptiveMerge(pObj.get());
   }

   notify(SIGNAL_NAME(Subject, Modified));

   return true;
}

DynamicObject* ConfigurationSettingsImp::deserialize(const Filename* pFilename) const
{
   if (pFilename == NULL)
   {
      return NULL;
   }
   FactoryResource<DynamicObject> pObj;
   XmlReader xmlReader(NULL, false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDomDoc(NULL);
   pDomDoc = xmlReader.parse(pFilename);

   if (pDomDoc == NULL)
   {
      return NULL;
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pConfList(NULL);
   pConfList = pDomDoc->getElementsByTagName(X("ConfigurationSettings"));
   if (pConfList == NULL || pConfList->getLength() != 1)
   {
      return NULL;
   }

   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pSettingsNodes(NULL);
   pSettingsNodes = (pConfList->item(0))->getChildNodes();
   for (unsigned int i = 0; i < pSettingsNodes->getLength(); i++)
   {
      string name;
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pSettingsNode = pSettingsNodes->item(i);
      int nodeType = pSettingsNode->getNodeType();
      string elementName = A(pSettingsNode->getNodeName());
      if ((nodeType == XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) &&
          (elementName == "group") && (pSettingsNode->hasAttributes()))
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* pAttr = pSettingsNode->getAttributes();
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNameAttr = pAttr->getNamedItem(X("name"));
         if (pNameAttr != NULL)
         {
            name = A(pNameAttr->getNodeValue());
         }
      }
      else
      {
         continue;
      }
      if (name == "settings")
      {
         pObj->fromXml(pSettingsNode, XmlBase::VERSION); 
      }
   }

   return pObj.release();
}

void ConfigurationSettingsImp::updateProductionStatus()
{
   if (mReleaseType != RT_NORMAL && mReleaseType != RT_PROTO)
   {
      mProductionRelease = false;
      return;
   }

#if defined(_DEBUG)
   mProductionRelease = false;
#else
   mProductionRelease = APP_IS_PRODUCTION_RELEASE;
#endif

   if (mProductionRelease)
   {
      // search all plugins until we get a false
      Service<PlugInManagerServices> pPlugInManager;
      vector<PlugInDescriptor*> plugins = pPlugInManager->getPlugInDescriptors();
      for (vector<PlugInDescriptor*>::const_iterator plugin = plugins.begin();
         mProductionRelease && (plugin != plugins.end());
         ++plugin)
      {
         PlugInDescriptor* pDesc = *plugin;
         if (pDesc == NULL)
         {
            continue;
         }
         mProductionRelease = pDesc->isProduction();
      }
   }
}

bool ConfigurationSettingsImp::serializeSettings(const string& filename, const DynamicObject* pSettings,
                                                 bool saveMru) const
{
   if (filename.empty())
   {
      return false;
   }

   const_cast<ConfigurationSettingsImp*>(this)->notify(SIGNAL_NAME(ConfigurationSettings, AboutToSave));

   XMLWriter xmlWriter("ConfigurationSettings");

   // Version, release date, and build date
   string dateFormat = "%d %B %Y";

   string releaseText = mpReleaseDate->getFormattedUtc(dateFormat);
   string buildRevision = getBuildRevision();

   xmlWriter.pushAddPoint(xmlWriter.addElement("opticks"));
   xmlWriter.addAttr("version", mVersion);
   xmlWriter.addAttr("build_revision", buildRevision);
   xmlWriter.addAttr("release_date", releaseText);
   xmlWriter.popAddPoint();

   if (pSettings == NULL)
   {
      return false;
   }

   xmlWriter.pushAddPoint(xmlWriter.addElement("group"));
   xmlWriter.addAttr("name", "settings");
   pSettings->toXml(&xmlWriter);
   xmlWriter.popAddPoint();

   if (saveMru && !mMruFiles.empty())
   {
      xmlWriter.pushAddPoint(xmlWriter.addElement("group"));
      xmlWriter.addAttr("name", "MRUFiles");
      for (vector<MruFile>::const_iterator iter = mMruFiles.begin(); iter != mMruFiles.end(); ++iter)
      {
         MruFile mruFile = *iter;
         if (mruFile.mName.empty() || mruFile.mImporterName.empty() || mruFile.mDescriptors.empty())
         {
            continue;
         }
         xmlWriter.pushAddPoint(xmlWriter.addElement("attribute"));
         xmlWriter.addAttr("name", mruFile.mName);
         xmlWriter.addAttr("importer", mruFile.mImporterName);

         for (vector<ImportDescriptor*>::iterator descriptorIter = mruFile.mDescriptors.begin();
            descriptorIter != mruFile.mDescriptors.end();
            ++descriptorIter)
         {
            ImportDescriptor* pImportDescriptor = *descriptorIter;
            if (pImportDescriptor != NULL)
            {
               DataDescriptor* pDescriptor = pImportDescriptor->getDataDescriptor();
               if (pDescriptor != NULL)
               {
                  xmlWriter.pushAddPoint(xmlWriter.addElement("DataDescriptor"));
                  pDescriptor->toXml(&xmlWriter);
                  xmlWriter.popAddPoint();
               }
            }
         }

         const DateTime* pDateTime = &(mruFile.mModificationTime);
         string modificationText = StringUtilities::toXmlString(pDateTime);
         xmlWriter.addAttr("modification_time", modificationText);

         xmlWriter.popAddPoint();
      }

      xmlWriter.popAddPoint();
   }

   FILE* pFile = fopen(filename.c_str(), "wt");
   if (pFile != NULL)
   {
      xmlWriter.writeToFile(pFile);
      fclose(pFile);
      return true;
   }

   return false;
}
