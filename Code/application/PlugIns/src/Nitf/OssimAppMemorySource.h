/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef OSSIMAPPMEMORYSOURCE_H
#define OSSIMAPPMEMORYSOURCE_H

#include <ossim/imaging/ossimImageSource.h>

class RasterElement;
class RasterFileDescriptor;

class OssimAppMemorySource : public ossimImageSource
{
public:
   OssimAppMemorySource(RasterElement& cube, const RasterFileDescriptor &exportDescriptor);

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& rect, ossim_uint32 resLevel = 0);
   
   virtual ossimScalarType getOutputScalarType() const;

   virtual ossim_uint32 getTileWidth() const;

   virtual ossim_uint32 getTileHeight() const;

   virtual ossimIrect getBoundingRect(ossim_uint32 resLevel = 0) const;

   ossim_uint32 getNumberOfInputBands() const;

   bool canConnectMyInputTo(ossim_int32 inputIndex, const ossimConnectableObject* pObj) const;

   virtual void initialize() {}

// override virtual methods here that describe
// min max pixels, invalid/null pixel values
// projection geometry
// bands
// could use the general raster info class.
private:
   OssimAppMemorySource& operator=(const OssimAppMemorySource& rhs);

   RasterElement& mCube;
   const RasterFileDescriptor& mExportDescriptor;
   unsigned int mColSkipFactor;
   unsigned int mRowSkipFactor;
};

#endif
