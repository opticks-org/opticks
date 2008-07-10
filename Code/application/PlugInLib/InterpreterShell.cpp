/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "InterpreterShell.h"
#include "PlugInManagerServices.h"

using namespace std;

InterpreterShell::InterpreterShell()
{
   setType(PlugInManagerServices::InterpreterType());
   destroyAfterExecute(false);
}

InterpreterShell::~InterpreterShell()
{
}

string InterpreterShell::getFileExtensions() const
{
   return mExtensions;
}

void InterpreterShell::setFileExtensions(const string& extensions)
{
   mExtensions = extensions.c_str();
}
