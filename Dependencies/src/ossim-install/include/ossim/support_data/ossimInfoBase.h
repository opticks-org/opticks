//----------------------------------------------------------------------------
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Base class for Info object.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef ossimInfoBase_HEADER
#define ossimInfoBase_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimReferenced.h>
#include <iosfwd>
#include <ossim/base/ossimIosFwd.h>
#include <memory>
#include <string>

class ossimFilename;
class ossimKeywordlist;

/**
 * @brief Info Base.
 *
 * This is the base class for all info objects.  The purpose of an Info object
 * is to dump whatever info is available for a given file name to user.
 */
class OSSIM_DLL ossimInfoBase : public ossimReferenced
{
public:
   
   /** default constructor */
   ossimInfoBase();


   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);

   /**
    * @brief open method.
    *
    * @param str File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(std::shared_ptr<ossim::istream>& str,
                     const std::string& connectionString);

   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const = 0;

   /**
    * @brief Sets the overview flag.
    * @param flag If true overview info is processed also.
    */
   void setProcessOverviewFlag(bool flag);
   
   /**
    * @brief Method to get the overview flag.
    * @return The overview flag.
    */
   bool getProcessOverviewFlag() const;

   /**
    * @brief Method to dump info to a keyword list.
    * @param kwl The keyword list to initialize.
    * @return true on success, false on error.
    */
   virtual bool getKeywordlist(ossimKeywordlist& kwl)const;

   /**
    * @brief Method to dump info to a keyword list.
    * @param kwl The keyword list to initialize.
    * @param entryIndex Entry to print.  Not supported by all info objects
    * yet.
    * @return true on success, false on error.
    */
   virtual bool getKeywordlist(ossimKeywordlist& kwl,
                               ossim_uint32 entryIndex)const;

protected:
   /** virtual destructor */
   virtual ~ossimInfoBase();
   
   bool theOverviewFlag; // If true overview information should be processed.
   
};

#endif /* End of "#ifndef ossimInfoBase_HEADER" */
