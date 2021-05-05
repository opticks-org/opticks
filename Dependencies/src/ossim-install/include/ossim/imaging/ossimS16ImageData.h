//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Class declaration of ossimS16ImageData.  Specialized image data object for
// signed short data.
//
//*************************************************************************
// $Id: ossimS16ImageData.h 16052 2009-12-08 22:20:40Z dburken $

#ifndef ossimS16ImageData_HEADER
#define ossimS16ImageData_HEADER

#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimNormalizedS16RemapTable.h>

class OSSIMDLLEXPORT ossimS16ImageData : public ossimImageData
{
public:
   ossimS16ImageData(ossimSource* source,
                     ossim_uint32 bands = 1);

   ossimS16ImageData(ossimSource* source,
                    ossim_uint32 bands,
                    ossim_uint32 width,
                    ossim_uint32 height);

   ossimS16ImageData(const ossimS16ImageData &rhs);
   

   /*!
    * Perform object duplication.
    */
   virtual ossimObject* dup()const;

   /*!
    * will fill the entire band with
    * the value.
    */
   void fill(ossim_uint32 band, double value);


   bool isNull(ossim_uint32 offset)const;
   void setNull(ossim_uint32 offset);

   virtual ossimDataObjectStatus validate() const;

   /*!
    * will go to the band and offset and compute the
    * normalized float and return it back to the
    * caller through the result argument.
    */
   virtual void getNormalizedFloat(ossim_uint32 offset,
                                   ossim_uint32 bandNumber,
                                   float& result)const;

   /*!
    * This will assign to this object a normalized
    * value by unnormalizing to its native type.
    */
   virtual void setNormalizedFloat(ossim_uint32 offset,
                                   ossim_uint32 bandNumber,
                                   float input);
   
   /*!
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedFloat(ossimImageData* result)const;
   
   /*!
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedDouble(ossimImageData* result)const;

   /*!
    * Will take the normalized input and convert it
    * to this tile's data type.  Example:  if this
    * tile is of type UCHAR and its input is of type
    * NORALIZED_FLOAT it will unnormalize the data by
    * doing:
    *
    * minPix + normalizedInput*(maxPix-minPix)
    *
    * on a per band basis.
    */
   virtual void unnormalizeInput(ossimImageData* normalizedInput);

   /*!
    * This will compute the average value for the band.
    */
   virtual double computeAverageBandValue(ossim_uint32 bandNumber = 0);

   /*!
    * This will call the compute average band value and then
    * use that in the calculation of:
    
    * It will then do a SUM[(Mean - Actual)^2]/width*height.
    *
    * This is the average variance from the passed in
    * mean.  Basically think of the mean as a completely
    * grey image and we would like to see how this
    * image varies from the passed in mean.
    */
   virtual double computeMeanSquaredError(double meanValue,
                                          ossim_uint32 bandNumber = 0);
  
   virtual void setValue(ossim_int32 x, ossim_int32 y, double color);

   /*!
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   virtual void copyTileToNormalizedBuffer(double* buf) const;
   
   /*!
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   virtual void copyNormalizedBufferToTile(double* buf);

   /*!
    * Will copy this tiles specified band number to the normalized buffer.
    * if the band is out of range then nothing is done and returns.
    */
   virtual void copyTileToNormalizedBuffer(ossim_uint32 band, double* buf) const;
   virtual void copyTileToNormalizedBuffer(ossim_uint32 band, float* buf)const;
   
   /*!
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   virtual void copyTileToNormalizedBuffer(float* buf) const;
   
   /*!
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   virtual void copyNormalizedBufferToTile(float* buf);
   
   /*!
    * Will copy the normalized buffer to this tiles
    * specified band.  If band is out of range then nothing
    * is done and returns.
    */
   virtual void copyNormalizedBufferToTile(ossim_uint32 band,
                                           double* buf);
   virtual void copyNormalizedBufferToTile(ossim_uint32 band,
                                           float* buf);

   /** @return The first non-null index of the normalized remap table. */
   virtual ossim_float64 getMinNormalizedPix() const;
   
protected:
   virtual ~ossimS16ImageData();
   ossimS16ImageData();

private:

   static const ossimNormalizedS16RemapTable m_remapTable;

TYPE_DATA
};

inline ossim_float64 ossimS16ImageData::getMinNormalizedPix() const
{
   return m_remapTable[1];
}

#endif
