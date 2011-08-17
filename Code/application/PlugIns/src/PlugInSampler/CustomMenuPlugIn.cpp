/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "CustomMenuPlugIn.h"
#include "DesktopServices.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterElement.h"

CustomMenuPlugIn::CustomMenuPlugIn(const std::string& name, const std::string& fullPath)
   : mMenuCommand(name), mFullPath(fullPath)
{
   setName(mMenuCommand + "CustomMenuPlugIn");
   setProductionStatus(false);
   setDescriptorId(mMenuCommand + "4DE27637-11D8-4017-AB85-BF190D995F45");
   allowMultipleInstances(false);
   setType("Sample");
   setWizardSupported(false);
   setMenuLocation("[Demo]/Dynamic PlugIns/" + mMenuCommand);
}

bool CustomMenuPlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(), NULL));
   return true;
}

bool CustomMenuPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool CustomMenuPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);
   RasterElement* pElement = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   std::string message = "This is a dynamic plug-in executed from the menu named \"" + mMenuCommand + "\".";
   if (pElement != NULL)
   {
      message += " The name of the currently selected raster element is " + pElement->getName() + ".";
   }
   else
   {
      message += " There is no currently selected raster element.";
   }
   message += " This plug-in was loaded using the file located at " + mFullPath + ".";
   Service<DesktopServices>()->showMessageBox("Dynamic Plug-In (" + mMenuCommand + ")",
      message, "OK");
   return true;
}
