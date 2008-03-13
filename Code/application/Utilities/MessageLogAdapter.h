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

class MessageLogAdapter : public MessageLog, public MessageLogImp
{
public:
   MessageLogAdapter(const char* name, const char* path, QFile *journal);
   virtual ~MessageLogAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // Subject
   bool attach(const std::string &signal, const Slot &slot)
   {
      return MessageLogImp::attach(signal, slot);
   }
   bool detach(const std::string &signal, const Slot &slot)
   {
      return MessageLogImp::detach(signal, slot);
   }

   // MessageLog
   Message* createMessage(const std::string &action,
                          const std::string &component,
                          const std::string &key,
                          bool finalizeOnCreate = false,
                          bool recurse = true);
   Step* createStep(const std::string &action,
		    const std::string &component,
		    const std::string &key,
		    bool recurse = true);
   MessageLog::size_t size() const { return MessageLogImp::size(); }
   MessageLog::iterator begin();
   MessageLog::const_iterator begin() const;
   MessageLog::iterator end();
   MessageLog::const_iterator end() const;
   Message *operator[](MessageLog::size_t i) { return MessageLogImp::operator[](i); }
   const Message *operator[](MessageLog::size_t i) const { return MessageLogImp::operator[](i); }
   const std::string& getLogName();
};

class MessageAdapter : public Message, public MessageImp
{
public:
   MessageAdapter(const std::string &action,
                  const std::string &component,
                  const std::string &key,
                  DateTime *timestamp=NULL,
                  Step *pParent=NULL);
   virtual ~MessageAdapter();

   MESSAGEADAPTER_METHODS(MessageImp)
};

class StepAdapter : public Step, public StepImp
{
public:
   StepAdapter(const std::string &action,
               const std::string &component,
               const std::string &key,
               DateTime *timestamp=NULL,
               Step *pParent=NULL);
   virtual ~StepAdapter();

   Step *StepAdapter::addStep(const std::string &action,
                              const std::string &component,
                              const std::string &key,
                              bool recurse=true);
   Message *StepAdapter::addMessage(const std::string &action,
                                    const std::string &component,
                                    const std::string &key,
                                    bool finalizeOnCreate=false,
                                    bool recurse=true);
   bool finalize(Message::Result result = Message::Success);
   bool finalize(Message::Result result = Message::Success, const std::string& failureReason = "");
   const std::string& getFailureMessage() const;
   Message::Result getResult() const;

   Step::size_t size() const { return StepImp::size(); }
   Step::iterator begin();
   Step::const_iterator begin() const;
   Step::iterator end();
   Step::const_iterator end() const;
   Message *operator[](Step::size_t i) { return StepImp::operator[](i); }
   const Message *operator[](Step::size_t i) const { return StepImp::operator[](i); }

public: // MessageImp
   bool addProperty(const std::string &name, const char *value);
   bool addProperty(const std::string &name, char value);
   bool addProperty(const std::string &name, unsigned char value);
   bool addProperty(const std::string &name, short value);
   bool addProperty(const std::string &name, unsigned short value);
   bool addProperty(const std::string &name, int value);
   bool addProperty(const std::string &name, unsigned int value);
   bool addProperty(const std::string &name, long value);
   bool addProperty(const std::string &name, unsigned long value);
   bool addProperty(const std::string &name, float value);
   bool addProperty(const std::string &name, double value);
   bool addProperty(const std::string &name, const std::string &value);
   bool addProperty(const std::string &name, const Filename *value);
   bool addProperty(const std::string &name, const DateTime *value);
   bool addProperty(const std::string &name, const std::vector<char>& value);
   bool addProperty(const std::string &name, const std::vector<unsigned char>& value);
   bool addProperty(const std::string &name, const std::vector<short>& value);
   bool addProperty(const std::string &name, const std::vector<unsigned short>& value);
   bool addProperty(const std::string &name, const std::vector<int>& value);
   bool addProperty(const std::string &name, const std::vector<unsigned int>& value);
   bool addProperty(const std::string &name, const std::vector<long>& value);
   bool addProperty(const std::string &name, const std::vector<unsigned long>& value);
   bool addProperty(const std::string &name, const std::vector<float>& value);
   bool addProperty(const std::string &name, const std::vector<double>& value);
   bool addProperty(const std::string &name, const std::vector<bool>& value);
   bool addProperty(const std::string &name, const std::vector<std::string>& value);
   bool addProperty(const std::string &name, const std::vector<const Filename*>& value);
   bool addBooleanProperty(const std::string &name, bool value);
   virtual std::string getComponent() const;
   virtual std::string getKey() const;
   const DynamicObject *getProperties() const { return StepImp::getProperties(); }
   std::string getAction() const;
   std::string propertyToString(const std::string &type, void *pValue) const;
   std::string getStringId();
   NumChain &getId();
   void setId(NumChain *v);
   const Step *getParent() const { return StepImp::getParent(); }

   void serializeDate(std::string &date, std::string &time) const;


public: // Serialazable
   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version) { return false; }
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& name) const;

public: // Subject
   bool attach(const std::string &signal, const Slot &slot)
   {
      return StepImp::attach(signal, slot);
   }
   bool detach(const std::string &signal, const Slot &slot)
   {
      return StepImp::detach(signal, slot);
   }
};

#endif
