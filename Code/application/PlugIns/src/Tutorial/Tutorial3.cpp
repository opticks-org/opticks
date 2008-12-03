/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "Tutorial3.h"
#include <limits>

namespace
{
   template<typename T>
   void updateStatistics(T *pData, double &min, double &max, double &total)
   {
      min = std::min(min, static_cast<double>(*pData));
      max = std::max(max, static_cast<double>(*pData));
      total += *pData;
   }
};

Tutorial3::Tutorial3()
{
   setDescriptorId("{2073076C-2676-45B9-AA7B-A2607104655C}");
   setName("Tutorial 3");
   setDescription("Accessing cube data.");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[Tutorial]/Tutorial 3");
   setAbortSupported(true);
}

Tutorial3::~Tutorial3()
{
}

bool Tutorial3::getInputSpecification(PlugInArgList *&pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Generate statistics for this raster element");
   return true;
}

bool Tutorial3::getOutputSpecification(PlugInArgList *&pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<double>("Minimum", "The minimum value");
   pOutArgList->addArg<double>("Maximum", "The maximum value");
   pOutArgList->addArg<unsigned int>("Count", "The number of pixels");
   pOutArgList->addArg<double>("Mean", "The average value");
   return true;
}

bool Tutorial3::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   StepResource pStep("Tutorial 3", "app", "27170298-10CE-4E6C-AD7A-97E8058C29FF");
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

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());
   double min = std::numeric_limits<double>::max();
   double max = -min;
   double total = 0.0;
   for (unsigned int row = 0; row < pDesc->getRowCount(); row++)
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

      for (unsigned int col = 0; col < pDesc->getColumnCount(); col++)
      {
         switchOnEncoding(pDesc->getDataType(), updateStatistics, pAcc->getColumn(), min, max, total);
         pAcc->nextColumn();
      }
      pAcc->nextRow();
   }
   unsigned int count = pDesc->getColumnCount() * pDesc->getRowCount();
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
