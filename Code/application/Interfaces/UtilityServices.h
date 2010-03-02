/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _UTILITYSERVICES_H
#define _UTILITYSERVICES_H

#include "ColorType.h"
#include "DateTime.h"
#include "MessageLogMgr.h"
#include "Progress.h"
#include "Service.h"

/**
 *  \ingroup ServiceModule
 *  Provides access to data objects not available in the object factory
 *
 *  Access to instantiation of Utilities objects found
 *  in the Interfaces. These are inherited from Serializable,
 *  and therefore cannot be created with the ObjectFactory.
 *
 *  @see        ObjectFactory
 */
class UtilityServices
{
public:
   /**
    *  Instantiate an object that can be used with the
    *  DateTime interface.
    *
    *  @return  Pointer to an object publicizing the DateTime
    *           interface.
    */
   virtual DateTime* getDateTime() = 0;

   /**
    *  Delete an object that can be used with the
    *  DateTime interface.
    *
    *  @param   dt
    *           Pointer to an object publicizing the DateTime
    *           interface.
    */
   virtual void destroyDateTime(DateTime* dt) = 0;

   /**
    *  Instantiate an object that can be used with the
    *  Progress interface.
    *
    *  @param   threadSafe
    *           should a thread safe Progress be created?
    *           This is needed for background plugins.
    *           If the plugin is not running in the background, you
    *           should use a non-thread safe version as there
    *           is less overhead
    *
    *  @return  Pointer to an object publicizing the DateTime
    *           interface.
    */
   virtual Progress* getProgress(bool threadSafe=false) = 0;

   /**
    *  Delete an object that can be used with the
    *  Progress interface.
    *
    *  @param   prog
    *           Pointer to an object publicizing the Progress
    *           interface.
    */
   virtual void destroyProgress(Progress* prog) = 0;

   /**
    *  Returns a pointer to the MessageLogMgr singleton.
    *
    *  @return  Pointer to the MessageLogMgr singleton.
    */
   virtual MessageLogMgr* getMessageLog() const = 0;

   /**
    *  Returns the number of processors on the computer.
    *
    *  @return  The number of processors on the computer.
    */
   virtual unsigned int getNumProcessors() const = 0;

   /**
    *  Returns the default classification level, based on the value in
    *  the first row of the SecurityMarkings/Classification.txt file.
    *
    *  @return the default classification level.
    */
   virtual std::string getDefaultClassification() const = 0;

   /**
    *  Returns the file control security codes.
    *
    *  @return  The file control security codes.
    */
   virtual const std::vector<std::string>& getFileControls() const = 0;

   /**
    *  Returns the file downgrade security codes.
    *
    *  @return  The file downgrade security codes.
    */
   virtual const std::vector<std::string>& getFileDowngrades() const = 0;

   /**
    *  Returns the declassification type security codes.
    *
    *  @return  The declassification type security codes.
    */
   virtual const std::vector<std::string>& getDeclassificationTypes() const = 0;

   /**
    *  Returns the classification reason security codes.
    *
    *  @return  The classification reason security codes.
    */
   virtual const std::vector<std::string>& getClassificationReasons() const = 0;

   /**
    *  Returns the declassification exemptions security codes.
    *
    *  @return  The declassification exemptions security codes.
    */
   virtual const std::vector<std::string>& getDeclassificationExemptions() const = 0;

   /**
    *  Returns the file releasing security codes.
    *
    *  @return  The file releasing security codes.
    */
   virtual const std::vector<std::string>& getFileReleasing() const = 0;

   /**
    *  Returns the country code security codes.
    *
    *  @return  The country code security codes.
    */
   virtual const std::vector<std::string>& getCountryCodes() const = 0;

   /**
    *  Returns the system security codes.
    *
    *  @return  The system security codes.
    */
   virtual const std::vector<std::string>& getSystems() const = 0;

   /**
    *  Returns the codeword security codes.
    *
    *  @return  The codeword security codes.
    */
   virtual const std::vector<std::string>& getCodewords() const = 0;

   /**
    *  Provides a color taken from a color table
    *
    *  This method returns a ColorType from an autocolor.clu file based on the index
    *
    *  @param   color_index
    *           The index value of the color in the autocolor file.
    *
    *  @return  The color in the autocolor file.  A red ColorType is returned if
    *           there is a failure.
    */
   virtual ColorType getAutoColor(int color_index) const = 0;

   /**
    *  Returns the size of the largest free contiguous memory block.
    *
    *  Windows fragments memory with DLL's statically mapped throughout
    *  the process space.  This method walks the virtual memory and 
    *  determine the largest contiguous free block of memory.  UNIX
    *  platforms do not have this limitation and for UNIX this method returns
    *  the available virtual memory.
    *
    *  @return  The size of the largest block possible in bytes.
    */
   virtual size_t getMaxMemoryBlockSize() = 0;

   /**
    *  Returns the total amount of physical memory.
    *
    *  @return  The amount of total physical memory in bytes.
    */
   virtual size_t getTotalPhysicalMemory() = 0;

   /**
    *  Returns the current amount of available virtual memory.
    *
    *  @return  The amount of free virtual memory in bytes.
    */
   virtual size_t getAvailableVirtualMemory() = 0;

   /**
    *  Returns the amount of available disk space.
    *
    *  WARNING: This method is currently unimplemented.  It will
    *  always return 0.
    * 
    *  @param   path 
    *           The directory path used to determine the disk to query.
    *           If NULL, the available space on the disk associated with the 
    *           current working directory is returned.
    *
    *  @return  The amount of free disk space in bytes for the disk
    *           associated with the given path.
    */
   virtual uint64_t getAvailableDiskSpace( std::string path = "" ) = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~UtilityServices() {}
};

#endif
