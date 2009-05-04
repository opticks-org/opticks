/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DimensionDescriptor.h"
#include "DummyCustomAlgorithm.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "ThresholdLayer.h"

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, DummyCustomAlgorithm);

DummyCustomAlgorithm::DummyCustomAlgorithm()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName("Dummy Custom Algorithm");
   setDescription("DummyCustomAlgorithm");
   setShortDescription("DummyCustomAlgorithm");
   setDescriptorId("{7557F76C-E66A-4afe-A471-1021B8C14102}");
}

DummyCustomAlgorithm::~DummyCustomAlgorithm()
{}

bool DummyCustomAlgorithm::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   if (pArg != NULL)
   {
      mpModel->addElementType("DummyType");
      pArg->setName("Custom Test Element");
      pArg->setType("DummyType");
      pArgList->addArg(*pArg);
   }
   return true;
}

bool DummyCustomAlgorithm::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool DummyCustomAlgorithm::execute(PlugInArgList* inputArgList, PlugInArgList* outputArgList)
{
   return true;
}
