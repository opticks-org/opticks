/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterPagerShell.h"
#include "PlugInManagerServices.h"

using namespace std;

RasterPagerShell::RasterPagerShell()
{
   setType(PlugInManagerServices::RasterPagerType());
   allowMultipleInstances(true);
   destroyAfterExecute(false);
   setWizardSupported(false);
}

RasterPagerShell::~RasterPagerShell()
{
}

bool RasterPagerShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool RasterPagerShell::serialize(SessionItemSerializer& serializer) const
{
   return true;
}
