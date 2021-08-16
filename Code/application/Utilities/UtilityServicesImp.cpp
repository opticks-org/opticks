/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "AppConfig.h"
#include "ColorMap.h"
#include "ConfigurationSettingsImp.h"
#include "DateTimeImp.h"
#include "MessageLogMgrImp.h"
#include "ProgressAdapter.h"
#include "SessionManager.h"
#include "StringUtilities.h"
#include "ThreadSafeProgressAdapter.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"

#if defined(WIN_API)
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

#include <algorithm>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

using namespace std;

struct SecurityFieldValue
{
   string mType;
   string mName;
};

UtilityServicesImp* UtilityServicesImp::spInstance = NULL;
bool UtilityServicesImp::mDestroyed = false;

UtilityServicesImp* UtilityServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use UtilityServices after "
            "destroying it.");
      }
      spInstance = new UtilityServicesImp;
      UtilityServicesImp::CallFunctionEvent::CallFunction = QEvent::registerEventType();
   }

   return spInstance;
}

void UtilityServicesImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy UtilityServices after "
         "destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

DateTime* UtilityServicesImp::getDateTime()
{
   DateTimeImp* pDt = new DateTimeImp;
   mDts.insert(pair<DateTime*, DateTimeImp*>(static_cast<DateTime*>(pDt), pDt));
   return pDt;
}

void UtilityServicesImp::destroyDateTime(DateTime* dt)
{
   map<DateTime*, DateTimeImp*>::iterator itr;
   itr = mDts.find(dt);
   if (itr != mDts.begin())
   {
      delete itr->second;
      mDts.erase(itr);
   }
}

Progress* UtilityServicesImp::getProgress(bool threadSafe)
{
   if (threadSafe)
   {
      ThreadSafeProgressAdapter* pDt = new ThreadSafeProgressAdapter;
      mProgs.insert(pair<Progress*, ProgressImp*>(static_cast<Progress*>(pDt), pDt));
      return pDt;
   }
   // else
   ProgressAdapter* pDt = new ProgressAdapter;
   mProgs.insert(pair<Progress*, ProgressImp*>(static_cast<Progress*>(pDt), pDt));
   return pDt;
}

void UtilityServicesImp::destroyProgress(Progress* pProgress)
{
   map<Progress*, ProgressImp*>::iterator itr = mProgs.find(pProgress);
   if (itr != mProgs.end())
   {
      delete itr->second;
      mProgs.erase(itr);
   }
}

MessageLogMgr *UtilityServicesImp::getMessageLog() const
{
   MessageLogMgr* pMessageLogMgr = MessageLogMgrImp::instance();
   return pMessageLogMgr;
}

unsigned int UtilityServicesImp::getNumProcessors() const
{
#if defined(WIN_API)
   SYSTEM_INFO systemInfo;
   GetSystemInfo(&systemInfo);
   return systemInfo.dwNumberOfProcessors;
#else
   return sysconf(_SC_NPROCESSORS_CONF);
#endif
}

std::string UtilityServicesImp::getDefaultClassification() const
{
   if (!mClassificationOverride.empty())
   {
      //Allow testing code to call overrideDefaultClassification()
      //to temporily change the default classification.  This
      //string should only be non-empty in testing code.  If this string
      //is non-empty in production code, that is a big problem.
      return mClassificationOverride;
   }

   static string sClassificationText = "";
   if (sClassificationText.empty() == true)
   {
      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      string filename;
      if (pSupportFilesPath != NULL)
      {
         filename = pSupportFilesPath->getFullPathAndName();
      }
      if (!filename.empty())
      {
         filename += "/SecurityMarkings/ClassificationLevels.txt";
      }

      FILE* classFile = NULL;
      classFile = fopen(filename.c_str(), "r");
      if (classFile != NULL)
      {
         char buffer[1000];
         if (fgets(buffer, 999, classFile) == NULL)
         {
            fclose(classFile);
            return NULL;
         }

         for (unsigned int i = 0; i < strlen(buffer); i++)
         {
            if (buffer[i] == '\n' || buffer[i] == '\r')
            {
               buffer[i] = 0;
            }
         }

         sClassificationText = buffer;
         fclose(classFile);
      }
   }

   return sClassificationText;
}

void UtilityServicesImp::overrideDefaultClassification(const std::string& newClassification)
{
   mClassificationOverride = newClassification;
}

ColorType UtilityServicesImp::getAutoColor(int color_index) const
{
   static bool sFirstTime = true;
   static ColorMap sColormap;
   ColorType color;

   //first time we call this function, attempt to read in the color table
   if (sFirstTime)
   {
      sFirstTime = false;
      string filename = "autocolor.clu";
      try
      {
         string fullPath;
         const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
         if (pSupportFilesPath != NULL)
         {
            fullPath = pSupportFilesPath->getFullPathAndName() + SLASH + "ColorTables" + SLASH + filename;
         }
         ColorMap cm(fullPath);
         sColormap = cm;
      }
      catch (std::exception &exc)// bad color map file
      {
         MessageLogMgr* pLogMgr = getMessageLog();
         if (pLogMgr != NULL)
         {
            Service<SessionManager> pSessionManager;
            MessageLog* pLog(pLogMgr->getLog(pSessionManager->getName()));
            if (pLog != NULL)
            {
               Message* msg(pLog->createMessage("Exception", "app", "35E9D938-E536-4aa2-82F2-FB10E71D1DD1"));
               msg->addProperty("message", exc.what());
               msg->finalize();
            }
         }
         std::vector<ColorType> table;
         const string& name = "Auto Colors";
         table.push_back(ColorType(255, 0, 0));

         ColorMap cm(name, table);
         sColormap = cm;
      }
   }

   int size = sColormap.getTable().size();
   int index = color_index%size;

   color = sColormap[index];
   return color;
}

