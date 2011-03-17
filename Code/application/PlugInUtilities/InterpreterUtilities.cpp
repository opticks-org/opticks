/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Interpreter.h"
#include "InterpreterManager.h"
#include "InterpreterUtilities.h"
#include "PlugIn.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"

#include <boost/any.hpp>

bool InterpreterUtilities::executeScopedCommand(const std::string& interpreterName, const std::string& command,
                                                const Slot& output, const Slot& error, Progress* pProgress)
{
   Interpreter* pInterpreter = getInterpreter(interpreterName);
   if (pInterpreter == NULL)
   {
      return false;
   }
   return pInterpreter->executeScopedCommand(command, output, error, pProgress);
}

class ScriptListener
{
public:
   ScriptListener() : mHasErrorText(false) {}
   virtual ~ScriptListener() {}

   void receiveStandardOutput(Subject& subject, const std::string& signal, const boost::any& data)
   {
      std::string text = boost::any_cast<std::string>(data);
      mOutputText += text;
   }
   void receiveErrorOutput(Subject& subject, const std::string& signal, const boost::any& data)
   {
      std::string text = boost::any_cast<std::string>(data);
      mOutputText += text;
      mHasErrorText = true;
   }

   std::string mOutputText;
   bool mHasErrorText;
};

bool InterpreterUtilities::executeScopedCommand(const std::string& interpreterName, const std::string& command,
                                                std::string& outputAndError, bool& hasErrorText, Progress* pProgress)
{
   Interpreter* pInterpreter = getInterpreter(interpreterName);
   if (pInterpreter == NULL)
   {
      return false;
   }
   ScriptListener listener;
   bool retVal = pInterpreter->executeScopedCommand(command, Slot(&listener, &ScriptListener::receiveStandardOutput),
      Slot(&listener, &ScriptListener::receiveErrorOutput), pProgress);
   outputAndError = listener.mOutputText;
   hasErrorText = listener.mHasErrorText;
   return retVal;
}

bool InterpreterUtilities::isInterpreterAvailable(const std::string& interpreterName)
{
   return getInterpreter(interpreterName) != NULL;
}

std::vector<std::string> InterpreterUtilities::getInterpreters()
{
   std::vector<std::string> retVal;
   Service<PlugInManagerServices> pPlugInMgr;
   std::vector<PlugInDescriptor*> descriptors = pPlugInMgr->getPlugInDescriptors(
      PlugInManagerServices::InterpreterManagerType());
   for (std::vector<PlugInDescriptor*>::iterator iter = descriptors.begin();
      iter != descriptors.end();
      ++iter)
   {
      if (*iter != NULL)
      {
         retVal.push_back((*iter)->getName());
      }
   }
   return retVal;
}

Interpreter* InterpreterUtilities::getInterpreter(const std::string& interpreterName)
{
   Service<PlugInManagerServices> pPlugInMgr;
   std::vector<PlugIn*> plugins = pPlugInMgr->getPlugInInstances(interpreterName);
   if (plugins.empty())
   {
      return NULL;
   }
   InterpreterManager* pMgr = dynamic_cast<InterpreterManager*>(plugins.front());
   if (pMgr == NULL || !pMgr->isStarted())
   {
      return NULL;
   }
   return pMgr->getInterpreter();
}
