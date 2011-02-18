/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Scriptor.h"
#include "SessionItemSerializer.h"
#include "StringUtilities.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, Scriptor);

Scriptor::Scriptor() : mCommandNumber(0)
{
   // Initialization
   setName("Sample Scriptor");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrate how to write an interpreter plug-in.");
   setFileExtensions("Sample Scripting Files (*.scr)");
   setProductionStatus(false);
   setDescriptorId("{5E784B90-4E74-484e-8E2A-6F61F2E835DC}");
   allowMultipleInstances(false);
   destroyAfterExecute(false);
   setWizardSupported(false);
   setAbortSupported(false);
}

string Scriptor::getPrompt() const
{
   return getName() + "[" + StringUtilities::toDisplayString(mCommandNumber) + "]> ";
}

bool Scriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   if (!pInArgList->getPlugInArgValue<string>(CommandArg(), mCommand) || mCommand.empty())
   {
      return false;
   }

   // TODO: Execute the command here
   // bSuccess = runCommand(mCommand);
   ++mCommandNumber;

   // Generate the output text
   // For this example, a valid command begins with an upper or lowercase letter
   bool bValidCommand = (mCommand[0] >= 'a' && mCommand[0] <= 'z') || (mCommand[0] >= 'A' && mCommand[0] <= 'Z');

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
      pOutArgList->setPlugInArgValue(OutputTextArg(), &returnText);
      pOutArgList->setPlugInArgValue(ReturnTypeArg(), &returnType);
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
