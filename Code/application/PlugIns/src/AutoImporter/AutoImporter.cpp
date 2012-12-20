/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include "AppVerify.h"
#include "AppVersion.h"
#include "AutoImporter.h"
#include "DataDescriptor.h"
#include "DataElement.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Importer.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInDescriptor.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"

#include <algorithm>
#include <list>
#include <utility>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksAutoImporter, AutoImporter);

AutoImporter::AutoImporter() :
   mbInteractive(false),
   mpProgress(NULL),
   mpElement(NULL),
   mpPlugIn(NULL)
{
   setName("Auto Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("");
   setSubtype("Auto");
   setDescriptorId("{FAC90911-7B81-436c-932F-FA69498EE95A}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AutoImporter::~AutoImporter()
{
   for (map<string, PlugIn*>::iterator iter = mPlugIns.begin(); iter != mPlugIns.end(); ++iter)
   {
      PlugIn* pPlugIn = iter->second;
      if (pPlugIn != NULL)
      {
         mpPlugInManager->destroyPlugIn(pPlugIn);
      }
   }
}

class FindDescriptor
{
public:
   FindDescriptor(string name) :
      mName(name)
   {
   }

   bool operator()(const PlugInDescriptor* pDescriptor)
   {
      return (pDescriptor != NULL && pDescriptor->getName() == mName);
   }

   string mName;
};

string AutoImporter::getDefaultExtensions() const
{
   static string sDefaultExtensions = "";
   if (sDefaultExtensions.empty() == true)
   {
      vector<string> extensions;

      vector<PlugInDescriptor*> importerPlugIns =
         mpPlugInManager->getPlugInDescriptors(PlugInManagerServices::ImporterType());

      //remove self from list of importers
      vector<PlugInDescriptor*>::iterator newEnd =
         remove_if(importerPlugIns.begin(), importerPlugIns.end(), FindDescriptor(getName()));
      importerPlugIns.erase(newEnd, importerPlugIns.end());

      for (vector<PlugInDescriptor*>::iterator iter = importerPlugIns.begin();
           iter != importerPlugIns.end(); ++iter)
      {
         PlugInDescriptor* pDescriptor = *iter;
         if (pDescriptor == NULL)
         {
            continue;
         }
         string filters = pDescriptor->getFileExtensions();

         vector<string> filterList;
         while (filters.empty() == false)
         {
            string::size_type pos = filters.find(";;");
            if (pos == string::npos)
            {
               filterList.push_back(filters);
               filters.erase();
            }
            else
            {
               string currentFilter = filters.substr(0, pos);
               filterList.push_back(currentFilter);

               filters = filters.substr(pos + 2);
            }
         }

         for (vector<string>::iterator filterIter = filterList.begin(); filterIter != filterList.end(); ++filterIter)
         {
            string currentFilters = *filterIter;
            if (currentFilters.empty() == false)
            {
               // Add the file filters
               if (sDefaultExtensions.empty() == false)
               {
                  sDefaultExtensions += ";;";
               }

               sDefaultExtensions += currentFilters;

               // Extract the individual extensions for the all files filter
               string::size_type startPos = currentFilters.find("(");
               if (startPos != string::npos)
               {
                  startPos++;

                  string::size_type endPos = currentFilters.rfind(")");
                  if (endPos == string::npos)
                  {
                     endPos = currentFilters.length() - 1;
                  }

                  currentFilters = currentFilters.substr(startPos, endPos - startPos);
               }

               string extension;

               string::size_type pos = currentFilters.find(" ");
               if (pos == string::npos)
               {
                  extension = currentFilters;
                  currentFilters.erase();
               }
               else
               {
                  extension = currentFilters.substr(0, pos);
                  currentFilters = currentFilters.substr(pos + 1, currentFilters.length() - pos);
               }

               while (extension.empty() == false)
               {
                  bool bContains = false;

                  if ((extension == "*") || (extension == "*.*"))
                  {
                     bContains = true;
                  }
                  else
                  {
                     for (vector<string>::iterator extensionIter = extensions.begin();
                        extensionIter != extensions.end(); ++extensionIter)
                     {
                        string currentExtension = *extensionIter;
                        if (currentExtension == extension)
                        {
                           bContains = true;
                        }
                     }
                  }

                  if (bContains == false)
                  {
                     extensions.push_back(extension);
                  }

                  pos = currentFilters.find(" ");
                  if (pos == string::npos)
                  {
                     extension = currentFilters;
                     currentFilters.erase();
                  }
                  else
                  {
                     extension = currentFilters.substr(0, pos);
                     currentFilters = currentFilters.substr(pos + 1, currentFilters.length() - pos);
                  }
               }
            }
         }
      }

      // Create the all files filter
      string allFilesFilter;
      for (vector<string>::iterator iter = extensions.begin(); iter != extensions.end(); ++iter)
      {
         string extension = *iter;
         if (extension.empty() == false)
         {
            if (allFilesFilter.empty() == false)
            {
               allFilesFilter += " ";
            }

            allFilesFilter += extension;
         }
      }

      if (allFilesFilter.empty() == false)
      {
         allFilesFilter = "All " APP_NAME " Files (" + allFilesFilter + ")";
      }

      // Add the all files filter to the importer filters
      if (sDefaultExtensions.empty() == false)
      {
         sDefaultExtensions.insert(0, ";;");
      }

      sDefaultExtensions.insert(0, allFilesFilter);
   }

   return sDefaultExtensions;
}

bool AutoImporter::isProcessingLocationSupported(ProcessingLocation location) const
{
   bool bSupported = false;

   Importer* pImporter = dynamic_cast<Importer*>(mpPlugIn);
   if (pImporter != NULL)
   {
      bSupported = pImporter->isProcessingLocationSupported(location);
   }

   return bSupported;
}

vector<ImportDescriptor*> AutoImporter::getImportDescriptors(const string& filename)
{
   vector<ImportDescriptor*> descriptors;

   Importer* pImporter = findImporter(filename);
   if (pImporter != NULL)
   {
      descriptors = pImporter->getImportDescriptors(filename);
   }

   return descriptors;
}

unsigned char AutoImporter::getFileAffinity(const string& filename)
{
   findImporter(filename);

   map<string, pair<string, unsigned char> >::iterator iter = mFilenames.find(filename);
   if (iter != mFilenames.end())
   {
      return iter->second.second;
   }

   return Importer::CAN_NOT_LOAD;
}

QWidget* AutoImporter::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   QWidget* pPreviewWidget = NULL;

   Importer* pImporter = findImporter(pDescriptor);
   if (pImporter != NULL)
   {
      pPreviewWidget = pImporter->getPreview(pDescriptor, pProgress);
   }

   return pPreviewWidget;
}

bool AutoImporter::validate(const DataDescriptor* pDescriptor,
                            const vector<const DataDescriptor*>& importedDescriptors,
                            string& errorMessage) const
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   bool bValid = false;

   Importer* pImporter = const_cast<AutoImporter*>(this)->findImporter(pDescriptor);
   if (pImporter != NULL)
   {
      bValid = pImporter->validate(pDescriptor, importedDescriptors, errorMessage);
      if (errorMessage.empty() == false)
      {
         VERIFY(mpPlugIn != NULL);
         string importerName = mpPlugIn->getName();
         errorMessage.insert(0, importerName + ":\n");
      }
   }

   return bValid;
}

QWidget* AutoImporter::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   QWidget* pWidget = NULL;

   Importer* pImporter = findImporter(pDescriptor);
   if (pImporter != NULL)
   {
      pWidget = pImporter->getImportOptionsWidget(pDescriptor);
   }

   return pWidget;
}

