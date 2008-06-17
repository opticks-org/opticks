/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "Scriptor.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "SessionItemSerializer.h"

using namespace std;

Scriptor::Scriptor()
{
   // Initialization
   setName("Sample Scriptor");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Provides command line utilities to execute IDL commands.");
   setFileExtensions("Sample Scripting Files (*.scr)");
   setProductionStatus(false);
   setDescriptorId("{5E784B90-4E74-484e-8E2A-6F61F2E835DC}");
   allowMultipleInstances(false);
   destroyAfterExecute(false);
}

Scriptor::~Scriptor()
{
}

bool Scriptor::setBatch()
{
   return true;
}

bool Scriptor::setInteractive()
{
   return true;
}

bool Scriptor::hasAbort()
{
   return false;
}

bool Scriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   // Set up list
   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = NULL;
   pArg = mpPlugInManager->getPlugInArg();      // Command
   if (pArg != NULL)
   {
      pArg->setName(CommandArg());
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool Scriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   // Set up list
   pArgList = mpPlugInManager->getPlugInArgList();
   if (pArgList == NULL)
   {
      return false;
   }

   // Output text
   PlugInArg* pArg = NULL;
   pArg = mpPlugInManager->getPlugInArg();
   if (pArg != NULL)
   {
      pArg->setName(OutputTextArg());
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   // Return type
   pArg = mpPlugInManager->getPlugInArg();
   if (pArg != NULL)
   {
      pArg->setName(ReturnTypeArg());
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool Scriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   bool bSuccess = false;
   bSuccess = extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   if (mCommand.empty() == true)
   {
      return false;
   }

   // TODO: Execute the command here
//   bSuccess = runCommand(mCommand);

   // Generate the output text
   bool bValidCommand = false;

   vector<string> commands;
   getKeywordList(commands);

   vector<string>::iterator iter = commands.begin();
   while (iter != commands.end())
   {
      string currentCommand = *iter;
      string temp = mCommand.substr(0, currentCommand.length());
      if (temp == currentCommand)
      {
         bValidCommand = true;
         break;
      }

      ++iter;
   }

   string returnText = "";
   string returnType = "";

   if (bValidCommand == true)
   {
      returnText = "Received command: " + mCommand;
      returnType = "Output";
   }
   else
   {
      returnText = "Invalid command: " + mCommand;
      returnType = "Error";
   }

   // Populate the output arg list
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      bool bSuccess = false;
      bSuccess = pOutArgList->getArg(OutputTextArg(), pArg);
      if ((bSuccess == true) && (pArg != NULL))
      {
         pArg->setActualValue(&returnText);
      }

      bSuccess = pOutArgList->getArg(ReturnTypeArg(), pArg);
      if ((bSuccess == true) && (pArg != NULL))
      {
         pArg->setActualValue(&returnType);
      }
   }

   return bSuccess;
}

void Scriptor::getKeywordList(vector<string>& list) const
{
   list.push_back("create");
   list.push_back("load");
   list.push_back("add");
   list.push_back("modify");
   list.push_back("remove");
   list.push_back("close");
   list.push_back("save");
   list.push_back("delete");
}

bool Scriptor::getKeywordDescription(const string& keyword, string& description) const
{
   if (keyword == "create")
   {
      description = "sample create keyword";
   }
   else if (keyword == "load")
   {
      description = "sample load keyword";
   }
   else if (keyword == "add")
   {
      description = "sample add keyword";
   }
   else if (keyword == "modify")
   {
      description = "sample modify keyword";
   }
   else if (keyword == "remove")
   {
      description = "sample remove keyword";
   }
   else if (keyword == "close")
   {
      description = "sample close keyword";
   }
   else if (keyword == "save")
   {
      description = "sample save keyword";
   }
   else if (keyword == "delete")
   {
      description = "sample delete keyword";
   }
   else
   {
      return false;
   }

   return true;
}

void Scriptor::getUserDefinedTypes(vector<string>& list) const
{
}

bool Scriptor::getTypeDescription(const string& type, string& description) const
{
   return false;
}

bool Scriptor::extractInputArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Command
   mCommand.erase();

   bool bSuccess = false;
   bSuccess = pArgList->getArg(CommandArg(), pArg);
   if ((bSuccess == true) && (pArg != NULL))
   {
      if (pArg->isActualSet() == true)
      {
         string* pCommand = NULL;
         pCommand = (string*) (pArg->getActualValue());
         if (pCommand != NULL)
         {
            mCommand = *pCommand;
         }
      }
   }

   return true;
}

bool Scriptor::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session restore
}

bool Scriptor::deserialize(SessionItemDeserializer &deserializer)
{
   return true;
}
