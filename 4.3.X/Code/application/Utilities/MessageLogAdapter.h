/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MESSAGELOGADAPTER_H
#define MESSAGELOGADAPTER_H

#include "MessageLog.h"
#include "MessageLogImp.h"

class MessageLogAdapter : public MessageLog, public MessageLogImp MESSAGELOGADAPTEREXTENSION_CLASSES
{
public:
   MessageLogAdapter(const char* name, const char* path, QFile *journal);
   virtual ~MessageLogAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   MESSAGELOGADAPTER_METHODS(MessageLogImp)
};

class MessageAdapter : public Message, public MessageImp MESSAGEADAPTEREXTENSION_CLASSES
{
public:
   MessageAdapter(const std::string& action, const std::string& component, const std::string& key,
      DateTime* timestamp = NULL, Step* pParent = NULL);
   virtual ~MessageAdapter();

   const std::string& getObjectType() const
   {
      static std::string sType("MessageAdapter");
      return sType;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "Message"))
      {
         return true;
      }

      return MessageImp::isKindOf(className);
   }

   MESSAGEADAPTER_METHODS(MessageImp)
};

class StepAdapter : public Step, public StepImp STEPADAPTEREXTENSION_CLASSES
{
public:
   StepAdapter(const std::string& action, const std::string& component, const std::string& key,
      DateTime* timestamp = NULL, Step* pParent = NULL);
   virtual ~StepAdapter();

   const std::string& getObjectType() const
   {
      static std::string sType("StepAdapter");
      return sType;
   }

   bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "Step"))
      {
         return true;
      }

      return MessageImp::isKindOf(className);
   }

   STEPADAPTER_METHODS(StepImp)
};

#endif
