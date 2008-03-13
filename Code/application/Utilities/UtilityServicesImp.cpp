/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "UtilityServicesImp.h"

#include "ColorMap.h"
#include "ConfigurationSettingsImp.h"
#include "DateTimeImp.h"
#include "MessageLogMgrImp.h"
#include "ProgressAdapter.h"
#include "SessionManager.h"
#include "ThreadSafeProgressAdapter.h"

#if defined(WIN_API)
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

using namespace std;

#include "ExportAgentAdapter.h"
#include "ImportAgentAdapter.h"
#include "ExecutableAgentAdapter.h"

UtilityServicesImp* UtilityServicesImp::spInstance = NULL;
bool UtilityServicesImp::mDestroyed = false;

UtilityServicesImp* UtilityServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if(mDestroyed)
      {
         throw std::logic_error("Attempting to use UtilityServices after "
            "destroying it.");
      }
      spInstance = new UtilityServicesImp;
   }

   return spInstance;
}

void UtilityServicesImp::destroy()
{
   if(mDestroyed)
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
   mDts.insert(pair<DateTime*,DateTimeImp*> ((DateTime*) pDt, pDt));
   return pDt;
}

void UtilityServicesImp::destroyDateTime(DateTime* dt)
{
   map<DateTime*, DateTimeImp*>::iterator itr;
   itr = mDts.find(dt);
   if (itr != mDts.begin() )
   {
      delete itr->second;
      mDts.erase(itr);
   }
}

Progress* UtilityServicesImp::getProgress(bool threadSafe)
{
   if(threadSafe)
   {
      ThreadSafeProgressAdapter* pDt = new ThreadSafeProgressAdapter;
      mProgs.insert(pair<Progress*,ProgressImp*> ((Progress*)pDt, pDt));
      return pDt;
   }
   // else
   ProgressAdapter* pDt = new ProgressAdapter;
   mProgs.insert(pair<Progress*,ProgressImp*> ((Progress*) pDt, pDt));
   return pDt;
}

void UtilityServicesImp::destroyProgress(Progress* mx)
{
   map<Progress*, ProgressImp*>::iterator itr;
   itr = mProgs.find(mx);
   if (itr != mProgs.begin() && itr != mProgs.end())
   {
      delete itr->second;
      mProgs.erase(itr);
   }
}

MessageLogMgr *UtilityServicesImp::getMessageLog() const
{
   MessageLogMgr *pMessageLogMgr = NULL;
   pMessageLogMgr = MessageLogMgrImp::instance();
   return pMessageLogMgr;
}

unsigned int UtilityServicesImp::getNumProcessors() const
{
#if defined(WIN_API)
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
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
   static string classificationText = "";

   if (classificationText.empty() == true)
   {
      const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
      string filename;
      if (pSupportFilesPath != NULL)
      {
         filename = pSupportFilesPath->getFullPathAndName();
      }
      if(!filename.empty())
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

         classificationText = buffer;
         fclose(classFile);
      }
   }

   return classificationText;
}

void UtilityServicesImp::overrideDefaultClassification(const std::string& newClassification)
{
   mClassificationOverride = newClassification;
}

ColorType UtilityServicesImp::getAutoColor( int color_index ) const
{
   static bool bFirstTime = true;
   static ColorMap colormap;
   ColorType color;

   //first time we call this function, attempt to read in the color table
   if (bFirstTime)
   {
      bFirstTime = false;
      string filename = "autocolor.clu";
      try
      {
         string fullPath;
         const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
         if(pSupportFilesPath != NULL)
         {
            fullPath = pSupportFilesPath->getFullPathAndName() + SLASH + "ColorTables" + SLASH + filename;
         }
         ColorMap cm(fullPath);
         colormap = cm;
      }
      catch(std::exception &exc)// bad color map file
      {
         MessageLogMgr *pLogMgr = getMessageLog();

         if (pLogMgr != NULL)
         {
            Service<SessionManager> pSessionManager;
            MessageLog *pLog(pLogMgr->getLog(
                              pSessionManager->getName()));
            if(pLog != NULL)
            {
               Message *msg(pLog->createMessage("Exception", "app", "35E9D938-E536-4aa2-82F2-FB10E71D1DD1"));
               msg->addProperty("message",exc.what());
               msg->finalize();
            }
         }
         std::vector<ColorType> table;
         const string& name = "Auto Colors";
         table.push_back(ColorType(255,0,0));

         ColorMap cm(name, table);
         colormap = cm;
      }
   }

   int size = colormap.getTable().size();
   int index = color_index%size;

   color = colormap[index];
   return color;
}

size_t UtilityServicesImp::getMaxMemoryBlockSize()
{
#if defined(UNIX_API)
   //this is the best we can do on Solaris
   return UtilityServicesImp::getAvailableVirtualMemory();
#elif defined(WIN_API)
   // Get maximum address range from system info.
   SYSTEM_INFO systemInfo;
   GetSystemInfo(&systemInfo);
   char* maxAddress = (char*)systemInfo.lpMaximumApplicationAddress;

   //
   // Walk process addresses.
   //

   char*  pMemory = NULL;
   MEMORY_BASIC_INFORMATION memoryInfo;
   char*  baseAddress = 0;
   size_t regionSize = 0;

   while (pMemory < maxAddress)	{

      size_t ret = VirtualQuery( pMemory, &memoryInfo, sizeof (MEMORY_BASIC_INFORMATION));

      // Check to make sure we didn't fail.
      if (ret!=sizeof(MEMORY_BASIC_INFORMATION)) {
         pMemory = maxAddress; // Quit walking the process
         continue;
      }

      // Determine if this block of memory is free and larger than the saved block. 

      if ((memoryInfo.State==MEM_FREE) && (memoryInfo.RegionSize>regionSize)) {
         baseAddress = static_cast<char*>(memoryInfo.BaseAddress);
         regionSize = memoryInfo.RegionSize;
      }
	   // Increment to next region of memory.
      pMemory = (char*)((char*)memoryInfo.BaseAddress + memoryInfo.RegionSize);  
	}
   return regionSize;
#endif
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
   stat.dwLength = sizeof (stat);
   GlobalMemoryStatusEx (&stat);
   total = static_cast<size_t>(stat.ullTotalPhys);
#endif
   return total;
}

size_t UtilityServicesImp::getAvailableVirtualMemory()
{ 
   size_t total = 0;

#if defined(UNIX_API)
   struct rlimit64 totMem;
   unsigned long rlim_ret = getrlimit64(RLIMIT_VMEM,&totMem);
   total = totMem.rlim_max;
#elif defined(WIN_API)
   MEMORYSTATUSEX stat;
   stat.dwLength = sizeof (stat);
   GlobalMemoryStatusEx (&stat);
   total = static_cast<size_t>(stat.ullAvailVirtual);
#endif
   return total;
}

uint64_t UtilityServicesImp::getAvailableDiskSpace( string path )
{
   return 0;
}
