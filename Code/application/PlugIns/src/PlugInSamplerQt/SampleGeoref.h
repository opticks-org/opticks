/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAMPLEGEOREF_H
#define SAMPLEGEOREF_H

#include "GeoreferenceShell.h"

class SampleGeorefGui;

/**
 * Sample GeoreferenceAlgorithm plugin.
 *
 * Latitude and Longitude are factored by a user-specified amount.
 */
class SampleGeoref : public GeoreferenceShell
{
public:
   SampleGeoref();
   virtual ~SampleGeoref();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const;
   QWidget* getWidget(RasterDataDescriptor* pDescriptor);
   LocationType geoToPixel(LocationType geo, bool* pAccurate = NULL) const;
   LocationType pixelToGeo(LocationType pixel, bool* pAccurate = NULL) const;

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

private:
   SampleGeoref(const SampleGeoref& rhs);
   SampleGeoref& operator=(const SampleGeoref& rhs);

   int mXSize;
   int mYSize;
   double mXScale;
   double mYScale;
   bool mExtrapolate;

   RasterElement* mpRaster;
   SampleGeorefGui* mpGui;
};

#endif
