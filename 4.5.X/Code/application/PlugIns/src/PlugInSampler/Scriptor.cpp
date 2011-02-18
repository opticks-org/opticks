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

#include <boost/tokenizer.hpp>

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, Scriptor);
REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, StartScriptorMenuItem);

Scriptor::Scriptor() : mpInterpreter(new ScriptorExecutor), mStartCount(0)
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
}

bool Scriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
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

bool Scriptor::isStarted() const
{
   return (mStartCount >= 2);
}

bool Scriptor::start()
{
   ++mStartCount;
   if (mStartCount == 2)
   {
      notify(SIGNAL_NAME(InterpreterManager, InterpreterStarted));
   }
   return (mStartCount >= 2);
}

std::string Scriptor::getStartupMessage() const
{
   if (isStarted())
   {
      return "Scriptor has been started\n";
   }
   else
   {
      return "Failure starting up Scriptor, please try again.\n";
   }
}

Interpreter* Scriptor::getInterpreter() const
{
   if (isStarted())
   {
      return mpInterpreter.get();
   }
   else
   {
      return NULL;
   }
}

const std::string& Scriptor::getObjectType() const
{
   static std::string sType("Scriptor");
   return sType;
}

bool Scriptor::isKindOf(const std::string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

ScriptorExecutor::ScriptorExecutor() : mGlobalOutputShown(false), mCommandNumber(0)
{}

std::string ScriptorExecutor::getPrompt() const
{
   return "Sample Scriptor [" + StringUtilities::toDisplayString(mCommandNumber) + "]> ";
}

bool ScriptorExecutor::executeCommand(const std::string& command)
{
   return executeCommandInternal(command, false);
}

bool ScriptorExecutor::executeScopedCommand(const std::string& command, const Slot& output,
                                            const Slot& error, Progress* pProgress)
{
   attach(SIGNAL_NAME(ScriptorExecutor, ScopedOutputText), output);
   attach(SIGNAL_NAME(ScriptorExecutor, ScopedErrorText), error);
   bool retValue = executeCommandInternal(command, true);
   detach(SIGNAL_NAME(ScriptorExecutor, ScopedErrorText), error);
   detach(SIGNAL_NAME(ScriptorExecutor, ScopedOutputText), output);
   return retValue;
}

bool ScriptorExecutor::executeCommandInternal(const std::string& command, bool scoped)
{
   if (command.empty())
   {
      return true;
   }

   std::vector<std::string> lines;
   typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
   boost::char_separator<char> newlineSep("\n", "", boost::keep_empty_tokens);
   tokenizer tokens(command, newlineSep);
   std::copy(tokens.begin(), tokens.end(), std::back_inserter(lines));

   bool bValidCommand = true;
   for (std::vector<std::string>::iterator iter = lines.begin();
        iter != lines.end();
        ++iter)
   {
      ++mCommandNumber;

      std::string curCommand = *iter;
      if (curCommand.empty())
      {
         continue;
      }

      // Generate the output text
      // For this example, a valid command begins with an upper or lowercase letter
      bValidCommand = (curCommand[0] >= 'a' && curCommand[0] <= 'z') || (curCommand[0] >= 'A' && curCommand[0] <= 'Z');

      if (bValidCommand == true)
      {
         if (curCommand == "mixedOutput")
         {
            sendOutput("Here is some ", scoped);
            sendError("intermixed error output in the middle of ", scoped);
            sendOutput("normal output, while processing a valid command\n", scoped);
         }
         else if (curCommand == "printError")
         {
            sendError("Received valid command of printError that outputs error text\n", scoped);
         }
         else if (curCommand == "connect")
         {
            Service<ModelServices>()->attach(SIGNAL_NAME(ModelServices, ElementCreated), Slot(this, &ScriptorExecutor::elementCreated));
         }
         else if (curCommand == "disconnect")
         {
            Service<ModelServices>()->detach(SIGNAL_NAME(ModelServices, ElementCreated), Slot(this, &ScriptorExecutor::elementCreated));
         }
         else
         {
            sendOutput("Received command: " + curCommand + "\n", scoped);
         }
      }
      else
      {
         sendError("Invalid command: " + curCommand + "\n", scoped);
         break;
      }
   }

   return bValidCommand;
}

void ScriptorExecutor::sendOutput(const std::string& text, bool scoped)
{
   if (text.empty())
   {
      return;
   }
   if (scoped)
   {
      notify(SIGNAL_NAME(ScriptorExecutor, ScopedOutputText), text);
   }
   if (!scoped || mGlobalOutputShown)
   {
      notify(SIGNAL_NAME(Interpreter, OutputText), text);
   }
}

void ScriptorExecutor::sendError(const std::string& text, bool scoped)
{
   if (text.empty())
   {
      return;
   }
   if (scoped)
   {
      notify(SIGNAL_NAME(ScriptorExecutor, ScopedErrorText), text);
   }
   if (!scoped || mGlobalOutputShown)
   {
      notify(SIGNAL_NAME(Interpreter, ErrorText), text);
   }
}

bool ScriptorExecutor::isGlobalOutputShown() const
{
   return mGlobalOutputShown;
}

void ScriptorExecutor::showGlobalOutput(bool newValue)
{
   mGlobalOutputShown = newValue;
}

const std::string& ScriptorExecutor::getObjectType() const
{
   static std::string sType("ScriptorExecutor");
   return sType;
}

bool ScriptorExecutor::isKindOf(const std::string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void ScriptorExecutor::elementCreated(Subject& subject, const std::string& signal, const boost::any& data)
{
   sendOutput("ModelServices ElementCreated signal received\n", false);
}

StartScriptorMenuItem::StartScriptorMenuItem()
{
   setName("Start Scriptor Menu Item");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2010, Ball Aerospace & Technologies Corp.");
   setDescription("Starts scriptor");
   setProductionStatus(false);
   setDescriptorId("{BC1D5265-1952-4323-960B-8D5B4257C07D}");
   setWizardSupported(false);
   setAbortSupported(false);
   setMenuLocation("[Demo]/Start Scriptor");
   setType("Sample");
}

bool StartScriptorMenuItem::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool StartScriptorMenuItem::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool StartScriptorMenuItem::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   std::vector<PlugIn*> instances = Service<PlugInManagerServices>()->getPlugInInstances("Sample Scriptor");
   if (instances.empty())
   {
      return false;
   }
   Scriptor* pScriptor = dynamic_cast<Scriptor*>(instances.front());
   if (pScriptor == NULL)
   {
      return false;
   }
   return pScriptor->start();
}
