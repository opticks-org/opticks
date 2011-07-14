/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppVersion.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "GeoMosaicChip.h"
#include "GraphicGroup.h"
#include "LayerList.h"
#include "Layer.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <limits>
#include <vector>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>


REGISTER_PLUGIN_BASIC(OpticksGeoMosaic, GeoMosaicChip);

namespace
{
   class ChipStruct
   {
   public:
      ChipStruct() : mAccessor(NULL, NULL), mStartX(0), mStartY(0), mEndX(0), mEndY(0) {}

      ChipStruct(const ChipStruct& rhs) : mAccessor(NULL, NULL)
      {
         mAccessor = rhs.mAccessor;
         mStartX = rhs.mStartX;
         mStartY = rhs.mStartY;
         mEndX = rhs.mEndX;
         mEndY = rhs.mEndY;
      }

      ChipStruct& operator=(const ChipStruct& rhs)
      {
         if (this != &rhs)
         {
            mAccessor = rhs.mAccessor;
            mStartX = rhs.mStartX;
            mStartY = rhs.mStartY;
            mEndX = rhs.mEndX;
            mEndY = rhs.mEndY;
         }
         return *this;
      }

      DataAccessor mAccessor;
      double mStartX;
      double mStartY;
      double mEndX;
      double mEndY;
   };

   inline bool orderedIsBetween(double x, double x1, double x2)
   {
      double e = 0.0001;
      return (x1 - e <= x) && (x <= x2 + e);
   }

   inline bool isContainedIn(double x, double y, double x1, double y1, double x2, double y2)
   {
      return orderedIsBetween(x, x1, x2) && orderedIsBetween(y, y1, y2);
   }
}

GeoMosaicChip::GeoMosaicChip()
{
   setDescriptorId("{108ECF1B-193F-4520-95DF-863926C7E47E}");
   setName("Mosaic Chip");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Mosaic Chip");
   setMenuLocation("[General Algorithms]\\Mosaic Chip");
   allowMultipleInstances(true);
   setAbortSupported(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GeoMosaicChip::~GeoMosaicChip()
{}

bool GeoMosaicChip::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription());
   pInArgList->addArg<SpatialDataView>(Executable::ViewArg(), "View from which to chip.");
   return true;
}

bool GeoMosaicChip::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL, "The chipped element");
   return true;
}

