/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeoreferenceDescriptor.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SampleGeoref.h"
#include "SampleGeorefGui.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <boost/any.hpp>

XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksPlugInSamplerQt, SampleGeoref);

SampleGeoref::SampleGeoref() :
   mXSize(10),
   mYSize(5),
   mXScale(1.0),
   mYScale(1.0),
   mExtrapolate(false),
   mpRaster(NULL),
   mpGui(NULL)
{
   setName("SampleGeoref");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Simple Georeference plugin");
   setProductionStatus(false);
   setDescriptorId("{57B86486-3B3B-465a-B74F-53A17290D982}");
   allowMultipleInstances(true);
   executeOnStartup(false);
   destroyAfterExecute(false);
}

SampleGeoref::~SampleGeoref()
{
   delete mpGui;
}

bool SampleGeoref::getInputSpecification(PlugInArgList*& pArgList)
{
   if (GeoreferenceShell::getInputSpecification(pArgList) == false)
   {
      return false;
   }

   // Do not set a default value in the input args so that the georeference
   // descriptor values will be used if the arg value is not set
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<int>("XSize", "The number of degrees to specify for latitude.  If not specified, "
      "the x-size value contained in the " + Executable::DataElementArg() + " input will be used.") == true);
   VERIFY(pArgList->addArg<int>("YSize", "The number of degrees to specify for longitude.  If not specified, "
      "the y-size value contained in the " + Executable::DataElementArg() + " input will be used.") == true);
   VERIFY(pArgList->addArg<bool>("Extrapolate", "Whether to extrapolate geographic coordinates outside of "
      "the raster element extents.  If not specified, the extrapolate value contained in the " +
      Executable::DataElementArg() + " input will be used.") == true);

   return true;
}

bool SampleGeoref::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   // Do any kind of setup we need before converting coordinates.
   // In this case, get our X and Y factors.

   StepResource pStep("Run Sample Georef", "app", "CFCB8AA9-D504-42e9-86F0-547DF9B4798A");
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   FAIL_IF(!isBatch(), "Interactive mode is not supported.", return false);

   mpRaster = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   FAIL_IF(mpRaster == NULL, "Could not find the raster element", return false);

   // Get the values from the input args
   bool xSizeArgSet = pInArgList->getPlugInArgValue("XSize", mXSize);
   bool ySizeArgSet = pInArgList->getPlugInArgValue("YSize", mYSize);
   bool extrapolateArgSet = pInArgList->getPlugInArgValue("Extrapolate", mExtrapolate);

   // If the arg values are not set, get the values from the georeference descriptor;
   // otherwise update the georeference descriptor with the arg values
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   FAIL_IF(pDescriptor == NULL, "Could not get the data descriptor.", return false);

   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      if (xSizeArgSet == false)
      {
         mXSize = dv_cast<int>(pGeorefDescriptor->getAttributeByPath("SampleGeoref/XSize"), mXSize);
      }

      if (ySizeArgSet == false)
      {
         mYSize = dv_cast<int>(pGeorefDescriptor->getAttributeByPath("SampleGeoref/YSize"), mYSize);
      }

      if (extrapolateArgSet == false)
      {
         mExtrapolate = dv_cast<bool>(pGeorefDescriptor->getAttributeByPath("SampleGeoref/Extrapolate"), mExtrapolate);
      }
   }

   // Calculate the scale for the georeference
   unsigned int rows = pDescriptor->getRowCount();
   unsigned int cols = pDescriptor->getColumnCount();

   mXScale = static_cast<double>(mXSize) / rows;
   mYScale = static_cast<double>(mYSize) / cols;

   // Set the georeference plug-in into the raster element
   mpRaster->setGeoreferencePlugin(this);

   // Update the georeference descriptor with the current georeference parameters if necessary
   if (pGeorefDescriptor != NULL)
   {
      // Georeference plug-in
      const std::string& plugInName = getName();
      pGeorefDescriptor->setGeoreferencePlugInName(plugInName);

      // X-size
      if (xSizeArgSet == true)
      {
         pGeorefDescriptor->setAttributeByPath("SampleGeoref/XSize", mXSize);
      }

      // Y-size
      if (ySizeArgSet == true)
      {
         pGeorefDescriptor->setAttributeByPath("SampleGeoref/YSize", mYSize);
      }

      // Extrapolate
      if (extrapolateArgSet == true)
      {
         pGeorefDescriptor->setAttributeByPath("SampleGeoref/Extrapolate", mExtrapolate);
      }
   }

   pStep->finalize(Message::Success);
   return true;
}

