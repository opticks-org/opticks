/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardShell.h"
#include "PlugInManagerServices.h"

WizardShell::WizardShell()
{
   setType(PlugInManagerServices::WizardType());
}

WizardShell::~WizardShell()
{
}

bool WizardShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}