bool GeoMosaicChip::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("MosaicChip", "app", "95034AC8-EC4C-4CB6-9089-4EF1DCBB41C3");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (pView == NULL)
   {
      std::string msg = "A view must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }

   Service<ModelServices> pModel;
   Service<DesktopServices> pDesktop;
   RasterElement* pPrimaryRaster = pView->getLayerList()->getPrimaryRasterElement();
   // This is the cube from which the AOIs will be selected
   std::vector<DataElement*> pAois = pModel->getElements(pPrimaryRaster, TypeConverter::toString<AoiElement>());
   if (pAois.empty())
   {
      std::string msg = "Raster Element does not contain an AOI.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   QStringList aoiNames;
   for (std::vector<DataElement*>::iterator it = pAois.begin(); it != pAois.end(); ++it)
   {
      aoiNames << QString::fromStdString((*it)->getName());
   }
   bool ok;
   QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
      "Select an AOI", "Select an AOI for processing", aoiNames, 0, true, &ok);
   // select AOI
   if (!ok)
   {
      std::string msg = getName() + " has been aborted.";
      pStep->finalize(Message::Abort, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ABORT);
      }
      return false;
   }

   AoiElement* pAoi = NULL;
   std::string strAoi = aoi.toStdString();
   for (std::vector<DataElement*>::iterator it = pAois.begin(); it != pAois.end(); ++it)
   {
      if ((*it)->getName() == strAoi)
      {
         pAoi = static_cast<AoiElement*>(*it);
         break;
      }
   }
   if (pAoi == NULL)
   {
      std::string msg = "Invalid AOI.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }

#pragma message(__FILE__ "(" STRING(__LINE__) ") The class BitMask does not support values smaller than 0, 0 for any masking")
   //Get the bounding info and always assume that the region is rectangular
   GraphicGroup* pGroup = pAoi->getGroup();
   LocationType Ll = pGroup->getLlCorner();
   LocationType Ur = pGroup->getUrCorner();
   // These values represent the bounding box of the rectangle and the pixel count
   int aoiStartX = floor(Ll.mX);
   int aoiStartY = floor(Ll.mY);
   int aoiEndX = floor(Ur.mX);
   int aoiEndY = floor(Ur.mY);
   int count = (aoiEndX - aoiStartX + 1) * (aoiEndY - aoiStartY + 1);
   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList == NULL)
   {
      return false;
   }

   RasterDataDescriptor* pPrimaryDesc = dynamic_cast<RasterDataDescriptor*>(pPrimaryRaster->getDataDescriptor());
   if (pPrimaryDesc == NULL)
   {
      return false;
   }
   std::string chipName = pView->getName() + " Chip";
   unsigned int numRows = static_cast<unsigned int>(aoiEndY - aoiStartY + 1);
   unsigned int numColumns = static_cast<unsigned int>(aoiEndX - aoiStartX + 1);
   unsigned int numBands = pPrimaryDesc->getBandCount();
   EncodingType outputDataType = pPrimaryDesc->getDataType();
   ModelResource<RasterElement> pSubCubeRaster = 
      ModelResource<RasterElement>(RasterUtilities::createRasterElement(chipName, numRows, numColumns, numBands,
      outputDataType, BIP, true, NULL));
   if (pSubCubeRaster.get() == NULL)
   {
      pSubCubeRaster = 
         ModelResource<RasterElement>(RasterUtilities::createRasterElement(chipName, numRows, numColumns, numBands,
            outputDataType, BIP, false, NULL));
      if (pSubCubeRaster.get() == NULL)
      {
         std::string msg = "Raster Element could not be created.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         return false;
      }
   }

   bool invalidData = false;
   std::vector<ChipStruct> chips;
   std::vector<Layer*> layers;
   pLayerList->getLayers(RASTER, layers);
   std::reverse(layers.begin(), layers.end());
   for (unsigned int idx = 0; idx < layers.size(); ++idx)
   {
      Layer* pLayer = layers[idx];
      if (pLayer == NULL || !pView->isLayerDisplayed(pLayer))
      {
         continue;
      }
      ChipStruct item;
      RasterElement* pElement = dynamic_cast<RasterElement*>(pLayer->getDataElement());
      if (pElement == NULL)
      {
         invalidData = true;
         break;
      }
      RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      if (pDesc == NULL)
      {
         invalidData = true;
         break;
      }
      if (pDesc->getBandCount() != numBands)
      {
         chips.clear(); //required to free resources held by DataAccessors
         std::string msg = pElement->getName() + " has " +
            StringUtilities::toDisplayString(pDesc->getBandCount()) + " bands when it "
            "must match the primary raster element which has " + StringUtilities::toDisplayString(numBands) +
            " bands for chipping to be permitted.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         invalidData = true;
         break;
      }
      if (pDesc->getDataType() != outputDataType)
      {
         chips.clear(); //required to free resources held by DataAccessors
         std::string msg = pElement->getName() + " has an encoding type of " +
            StringUtilities::toDisplayString(pDesc->getDataType()) + " it "
            "must match the primary raster element which has an encoding type of " +
            StringUtilities::toDisplayString(outputDataType) +
            " for chipping to be permitted.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         invalidData = true;
         break;
      }
      FactoryResource<DataRequest> pRequest;
      pRequest->setInterleaveFormat(BIP);
      DataAccessor origAccessor = pElement->getDataAccessor(pRequest.release());
      if (!origAccessor.isValid())
      {
         chips.clear(); //required to free resources held by DataAccessors
         std::string msg = "Could not access required data.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         break;
      }
      item.mAccessor = origAccessor;
      item.mStartX = layers[idx]->getXOffset();
      item.mStartY = layers[idx]->getYOffset();
      item.mEndX = pDesc->getColumnCount() - 1 + item.mStartX;
      item.mEndY = pDesc->getRowCount() - 1 + item.mStartY;
      chips.push_back(item);
   }
   if (invalidData || chips.empty())
   {
      chips.clear(); //required to free resources held by DataAccessors
      return false;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BIP);
   pRequest->setWritable(true);
   DataAccessor pSubAccessor = pSubCubeRaster->getDataAccessor(pRequest.release());

   void* pDestData = NULL;
   void* pSourceData = NULL;
   void* pValue = NULL;
   RasterElement* pElement = NULL;
   RasterDataDescriptor* pDesc = NULL;
   int currentCount = 0;
   int currentPercent = 0;
   int savePercent = 0;
   unsigned int bytesToCopy = numBands * RasterUtilities::bytesInEncoding(outputDataType);
   for (int row = aoiStartY; row <= aoiEndY; ++row)
   {
      for (int col = aoiStartX; col <= aoiEndX; ++col)
      {
         for (unsigned int idx = 0; idx < chips.size(); ++idx)
         {            
            if (isContainedIn(col, row, chips[idx].mStartX, chips[idx].mStartY, chips[idx].mEndX, chips[idx].mEndY))
            {
               if (!pSubAccessor.isValid())
               {
                  chips.clear(); //required to free resources held by DataAccessors
                  return false;
               }
               if (!chips[idx].mAccessor.isValid())
               {
                  chips.clear(); //required to free resources held by DataAccessors
                  return false;
               }
               chips[idx].mAccessor->toPixel(row - chips[idx].mStartY, col - chips[idx].mStartX);
               pSubAccessor->toPixel(row - aoiStartY, col - aoiStartX);
               pSourceData = chips[idx].mAccessor->getColumn();
               pValue = pSubAccessor->getColumn();
               memcpy(pValue, pSourceData, bytesToCopy);
               break;
            }
         }
         ++currentCount;
         currentPercent = 100.0 * currentCount / count;
         if (currentPercent != savePercent)
         {
            if (isAborted())
            {
               pProgress->updateProgress("User aborted", 0, ABORT); 
               chips.clear(); //required to free resources held by DataAccessors
               return false;
            }
            savePercent = currentPercent;
            pProgress->updateProgress("Chipping...", currentPercent, NORMAL);
         }
      }
   }
   chips.clear(); //required to free resources held by DataAccessors

   // Create the SpatialDataView for the results
   SpatialDataWindow* mpWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(chipName, SPATIAL_DATA_WINDOW));
   if (mpWindow == NULL)
   {
      std::string msg = "Could not create the window.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   SpatialDataView* pCurView = mpWindow->getSpatialDataView();
   if (pCurView == NULL)
   {
      std::string msg = "Could not create the view.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   pCurView->setPrimaryRasterElement(pSubCubeRaster.get());
   pCurView->createLayer(RASTER, pSubCubeRaster.get());

   pOutArgList->setPlugInArgValue("Result", pSubCubeRaster.release());

   pProgress->updateProgress("Done", 100, NORMAL);
   return true;
}