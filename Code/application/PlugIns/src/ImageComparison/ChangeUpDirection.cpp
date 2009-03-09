/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "ChangeUpDirection.h"
#include "DesktopServices.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "PlugInArgList.h"
#include "PlugInFactory.h"
#include "PlugInManagerServices.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "Undo.h"

PLUGINFACTORY(ChangeUpDirection);

ChangeUpDirection::ChangeUpDirection() : mAbort(false)
{
   setName("Change Up Direction");
   setCreator(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Rotate a data set so there is a new up direction.");
   setDescriptorId("{2e9c4c05-c7c6-4644-b6a7-9ef75e22ccb6}");
   setMenuLocation("[General Algorithms]/Change Up Direction");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

ChangeUpDirection::~ChangeUpDirection()
{
}

bool ChangeUpDirection::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(ProgressArg(), NULL));
   VERIFY(pInArgList->addArg<RasterElement>(DataElementArg()));
   if (isBatch())
   {
      VERIFY(pInArgList->addArg<double>("Rotation"));
   }
   else
   {
      VERIFY(pInArgList->addArg<SpatialDataView>(ViewArg()));
   }
   return true;
}

bool ChangeUpDirection::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pOutArgList->addArg<RasterElement>("Rotated Element"));
   return true;
}

bool ChangeUpDirection::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(ProgressArg()),
      "Rotating data.", "app", "{11adadb9-c133-49de-8cf5-a16372da2578}");

   RasterElement* pData = pInArgList->getPlugInArgValue<RasterElement>(DataElementArg());
   if (pData == NULL)
   {
      progress.report("No data element specified.", 0, ERRORS, true);
      return false;
   }
   double rotation = 0.0;
   SpatialDataView* pOrigView = NULL;
   if (isBatch())
   {
      if (!pInArgList->getPlugInArgValue("Rotation", rotation))
      {
         progress.report("No rotation specified.", 0, ERRORS, true);
         return false;
      }
   }
   else
   {
      pOrigView = pInArgList->getPlugInArgValue<SpatialDataView>(ViewArg());
      if (pOrigView == NULL)
      {
         progress.report("No view specified.", 0, ERRORS, true);
         return false;
      }
      GraphicLayer* pLayer = dynamic_cast<GraphicLayer*>(pOrigView->getActiveLayer());
      if (pLayer == NULL)
      {
         pLayer = dynamic_cast<GraphicLayer*>(pOrigView->getTopMostLayer(ANNOTATION));
      }
      GraphicObject* pArrow = NULL;
      if (pLayer != NULL)
      {
         std::list<GraphicObject*> objects;
         pLayer->getObjects(ARROW_OBJECT, objects);
         if (!objects.empty())
         {
            pArrow = objects.back();
         }
         if (objects.size() > 1)
         {
            progress.report("Multiple arrow objects found. Using the most recently added one.", 0, WARNING, true);
         }
      }
      if (pArrow == NULL)
      {
         progress.report("Unable to locate up direction. Add an arrow annotation and re-run this plugin.",
            0, ERRORS, true);
         return false;
      }
      LocationType ur = pArrow->getUrCorner();
      LocationType ll = pArrow->getLlCorner();
      double xlen = ur.mX - ll.mX;
      double ylen = ur.mY - ll.mY;

      // Initial rotatation value. The 90 degrees is due to the difference
      // in the "0 point" (right vs. up). Also account for explicit rotation
      // of the annotation object. Convert this to radians.
      rotation = (90 + pArrow->getRotation()) * 3.14158 / 180.0;

      // Determine a rotation adjustment based on the bounding box
      rotation += atan2(ylen, xlen);
   }
   if (fabs(rotation) < 0.00001)
   {
      progress.report("Rotation is 0.", 100, NORMAL);
      progress.upALevel();
      return true;
   }

   progress.report("Rotating data.", 10, NORMAL);
   const RasterDataDescriptor* pDesc = static_cast<const RasterDataDescriptor*>(pData->getDataDescriptor());
   VERIFY(pDesc);
   ModelResource<RasterElement> pRotated(RasterUtilities::createRasterElement(pData->getName() + "_rotated",
      pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getBandCount(),
      pDesc->getDataType(), pDesc->getInterleaveFormat()));
   if (pRotated.get() == NULL)
   {
      progress.report("Unable to rotate the data.", 0, ERRORS, true);
      return false;
   }
   std::vector<int> badValues = pDesc->getBadValues();
   if (badValues.empty())
   {
      badValues.push_back(0);
   }
   static_cast<RasterDataDescriptor*>(pRotated->getDataDescriptor())->setBadValues(badValues);
   if (!RasterUtilities::rotate(pRotated.get(), pData, rotation, badValues.front(),
               RasterUtilities::NEAREST_NEIGHBOR, progress.getCurrentProgress(), &mAbort))
   {
      return false;
   }
   std::vector<DimensionDescriptor> bands =
      static_cast<RasterDataDescriptor*>(pRotated->getDataDescriptor())->getBands();
   for (std::vector<DimensionDescriptor>::iterator band = bands.begin(); band != bands.end(); ++band)
   {
      Statistics* pStats = pRotated->getStatistics(*band);
      if (pStats != NULL)
      {
         pStats->setBadValues(badValues);
      }
   }
   pOutArgList->setPlugInArgValue("Rotated Element", pRotated.release());

   if (!isBatch())
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->createWindow(pRotated->getName(), SPATIAL_DATA_WINDOW));
      SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
      if (pView == NULL)
      {
         progress.report("Unable to create view.", 0, ERRORS, true);
         return false;
      }
      pView->setPrimaryRasterElement(pRotated.get());

      UndoLock lock(pView);
      RasterLayer* pLayer = static_cast<RasterLayer*>(pView->createLayer(RASTER, pRotated.get()));
      if (pLayer == NULL)
      {
         progress.report("Unable to create view.", 0, ERRORS, true);
         return false;
      }
   }

   progress.report("Rotation complete.", 100, NORMAL);
   progress.upALevel();
   return true;
}

bool ChangeUpDirection::abort()
{
   mAbort = AlgorithmShell::abort();
   return mAbort;
}