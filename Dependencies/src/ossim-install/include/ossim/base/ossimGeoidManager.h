//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class ossimGeoidManager. Maintains
//   a list of geoids.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: ossimGeoidManager.h 22900 2014-09-30 09:56:11Z dburken $

#ifndef ossimGeoidManager_HEADER
#define ossimGeoidManager_HEADER 1

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimGeoid.h>
#include <vector>

/*****************************************************************************
 *
 * CLASS: ossimGeoidManager 
 *
 *****************************************************************************/
class OSSIMDLLEXPORT ossimGeoidManager : public ossimGeoid
{
public:


   virtual ~ossimGeoidManager();
      
   /**
    * Implements singelton pattern:
    */
   static ossimGeoidManager* instance();

   
   /**
    * Permits initialization of geoids from directory name. Should never be
    * called since called on specific geoid types:
    */
   virtual bool open(const ossimFilename& dir, ossimByteOrder byteOrder);

   /**
    *  @return The offset from the ellipsoid to the geoid or ossim::nan()
    *  if grid does not contain the point.
    */
   virtual double offsetFromEllipsoid(const ossimGpt& gpt);

   /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error. DO NOTHING
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;

   /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Permits adding additional geoids to the list: 
    */
   virtual void addGeoid(ossimRefPtr<ossimGeoid> geoid, bool toFrontFlag=false);

   /**
    * @brief Gets the geoid for a given point. Typically first geoid in the
    * list.
    * @param gpt
    * @return ossimRefPtr<ossimGeoid> which can hold null pointer if there is
    * no coverage for the point.
    */
   ossimRefPtr<ossimGeoid> getGeoidForPoint( const ossimGpt& gpt );

   ossimGeoid* findGeoidByShortName(const ossimString& shortName, bool caseSensitive=true);

  /**
   * Permits to clear the GeoidList
   */
   virtual void clear();

   /** @return The number of geoids loaded. */
   ossim_uint32 getNumberOfGeoids() const;

   /**
    * @brief Gets the geoid for index.
    * @param index
    * @return ossimRefPtr<ossimGeoid> which can hold null pointer if the index
    * is out of range or no geoids loaded.
    */
   ossimRefPtr<ossimGeoid> getGeoid( ossim_uint32 index );

private:
   /**
    *  Private constructor.  Use "instance" method.
    */
   ossimGeoidManager();

   //static ossimGeoidManager* theInstance;
   mutable std::vector< ossimRefPtr<ossimGeoid> > theGeoidList;
   
   // will use this as a identity if one wants but don't want it part of the internal list
   //
   ossimRefPtr<ossimGeoid> theIdentityGeoid;
   
   TYPE_DATA
};

#endif
