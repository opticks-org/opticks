//*******************************************************************
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef ossimNitfImageBandV2_0_HEADER
#define ossimNitfImageBandV2_0_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimNitfImageBand.h>
#include <ossim/support_data/ossimNitfImageLutV2_0.h>
#include <string>


class OSSIM_DLL ossimNitfImageBandV2_0 : public ossimNitfImageBand
{
public:
   
   /** default constructor */
   ossimNitfImageBandV2_0();

   /** virtual destructory */
   virtual ~ossimNitfImageBandV2_0();
   
   virtual void parseStream(ossim::istream& in);
   virtual void writeStream(ossim::ostream& out);

   /**
    * @brief print method that outputs a key/value type format adding prefix
    * to keys.
    * @param out Stream to output to.
    * @param prefix Like "image0."
    * @param band zero based band.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string(),
                               ossim_uint32 band=0) const;   
   
   virtual ossim_uint32 getNumberOfLuts()const;
   virtual const ossimRefPtr<ossimNitfImageLut> getLut(ossim_uint32 idx)const;
   virtual ossimRefPtr<ossimNitfImageLut> getLut(ossim_uint32 idx);

   /** @return The band representation as an ossimString. */
   virtual ossimString getBandRepresentation()const;

   /**
    * Sets the band representation.
    *
    * @param rep The band representation.
    */
   virtual void setBandRepresentation(const ossimString& rep);

   /** @return The band significance as an ossimString. */
   virtual ossimString getBandSignificance()const;

   /**
    * Sets the band significance.
    *
    * @param rep The band significance.
    */
   virtual void setBandSignificance(const ossimString& rep);

   bool loadState(const ossimKeywordlist& kwl, const char* prefix, ossim_uint32 index=0);

protected:
   void clearFields();
   void printLookupTables(std::ostream& out)const;
   /*!
    * FIELD: IREPBAND,,
    * Is a required 2 byte field.  When theNumberOfBands is
    * 1 this field will contain all spaces.
    */
   char theBandRepresentation[3];

   /*!
    * FIELD:  ISUBCATnn
    * Is a required 6 byte field.
    */
   char theBandSignificance[7];

   /*!
    * FIELD:  IFCnn
    * Is an required 1 byte field.  Will be N
    */
   char theBandImageFilterCondition[2];

   /*!
    * FIELD:  IMFLTnn
    * is a required 3 byte field.  This is a reserved field
    */
   char theBandStandardImageFilterCode[4];

   /*!
    * FIELD:  NLUTSnn
    * This is a required 1 byte field.  Has value 0-4
    */
   char theBandNumberOfLuts[2];

   /*!
    * NELUTnn
    * This is a conditional field.
    */
   char theBandNumberOfLutEntries[6];

   std::vector<ossimRefPtr<ossimNitfImageLut> > theLookupTables;
};

#endif
