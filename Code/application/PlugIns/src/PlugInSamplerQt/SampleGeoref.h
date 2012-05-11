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

#include "Animation.h"
#include "AttachmentPtr.h"
#include "GeoreferenceShell.h"
#include "SampleGeorefGui.h"

/**
 * Sample GeoreferenceAlgorithm plugin.
 *
 * Latitude and Longitude are factored by a user-specified amount.
 */
class SampleGeoref : public GeoreferenceShell
{
public:
   SampleGeoref();
   ~SampleGeoref();

   bool setInteractive();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

   LocationType geoToPixel(LocationType geo, bool* pAccurate = NULL) const;
   LocationType pixelToGeo(LocationType pixel, bool* pAccurate = NULL) const;

   bool canHandleRasterElement(RasterElement *pRaster) const;

   QWidget *getGui(RasterElement *pRaster);

   bool validateGuiInput() const;

   void animationFrameChanged(Subject &subject, const std::string &signal, const boost::any &data);

private:
   SampleGeoref(const SampleGeoref& rhs);
   SampleGeoref& operator=(const SampleGeoref& rhs);

   static LocationType rotate(LocationType loc, double rad);

   int mXSize;
   int mYSize;
   double mXScale;
   double mYScale;

   bool mExtrapolate;

   double mFrames;
   unsigned int mCurrentFrame;

   bool mRotate;

   AttachmentPtr<Animation> mpAnimation;
   RasterElement* mpRaster;


   SampleGeorefGui* mpGui;
};

#endif
