/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudPagerShell.h"
#include "PlugInManagerServices.h"

using namespace std;

PointCloudPagerShell::PointCloudPagerShell()
{
   setType(PlugInManagerServices::PointCloudPagerType());
   allowMultipleInstances(true);
   destroyAfterExecute(false);
   setWizardSupported(false);
   setValidSessionSaveItem(false);
}

PointCloudPagerShell::~PointCloudPagerShell()
{
}

bool PointCloudPagerShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PointCloudPagerShell::serialize(SessionItemSerializer& serializer) const
{
   return true;
}