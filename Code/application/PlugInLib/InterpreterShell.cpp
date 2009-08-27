/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "InterpreterShell.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"

InterpreterShell::InterpreterShell()
{
   setType(PlugInManagerServices::InterpreterType());
   destroyAfterExecute(false);
}

InterpreterShell::~InterpreterShell()
{
}

bool InterpreterShell::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<std::string>(Interpreter::CommandArg()));
   return true;
}

bool InterpreterShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   VERIFY(pArgList->addArg<std::string>(Interpreter::ReturnTypeArg(), "Output"));
   VERIFY(pArgList->addArg<std::string>(Interpreter::OutputTextArg()));
   return true;
}

std::string InterpreterShell::getPrompt() const
{
   return getName() + "> ";
}

void InterpreterShell::getKeywordList(std::vector<std::string>& list) const
{
   list.clear();
}

bool InterpreterShell::getKeywordDescription(const std::string& keyword, std::string& description) const
{
   return false;
}

void InterpreterShell::getUserDefinedTypes(std::vector<std::string>& list) const
{
   list.clear();
}

bool InterpreterShell::getTypeDescription(const std::string& type, std::string& description) const
{
   return false;
}

std::string InterpreterShell::getFileExtensions() const
{
   return mExtensions;
}

void InterpreterShell::setFileExtensions(const std::string& extensions)
{
   mExtensions = extensions;
}