void AutoImporter::polishDataDescriptor(DataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return;
   }

   Importer* pImporter = findImporter(pDescriptor);
   if (pImporter != NULL)
   {
      pImporter->polishDataDescriptor(pDescriptor);
   }
}

bool AutoImporter::setBatch()
{
   mbInteractive = false;
   return true;
}

bool AutoImporter::setInteractive()
{
   mbInteractive = true;
   return true;
}

bool AutoImporter::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   // Menu command
   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);

   pArg->setName(Executable::MenuCommandArg());
   pArg->setType("string");
   pArg->setDescription("String name for the menu command.");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   // Progress
   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);

   pArg->setName(Executable::ProgressArg());
   pArg->setType("Progress");
   pArg->setDescription(Executable::ProgressArgDescription());
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   // Data element
   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);

   pArg->setName(Importer::ImportElementArg());
   pArg->setType("DataElement");
   pArg->setDescription("Data element to be imported.");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool AutoImporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   return true;
}

bool AutoImporter::hasAbort()
{
   return true;
}

bool AutoImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Auto Importer", "app", "5CBC6E31-AF2E-4C58-A846-86C347D22125");

   if (extractPlugInArgs(pInArgList) == false)
   {
      return false;
   }

   VERIFY(mpElement != NULL);

   string filename = mpElement->getFilename();
   findImporter(filename);

   // If no importer was found, return false.
   if (mpPlugIn == NULL)
   {
      return false;
   }

   ExecutableResource pExecutable(mpPlugIn, mMenuCommand, mpProgress, !mbInteractive);
   pExecutable->releasePlugIn();
   if (pExecutable.get() != NULL)
   {
      // Create the input arg list
      pExecutable->getInArgList().setPlugInArgValueLoose(Importer::ImportElementArg(), mpElement);
      if (!pExecutable->execute())
      {
         return false;
      }
   }
   else
   {
      string message = "Could not find an importer to execute!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   pStep->finalize(Message::Success);
   return true;
}