size_t UtilityServicesImp::getMaxMemoryBlockSize()
{
#if defined(UNIX_API)
   //this is the best we can do on Solaris
   return UtilityServicesImp::getAvailableVirtualMemory();
#elif defined(WIN_API)
   // Get maximum address range from system info
   SYSTEM_INFO systemInfo;
   GetSystemInfo(&systemInfo);
   char* pMaxAddress = reinterpret_cast<char*>(systemInfo.lpMaximumApplicationAddress);

   // Walk process addresses
   char* pMemory = NULL;
   MEMORY_BASIC_INFORMATION memoryInfo;
   size_t regionSize = 0;

   while (pMemory < pMaxAddress)
   {
      size_t ret = VirtualQuery(pMemory, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION));

      // Check to make sure we didn't fail
      if (ret != sizeof(MEMORY_BASIC_INFORMATION))
      {
         pMemory = pMaxAddress; // Quit walking the process
         continue;
      }

      // Determine if this block of memory is free and larger than the saved block
      if ((memoryInfo.State == MEM_FREE) && (memoryInfo.RegionSize > regionSize))
      {
         regionSize = memoryInfo.RegionSize;
      }

      // Increment to next region of memory
      pMemory = reinterpret_cast<char*>(memoryInfo.BaseAddress) + memoryInfo.RegionSize;
   }

   return regionSize;
#endif
}

bool UtilityServicesImp::loadSecurityMarkings(const string &strFilename)
{
   Service<MessageLogMgr> pMsgLog;
   MessageLog* pLog = pMsgLog->getLog();

   QDir xmldir(QString::fromStdString(strFilename));
   if (!xmldir.exists())
   {
      return false;
   }

   QStringList secExtensions;
   secExtensions.append("*.sec");

   XmlReader read(pLog, true);
   multimap<unsigned short, QString> dir;


   QFileInfoList secFiles = xmldir.entryInfoList(secExtensions, QDir::Files, QDir::Name);
   for (QFileInfoList::iterator secIter = secFiles.begin(); secIter != secFiles.end(); ++secIter)
   {
      QFileInfo cfgFile = *secIter;
      QStringList entries = cfgFile.fileName().split("-");
      if (entries.size() >= 2) 
      {
         QString loadOrderStr = entries[0];
         bool success;
         unsigned int loadOrder = loadOrderStr.toUInt(&success);
         if (success)
         {
            dir.insert(pair<unsigned short, QString>(loadOrder, (*secIter).absoluteFilePath()));
         }
      }
   }

   multimap<unsigned short, SecurityFieldValue> markingValues;

   for (multimap<unsigned short, QString>::iterator iter = dir.begin(); iter != dir.end(); ++iter)
   {
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pDocument = NULL;
      pDocument = read.parse((*iter).second.toStdString());
               
      if (pDocument == NULL)
      {
         continue;
      }

      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *pRoot = NULL;

      pRoot = pDocument->getDocumentElement();
      if (pRoot == NULL)
      {
         continue;
      }

      XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pChild = pRoot->getChildNodes();
      for (unsigned int i = 0; i < pChild->getLength(); ++i)
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode = pChild->item(i);

         // Check the child node type to see if it's a element.
         if (pNode->getNodeType() != DOMNode::ELEMENT_NODE)
         {
            continue;
         }

         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pNode);
         if (pElement != NULL)
         {
            SecurityFieldValue marking;
            marking.mType = A(pElement->getTagName());
            marking.mName = A(pElement->getAttribute(X("data")));
            int value = StringUtilities::fromXmlString<int>(A(pElement->getAttribute(X("priority"))));
            markingValues.insert(pair<unsigned short, SecurityFieldValue>(value, marking));
         }
      }
   }

   multimap<unsigned short, SecurityFieldValue>::iterator iter;
   for (iter = markingValues.begin(); iter != markingValues.end(); ++iter)
   {
      if ((*iter).second.mType == "CountryCode")
      {
         mCountryCodes.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "Codeword")
      {
         mCodeword.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "Systems")
      {
         mSystems.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "FileReleasing")
      {
         mFileReleasing.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "DeclassificationExemption")
      {
         mDeclassificationExemption.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "ClassificationReason")
      {
         mClassificationReason.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "DeclassificationType")
      {
         mDeclassificationType.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "FileDowngrade")
      {
         mFileDowngrade.push_back((*iter).second.mName);
      }
      else if ((*iter).second.mType == "FileControl")
      {
         mFileControl.push_back((*iter).second.mName);
      }
   }

   mCountryCodes.erase(unique(mCountryCodes.begin(), mCountryCodes.end()), mCountryCodes.end());  
   mCodeword.erase(unique(mCodeword.begin(), mCodeword.end()), mCodeword.end());
   mSystems.erase(unique(mSystems.begin(), mSystems.end()), mSystems.end());
   mFileReleasing.erase(unique(mFileReleasing.begin(), mFileReleasing.end()), mFileReleasing.end());
   mDeclassificationExemption.erase(unique(mDeclassificationExemption.begin(), mDeclassificationExemption.end()),
      mDeclassificationExemption.end());
   mClassificationReason.erase(unique(mClassificationReason.begin(), mClassificationReason.end()),
      mClassificationReason.end());
   mDeclassificationType.erase(unique(mDeclassificationType.begin(), mDeclassificationType.end()),
      mDeclassificationType.end());
   mFileDowngrade.erase(unique(mFileDowngrade.begin(), mFileDowngrade.end()), mFileDowngrade.end());
   mFileControl.erase(unique(mFileControl.begin(), mFileControl.end()), mFileControl.end());

   return true;
}

