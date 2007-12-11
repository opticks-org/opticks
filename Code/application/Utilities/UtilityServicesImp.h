/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _UTILITYSERVICESIMP_H
#define _UTILITYSERVICESIMP_H

#include "UtilityServices.h"
#include "TypesFile.h"

#include <map>

class DateTime;
class DateTimeImp;
class Progress;
class ProgressImp;
class MessageLogMgr;
class Options;

class UtilityServicesImp : public UtilityServices
{
public:
   static UtilityServicesImp* instance();
   static void destroy();

   DateTime* getDateTime();
   void destroyDateTime(DateTime* dt);
   Progress* getProgress(bool threadSafe=false);
   void destroyProgress(Progress* pg);
   MessageLogMgr* getMessageLog() const;
   unsigned int getNumProcessors() const;
   std::string getDefaultClassification() const;
   ColorType getAutoColor(int color_index) const;

   size_t getMaxMemoryBlockSize();
   size_t getTotalPhysicalMemory();
   size_t getAvailableVirtualMemory();
   uint64_t getAvailableDiskSpace( std::string path = "" );

   /**
    * Overrides the default classification with the given value.
    * This should NOT be put in the public interfaces, it should
    * only be used by internal testing code.
    * 
    * @param newClassification
    *        The new default level.  Provide empty string to
    *        stop overriding the default classification.
    *
    */
   void overrideDefaultClassification(const std::string& newClassification);

protected:
   virtual ~UtilityServicesImp() {};
   UtilityServicesImp() {};

private:
   std::map<DateTime*, DateTimeImp*> mDts;
   std::map<Progress*, ProgressImp*> mProgs;
   std::map<std::string, std::string> mSubtypeDirectories;

   static UtilityServicesImp* spInstance;
   static bool mDestroyed;
   std::string mClassificationOverride;
};

#endif
