//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for ossimImageMetaData.
// 
//*******************************************************************
//  $Id: ossimImageMetaData.h 21527 2012-08-26 16:50:49Z dburken $
#ifndef ossimImageMetaData_HEADER
#define ossimImageMetaData_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <iosfwd>
#include <string>
#include <vector>

class ossimKeywordlist;

class OSSIM_DLL ossimImageMetaData
{
public:
   ossimImageMetaData();
   ossimImageMetaData(ossimScalarType aType,
                      ossim_uint32 numberOfBands);
   ossimImageMetaData(const ossimImageMetaData& rhs);

   const ossimImageMetaData& operator=(const ossimImageMetaData& rhs);
   
   ~ossimImageMetaData();
   
   void clear();

   void setDefaultsForArrays();
   
   void setNumberOfBands(ossim_uint32 numberOfBands);
   
   ossim_uint32 getNumberOfBands()const;

   /**
    * @return string reference containing rgb band order, e.g. "(2,1,0)".
    * Will be an emptry string if not set.
    */
   const std::string& getRgbBands() const;

   /**
    * @brief Convenience method to get the zero based rgb output band list.
    *
    * This method returns true on success; false, if rgb_bands key is not set
    * in dot omd file.
    *
    * @note: Current ossimImageMetaData logic requires key "number_bands" to
    * be set and at least 3.
    * 
    * @param bandList Initialized by this on success.
    * 
    * @return false
    */
   bool getRgbBands(std::vector<ossim_uint32>& bandList) const;
   
   void setScalarType(ossimScalarType aType);
   
   ossimScalarType getScalarType()const;

   /** @return The bytes per pixel. This is for a single band. */
   ossim_uint32 getBytesPerPixel() const;
   
   double getMinPix(ossim_uint32 band)const;
   
   void setMinPix(ossim_uint32 band, double pix);
   
   void setMaxPix(ossim_uint32 band, double pix);
   
   void setNullPix(ossim_uint32 band, double pix);
   
   double getMaxPix(ossim_uint32 band)const;
   
   double getNullPix(ossim_uint32 band)const;
   
   const double* getMinPixelArray()const;
   
   const double* getMaxPixelArray()const;
   
   const double* getNullPixelArray()const;

   void setMinValuesValid(bool flag);
   
   void setMaxValuesValid(bool flag);
   
   void setNullValuesValid(bool flag);

   bool getMinValuesValidFlag()const;

   bool getMaxValuesValidFlag()const;

   bool getNullValuesValidFlag()const;

   bool isValid()const;
   
   bool loadState(const ossimKeywordlist& kwl,
                  const char* prefix=0);
   bool saveState(ossimKeywordlist& kwl,
                  const char* prefix=0)const;

   /**
    * @brief Method to update band values.
    *
    * Assumes a previous initialization and does not error out if band data is
    * not found.  This does NOT clear the object prior to loading like the
    * loadState(...) method.  Can be used to update min/max values from a
    * "compute min max".  
    *
    * @param kwl Keyword list to initialize from.
    *
    * @param prefix Prefix, e.g. "image0.".
    */
   void updateMetaData( const ossimKeywordlist& kwl,
                        const std::string& prefix );

   /**
    * @brief Print method.
    * @return std::ostream&
    */
   std::ostream& print(std::ostream& out) const;

   /**
    * @note  Since the print method is virtual, derived classes only need
    *        to implement that, not an addition operator<<.
    */
   friend OSSIM_DLL std::ostream& operator<<(std::ostream& out,
                                             const ossimImageMetaData& obj);
   
private:

   /**
    * Looks for ossimKeywordNames::NUMBER_BANDS_KW, if not found looks for.
    */
   ossim_uint32 getBandCount(const ossimKeywordlist& kwl,
                             const std::string& prefix) const;
   
   double*         theNullPixelArray;
   double*         theMinPixelArray;
   double*         theMaxPixelArray;
  
   bool            theMinValuesValidFlag;
   bool            theMaxValuesValidFlag;
   bool            theNullValuesValidFlag;
   
   ossimScalarType theScalarType;
   ossim_uint32    theBytesPerPixel;
   ossim_uint32    theNumberOfBands;

   //---
   // String containing rgb bands, e.g. (2,1,0);
   // Added to allow tiled tiffs to set band selection correctly by picking
   // up order from omd file.
   //---
   std::string theRgbBands; 
};

#endif /* #ifndef ossimImageMetaData_HEADER */
