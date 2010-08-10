/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "Tutorial4.h"
#include "TypeConverter.h"
#include <limits>
#include <vector>
#include <QtCore/QStringList>
#include <QtGui/QInputDialog>

REGISTER_PLUGIN_BASIC(OpticksTutorial, Tutorial4);

namespace
{
   template<typename T>
   void updateStatistics(T* pData, double& min, double& max, double& total)
   {
      min = std::min(min, static_cast<double>(*pData));
      max = std::max(max, static_cast<double>(*pData));
      total += *pData;
   }
};

Tutorial4::Tutorial4()
{
   setDescriptorId("{F58F8F9A-6D2D-4EB1-9FD8-FD6D2E6ECB49}");
   setName("Tutorial 4");
   setDescription("Using AOIs.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/Tutorial 4");
   setAbortSupported(true);
}

Tutorial4::~Tutorial4()
{
}

bool Tutorial4::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Generate statistics for this raster element");
   if (isBatch())
   {
      pInArgList->addArg<AoiElement>("AOI", NULL, "The AOI to calculate statistics over");
   }
   return true;
}

bool Tutorial4::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("Minimum", "The minimum value");
   pOutArgList->addArg<double>("Maximum", "The maximum value");
   pOutArgList->addArg<unsigned int>("Count", "The number of pixels");
   pOutArgList->addArg<double>("Mean", "The average value");
   return true;
}

bool Tutorial4::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Tutorial 4", "app", "95034AC8-EC4C-4CB6-9089-4EF0DCBB41C3");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (pCube == NULL)
   {
      std::string msg = "A raster cube must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);

   AoiElement* pAoi = NULL;
   if (isBatch())
   {
      pAoi = pInArgList->getPlugInArgValue<AoiElement>("AOI");
   }
   else
   {
      Service<ModelServices> pModel;
      std::vector<DataElement*> pAois = pModel->getElements(pCube, TypeConverter::toString<AoiElement>());
      if (!pAois.empty())
      {
         QStringList aoiNames("<none>");
         for (std::vector<DataElement*>::iterator it = pAois.begin(); it != pAois.end(); ++it)
         {
            aoiNames << QString::fromStdString((*it)->getName());
         }
         QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Select an AOI", "Select an AOI for processing", aoiNames);
         // select AOI
         if (aoi != "<none>")
         {
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
         }
      }
   } // end if

   unsigned int startRow = 0;
   unsigned int startColumn = 0;
   unsigned int endRow = pDesc->getRowCount() - 1;
   unsigned int endColumn = pDesc->getColumnCount() - 1;
   const BitMask* pPoints = (pAoi == NULL) ? NULL : pAoi->getSelectedPoints();
   if (pPoints != NULL && !pPoints->isOutsideSelected())
   {
      int x1;
      int x2;
      int y1;
      int y2;
      pPoints->getMinimalBoundingBox(x1, y1, x2, y2);

      startRow = y1;
      endRow = y2;
      startColumn = x1;
      endColumn = x2;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setRows(pDesc->getActiveRow(startRow), pDesc->getActiveRow(endRow));
   pRequest->setColumns(pDesc->getActiveColumn(startColumn), pDesc->getActiveColumn(endColumn));
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   double min = std::numeric_limits<double>::max();
   double max = -min;
   double total = 0.0;
   unsigned int count = 0;
   for (unsigned int row = startRow; row <= endRow; ++row)
   {
      if (isAborted())
      {
         std::string msg = getName() + " has been aborted.";
         pStep->finalize(Message::Abort, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ABORT);
         }

         return false;
      }
      if (!pAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }

         return false;
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", row * 100 / pDesc->getRowCount(), NORMAL);
      }

      for (unsigned int col = startColumn; col <= endColumn; ++col)
      {
         if (pPoints == NULL || pPoints->getPixel(col, row))
         {
            switchOnEncoding(pDesc->getDataType(), updateStatistics, pAcc->getColumn(), min, max, total);
            ++count;
            pAcc->nextColumn();
         }
      }
      pAcc->nextRow();
   }
   double mean = total / count;

   if (pProgress != NULL)
   {
      std::string msg = "Minimum value: " + StringUtilities::toDisplayString(min) + "\n"
                      + "Maximum value: " + StringUtilities::toDisplayString(max) + "\n"
                      + "Number of pixels: " + StringUtilities::toDisplayString(count) + "\n"
                      + "Average: " + StringUtilities::toDisplayString(mean);
      pProgress->updateProgress(msg, 100, NORMAL);
   }
   pStep->addProperty("Minimum", min);
   pStep->addProperty("Maximum", max);
   pStep->addProperty("Count", count);
   pStep->addProperty("Mean", mean);
   
   pOutArgList->setPlugInArgValue("Minimum", &min);
   pOutArgList->setPlugInArgValue("Maximum", &max);
   pOutArgList->setPlugInArgValue("Count", &count);
   pOutArgList->setPlugInArgValue("Mean", &mean);

   pStep->finalize();
   return true;
}
