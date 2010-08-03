/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGELOGRESOURCE_H
#define MESSAGELOGRESOURCE_H

#include "MessageLogMgr.h"
#include "Resource.h"
#include "SessionManager.h"

#include <string>

class Step;

class MessageObject
{
public:
   struct Args
   {
      Args(std::string stepName, std::string component, std::string key, std::string defaultFailureMsg,
           std::string messageLogName, bool createStep) :
         mStepName(stepName),
         mFailureMsg(defaultFailureMsg),
         mComponent(component),
         mKey(key),
         mCreateStep(createStep)
      {
         if (messageLogName.empty())
         {
            Service<SessionManager> pManager;
            mMessageLogName = pManager->getName();
         }
         else
         {
            mMessageLogName = messageLogName;
         }
      }

      std::string mMessageLogName;
      std::string mStepName;
      std::string mFailureMsg;
      std::string mComponent;
      std::string mKey;
      bool mCreateStep;
   };

   Message* obtainResource(const Args& args) const
   {
      Service<MessageLogMgr> pMgr;

      if (args.mMessageLogName.empty())
      {
         return NULL;
      }

      MessageLog* pLog = pMgr->getLog(args.mMessageLogName);
      if (pLog == NULL)
      {
         return NULL;
      }

      Message* pStep = NULL;
      if (args.mCreateStep)
      {
         pStep = pLog->createStep(args.mStepName, args.mComponent, args.mKey);
      }
      else
      {
         pStep = pLog->createMessage(args.mStepName, args.mComponent, args.mKey);
      }

      return pStep;
   }

   void releaseResource(const Args& args, Message* pStep) const
   {
      if ((pStep != NULL) && (args.mCreateStep == true))
      {
         //---- go ahead an finalize the step
         //---- as having failed,  this will
         //---- be a no-op if the step has already
         //---- been finalized.
         static_cast<Step*>(pStep)->finalize(Message::Failure, args.mFailureMsg);
      }
   }
};

class StepResource : public Resource<Step,MessageObject>
{
public:
   StepResource(std::string stepName, std::string component, std::string key, std::string defaultFailureMsg = "",
                std::string messageLogName = "") :
      Resource<Step, MessageObject>(MessageObject::Args(stepName, component, key, defaultFailureMsg,
         messageLogName, true))
   {
   }

   StepResource(Step* curStep, std::string defaultFailureMsg = "") :
      Resource<Step, MessageObject>(curStep, MessageObject::Args("", "", "", defaultFailureMsg, "", true))
   {
   }
};

class MessageResource : public Resource<Message, MessageObject>
{
public:
   MessageResource(std::string stepName, std::string component, std::string key, std::string messageLogName = "") :
      Resource<Message, MessageObject>(MessageObject::Args(stepName, component, key, "", messageLogName, false))
   {
   }
};

#endif
