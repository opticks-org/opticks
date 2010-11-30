/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "InterpreterManagerShell.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

InterpreterManagerShell::InterpreterManagerShell()
   : mInteractiveEnabled(true)
{
   setType(PlugInManagerServices::InterpreterManagerType());
   executeOnStartup(true);
   destroyAfterExecute(false);
   setWizardSupported(false);
   setAbortSupported(false);
   setSubtype("");
   allowMultipleInstances(false);
}

InterpreterManagerShell::~InterpreterManagerShell()
{}

bool InterpreterManagerShell::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool InterpreterManagerShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool InterpreterManagerShell::isInteractiveEnabled() const
{
   return mInteractiveEnabled;
}

void InterpreterManagerShell::setInteractiveEnabled(bool enabled)
{
   mInteractiveEnabled = enabled;
}

std::string InterpreterManagerShell::getFileExtensions() const
{
   return mExtensions;
}

void InterpreterManagerShell::setFileExtensions(const std::string& extensions)
{
   mExtensions = extensions;
}