unsigned char SampleGeoref::getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const
{
   // This simple plug-in can handle any RasterElement.
   // A real plug-in would probably need to check some parameter of the data here.

   return Georeference::CAN_NOT_GEOREFERENCE + 1;  // Return the lowest possible value that can be georeferenced so
                                                   // that the sample plug-in will always be the last plug-in selected
}

QWidget* SampleGeoref::getWidget(RasterDataDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   // Create the widget
   if (mpGui == NULL)
   {
      mpGui = new SampleGeorefGui();
   }

   // Set the georeference data into the widget for the given raster data
   GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
   if (pGeorefDescriptor != NULL)
   {
      mpGui->setGeoreferenceDescriptor(pGeorefDescriptor);
   }

   return mpGui;
}

LocationType SampleGeoref::geoToPixel(LocationType geo, bool* pAccurate) const
{
   LocationType pixel;
   pixel.mX = geo.mY / mYScale;
   pixel.mY = geo.mX / mXScale;

   // actual Georeference subclass will need to implement means to check for extrapolation
   // and to set pAccurate appropriately.
   if (pAccurate != NULL && mpRaster != NULL)
   {
      *pAccurate = false;
      RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDesc != NULL)
      {
         bool outsideCols = pixel.mX < 0.0 || pixel.mX > static_cast<double>(pDesc->getColumnCount());
         bool outsideRows = pixel.mY < 0.0 || pixel.mY > static_cast<double>(pDesc->getRowCount());
         bool locationExtrapolated = outsideCols || outsideRows;
         *pAccurate = (locationExtrapolated ? mExtrapolate : true);
      }
   }

   return pixel;
}

LocationType SampleGeoref::pixelToGeo(LocationType pixel, bool* pAccurate) const
{
   LocationType geo;
   geo.mX = pixel.mY * mXScale;
   geo.mY = pixel.mX * mYScale;

   // actual Georeference subclass will need to implement means to check for extrapolation
   // and to set pAccurate appropriately.
   if (pAccurate != NULL && mpRaster != NULL)
   {
      *pAccurate = false;
      RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
      if (pDesc != NULL)
      {
         bool outsideCols = pixel.mX < 0.0 || pixel.mX > static_cast<double>(pDesc->getColumnCount());
         bool outsideRows = pixel.mY < 0.0 || pixel.mY > static_cast<double>(pDesc->getRowCount());
         bool locationExtrapolated = outsideCols || outsideRows;
         *pAccurate = (locationExtrapolated ? mExtrapolate : true);
      }
   }

   return geo;
}

bool SampleGeoref::serialize(SessionItemSerializer& serializer) const
{
   if (mpRaster == NULL)
   {
      // execute() has not yet been called so there is no need to serialize any parameters
      return true;
   }

   XMLWriter writer("SampleGeoref");
   writer.addAttr("rasterId", mpRaster->getId());
   writer.addAttr("xSize", mXSize);
   writer.addAttr("ySize", mYSize);
   writer.addAttr("xScale", mXScale);
   writer.addAttr("yScale", mYScale);
   writer.addAttr("extrapolationAccurate", mExtrapolate);
   return serializer.serialize(writer);
}

bool SampleGeoref::deserialize(SessionItemDeserializer& deserializer)
{
   if (deserializer.getBlockSizes().empty() == true)
   {
      // The plug-in was serialized before execute() was called
      return true;
   }

   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, "SampleGeoref");
   if (pRootElement)
   {
      std::string windowId = A(pRootElement->getAttribute(X("rasterId")));
      mpRaster = dynamic_cast<RasterElement*>(Service<SessionManager>()->getSessionItem(windowId));
      if (mpRaster != NULL)
      {
         mXSize = atoi(A(pRootElement->getAttribute(X("xSize"))));
         mYSize = atoi(A(pRootElement->getAttribute(X("ySize"))));
         mXScale = atof(A(pRootElement->getAttribute(X("xScale"))));
         mYScale = atof(A(pRootElement->getAttribute(X("yScale"))));
         mExtrapolate = StringUtilities::fromXmlString<bool>(
            A(pRootElement->getAttribute(X("extrapolationAccurate"))));
         return true;
      }
   }
   return false;
}