const vector<string>& UtilityServicesImp::getCountryCodes() const
{
   return mCountryCodes;
}

const vector<string>& UtilityServicesImp::getCodewords() const 
{
   return mCodeword;
}

const vector<string>& UtilityServicesImp::getSystems() const
{
   return mSystems;
}

const vector<string>& UtilityServicesImp::getFileReleasing() const
{
   return mFileReleasing;
}

const vector<string>& UtilityServicesImp::getDeclassificationExemptions() const
{
   return mDeclassificationExemption;
}

const vector<string>& UtilityServicesImp::getClassificationReasons() const
{
   return mClassificationReason;
}

const vector<string>& UtilityServicesImp::getDeclassificationTypes() const
{
   return mDeclassificationType;
}

const vector<string>& UtilityServicesImp::getFileDowngrades() const
{
   return mFileDowngrade;
}

const vector<string>& UtilityServicesImp::getFileControls() const
{
   return mFileControl;
}

size_t UtilityServicesImp::getTotalPhysicalMemory()
{
   size_t total = 0;

#if defined(UNIX_API)
   size_t pages = sysconf(_SC_PHYS_PAGES);
   size_t pageSize = sysconf(_SC_PAGESIZE);
   total = pages * pageSize;
#elif defined(WIN_API)
   MEMORYSTATUSEX stat;
   stat.dwLength = sizeof(stat);
   GlobalMemoryStatusEx(&stat);
   total = static_cast<size_t>(stat.ullTotalPhys);
#endif
   return total;
}

size_t UtilityServicesImp::getAvailableVirtualMemory()
{
   size_t total = 0;

#if defined(SOLARIS)
   struct rlimit64 totMem;
   unsigned long rlim_ret = getrlimit64(RLIMIT_VMEM, &totMem);
   total = totMem.rlim_max;
#elif defined(LINUX)
   struct rlimit totMem;
   getrlimit(RLIMIT_AS, &totMem);
   total = totMem.rlim_max;
#elif defined(WIN_API)
   MEMORYSTATUSEX stat;
   stat.dwLength = sizeof(stat);
   GlobalMemoryStatusEx(&stat);
   total = static_cast<size_t>(stat.ullAvailVirtual);
#endif
   return total;
}

string UtilityServicesImp::getTextFromFile(const string& filename)
{
   QString fileStr = QString::fromStdString("Unable to open file: " + filename);
   QFile file(QString::fromStdString(filename));
   if (file.open(QFile::ReadOnly | QFile::Text))
   {
      QByteArray fileBytes(file.readAll());
      fileStr = QString(fileBytes);
   }

   return fileStr.toStdString();
}

void UtilityServicesImp::callInMainThread(std::function<void()> func)
{
   if (QCoreApplication::instance()->thread() == QThread::currentThread())
   {
      // already in the main thread, just call it
      func();
   }
   else
   {
      // send an event
      QEvent* pEvent = new UtilityServicesImp::CallFunctionEvent(func);
      QCoreApplication::postEvent(this, pEvent, Qt::HighEventPriority);
   }
}

bool UtilityServicesImp::event(QEvent* pEvent)
{
   if (pEvent != nullptr && pEvent->type() == UtilityServicesImp::CallFunctionEvent::CallFunction)
   {
      auto* pCfe = static_cast<UtilityServicesImp::CallFunctionEvent*>(pEvent);
      pCfe->getFunction()();
      return true;
   }
   return QObject::event(pEvent);
}

int UtilityServicesImp::CallFunctionEvent::CallFunction = -1;

UtilityServicesImp::CallFunctionEvent::CallFunctionEvent(std::function<void()> func) :
   QEvent(static_cast<QEvent::Type>(UtilityServicesImp::CallFunctionEvent::CallFunction)),
   mFunc(func)
{}

std::function<void()> UtilityServicesImp::CallFunctionEvent::getFunction() const
{
   return mFunc;
}