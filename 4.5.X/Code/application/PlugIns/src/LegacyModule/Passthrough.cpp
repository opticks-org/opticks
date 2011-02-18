/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Passthrough.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

#include <string>

using namespace std;

Passthrough::Passthrough()
{
   setName("Passthrough PlugIn");
   setProductionStatus(false);
   setType("Sample");
   setDescriptorId("{2CDA510E-B192-4923-9FAC-986B4F678D7F}");
   allowMultipleInstances(false);
}

bool Passthrough::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<unsigned int>("Input Integer", 10, "This input integer will be passed "
      "to the 'Output Integer' argument as is."));
   return true;
}

bool Passthrough::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<unsigned int>("Output Integer"));
   return true;
}

bool Passthrough::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   unsigned int inputValue = 0;
   VERIFY(pInArgList != NULL);
   VERIFY(pInArgList->getPlugInArgValue("Input Integer", inputValue));
   if (pOutArgList != NULL)
   {
      pOutArgList->setPlugInArgValue<unsigned int>("Output Integer", &inputValue);
   }
   return true;
}