bool AutoImporter::abort()
{
   Executable* pExecutable = dynamic_cast<Executable*>(mpPlugIn);
   if (pExecutable != NULL)
   {
      if (pExecutable->hasAbort() == true)
      {
         return pExecutable->abort();
      }
   }

   return false;
}

bool AutoImporter::extractPlugInArgs(const PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   StepResource pStep("Validate Inputs", "app", "2E851D37-622E-43F1-8E1E-6574D26E6DB0");

   // Menu command
   string* pCommand = pArgList->getPlugInArgValue<string>(Executable::MenuCommandArg());
   if (pCommand != NULL)
   {
      mMenuCommand = *pCommand;
   }

   // Progress
   mpProgress = pArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   // Data element
   mpElement = pArgList->getPlugInArgValue<DataElement>(Importer::ImportElementArg());
   if (mpElement == NULL)
   {
      string message = "The data element input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   pStep->finalize(Message::Success);
   return true;
}

bool AutoImporter::checkExtension(const PlugInDescriptor* pDescriptor, const string& filename) const
{
   if ((pDescriptor == NULL) || (filename.empty() == true))
   {
      return false;
   }

   // Get the filename without the path
   QFileInfo fileInfo(QString::fromStdString(filename));
   QString strFilename = fileInfo.fileName();
   if (strFilename.isEmpty() == true)
   {
      return false;
   }

   // Get the list of filename patterns from the importer
   QString strImporterExtensions = QString::fromStdString(pDescriptor->getFileExtensions());
   if (strImporterExtensions.isEmpty() == true)
   {
      return false;
   }

   QStringList filterListCandidates = strImporterExtensions.split(";;", QString::SkipEmptyParts);
   QStringList filterList;
   for (int i = 0; i < filterListCandidates.count(); i++)
   {
      QString strExtensions = filterListCandidates[i];
      if (strExtensions.isEmpty() == false)
      {
         int iOpenPos = strExtensions.indexOf("(");
         int iClosePos = strExtensions.lastIndexOf(")");
         strExtensions = strExtensions.mid(iOpenPos + 1, iClosePos - iOpenPos - 1);

         QStringList globPatterns = strExtensions.split(QString(" "), QString::SkipEmptyParts);
         QString catchAll = QString::fromStdString("*");
         QString catchAll2 = QString::fromStdString("*.*");
         for (int globCount = 0; globCount < globPatterns.count(); ++globCount)
         {
            QString pattern = globPatterns[globCount];
            if ((pattern != catchAll) && (pattern != catchAll2))
            {
               filterList << pattern;
            }
         }
      }
   }

   // Compare the filename against the importer filename patterns
   return QDir::match(filterList, strFilename);
}

Importer* AutoImporter::findImporter(const DataDescriptor* pDescriptor)
{
   string filename;
   if (pDescriptor != NULL)
   {
      const FileDescriptor* pFileDescriptor = pDescriptor->getFileDescriptor();
      if (pFileDescriptor != NULL)
      {
         filename = pFileDescriptor->getFilename();
      }
   }

   return findImporter(filename);
}

Importer* AutoImporter::findImporter(const string& filename)
{
   map<string, pair<string, unsigned char> >::iterator fileIter = mFilenames.find(filename);
   if (fileIter != mFilenames.end())
   {
      map<string, PlugIn*>::iterator plugInIter = mPlugIns.find(fileIter->second.first);
      if (plugInIter != mPlugIns.end())
      {
         mpPlugIn = plugInIter->second;
         return dynamic_cast<Importer*>(mpPlugIn);
      }
   }

   mpPlugIn = NULL;

   if (filename.empty() == true)
   {
      return NULL;
   }

   // Create the list of importers to search
   vector<PlugInDescriptor*> importers = mpPlugInManager->getPlugInDescriptors(PlugInManagerServices::ImporterType());

   //remove self from list of importers
   vector<PlugInDescriptor*>::iterator newEnd =
      remove_if(importers.begin(), importers.end(), FindDescriptor(getName()));
   importers.erase(newEnd, importers.end());

   // Check importers first based on file extension
   list<PlugInDescriptor*> remainingImporters;
   PlugIn* pPlugIn = NULL;
   unsigned char maxFileAffinity = Importer::CAN_NOT_LOAD;

   for (vector<PlugInDescriptor*>::iterator iter = importers.begin(); iter != importers.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor == NULL)
      {
         continue;
      }

      if (checkExtension(pDescriptor, filename))
      {
         PlugIn* pCurrentPlugIn = NULL;
         const string& plugInName = pDescriptor->getName();

         map<string, PlugIn*>::iterator plugInIter = mPlugIns.find(plugInName);
         if (plugInIter != mPlugIns.end())
         {
            pCurrentPlugIn = plugInIter->second;
         }
         else
         {
            PlugInResource pImporterRes(plugInName);
            pCurrentPlugIn = pImporterRes.release();
            if (pCurrentPlugIn != NULL)
            {
               mPlugIns[plugInName] = pCurrentPlugIn;
            }
         }

         Importer* pImporter = dynamic_cast<Importer*>(pCurrentPlugIn);
         if (pImporter != NULL)
         {
            unsigned char fileAffinity = pImporter->getFileAffinity(filename);
            if (fileAffinity > maxFileAffinity)
            {
               maxFileAffinity = fileAffinity;
               pPlugIn = pCurrentPlugIn;
            }
         }
      }
      else
      {
         remainingImporters.push_back(pDescriptor);
      }
   }

   if ((pPlugIn != NULL) && (maxFileAffinity >= Importer::CAN_LOAD))
   {
      mpPlugIn = pPlugIn;
      mFilenames[filename] = make_pair(mpPlugIn->getName(), maxFileAffinity);
      return dynamic_cast<Importer*>(mpPlugIn);
   }

   // Check the remaining importers
   for (list<PlugInDescriptor*>::iterator iter = remainingImporters.begin(); iter != remainingImporters.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if (pDescriptor == NULL)
      {
         continue;
      }

      PlugIn* pCurrentPlugIn = NULL;
      const string& plugInName = pDescriptor->getName();

      map<string, PlugIn*>::iterator plugInIter = mPlugIns.find(plugInName);
      if (plugInIter != mPlugIns.end())
      {
         pCurrentPlugIn = plugInIter->second;
      }
      else
      {
         PlugInResource pImporterRes(plugInName);
         pCurrentPlugIn = pImporterRes.release();
         if (pCurrentPlugIn != NULL)
         {
            mPlugIns[plugInName] = pCurrentPlugIn;
         }
      }

      Importer* pImporter = dynamic_cast<Importer*>(pCurrentPlugIn);
      if (pImporter != NULL)
      {
         unsigned char fileAffinity = pImporter->getFileAffinity(filename);
         if (fileAffinity > maxFileAffinity)
         {
            maxFileAffinity = fileAffinity;
            pPlugIn = pCurrentPlugIn;
         }
      }
   }

   // If an importer was found, assign it.
   if ((pPlugIn != NULL) && (maxFileAffinity > Importer::CAN_NOT_LOAD))
   {
      mpPlugIn = pPlugIn;
      mFilenames[filename] = make_pair(mpPlugIn->getName(), maxFileAffinity);
      return dynamic_cast<Importer*>(mpPlugIn);
   }

   return NULL;
}
