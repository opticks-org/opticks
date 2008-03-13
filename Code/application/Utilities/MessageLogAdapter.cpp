/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "MessageLogAdapter.h"

using namespace std;

MessageLogAdapter::MessageLogAdapter(const char* name, const char* path, QFile *journal) :
   MessageLogImp(name, path, journal)
{
}

MessageLogAdapter::~MessageLogAdapter()
{
}

const string& MessageLogAdapter::getObjectType() const
{
   static string type("MessageLogAdapter");
   return type;
}

bool MessageLogAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MessageLog"))
   {
      return true;
   }

   return MessageLogImp::isKindOf(className);
}

Message* MessageLogAdapter::createMessage(const string &action,
                                          const string &component,
                                          const string &key,
                                          bool finalizeOnCreate,
                                          bool recurse)
{
   return MessageLogImp::createMessage(action, component, key, finalizeOnCreate, recurse);
}

Step* MessageLogAdapter::createStep(const string &action,
                                    const string &component,
                                    const string &key,
                                    bool recurse)
{
   return MessageLogImp::createStep(action, component, key, recurse);
}

MessageLog::iterator MessageLogAdapter::begin()
{
   return MessageLogImp::begin();
}

MessageLog::const_iterator MessageLogAdapter::begin() const
{
   return MessageLogImp::begin();
}

MessageLog::iterator MessageLogAdapter::end()
{
   return MessageLogImp::end();
}

MessageLog::const_iterator MessageLogAdapter::end() const
{
   return MessageLogImp::end();
}

const string& MessageLogAdapter::getLogName()
{
   return MessageLogImp::getLogName();
}

MessageAdapter::MessageAdapter(const string &action,
                               const string &component,
                               const string &key,
                               DateTime *timestamp,
                               Step *pParent) : MessageImp(action, component, key, timestamp, pParent)
{
}

MessageAdapter::~MessageAdapter()
{
}


StepAdapter::StepAdapter(const string &action,
                         const string &component,
                         const string &key,
                         DateTime *timestamp,
                         Step *pParent) : StepImp(action, component, key, timestamp, pParent)
{
}

StepAdapter::~StepAdapter()
{
}

Step *StepAdapter::addStep(const string &action,
                           const string &component,
                           const string &key,
                           bool recurse)
{
   return StepImp::addStep(action, component, key, recurse);
}

Message *StepAdapter::addMessage(const string &action,
                                 const string &component,
                                 const string &key,
                                 bool finalizeOnCreate,
                                 bool recurse)
{
   return StepImp::addMessage(action, component, key, finalizeOnCreate, recurse);
}

bool StepAdapter::finalize(Message::Result result)
{
   return StepImp::finalize(result,"");
}

bool StepAdapter::finalize(Message::Result result, const string& failureReason)
{
   return StepImp::finalize(result, failureReason);
}

const string& StepAdapter::getFailureMessage() const
{
   return StepImp::getFailureMessage();
}

Message::Result StepAdapter::getResult() const
{
   return StepImp::getResult();
}

Step::iterator StepAdapter::begin()
{
   return StepImp::begin();
}

Step::const_iterator StepAdapter::begin() const
{
   return StepImp::begin();
}

Step::iterator StepAdapter::end()
{
   return StepImp::end();
}

Step::const_iterator StepAdapter::end() const
{
   return StepImp::end();
}

bool StepAdapter::addProperty(const string &name, const char *value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, char value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, unsigned char value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, short value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, unsigned short value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, int value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, unsigned int value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, long value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, unsigned long value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, float value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, double value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const string &value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const Filename *value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const DateTime *value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<char>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<unsigned char>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<short>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<unsigned short>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<int>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<unsigned int>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<long>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<unsigned long>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<float>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<double>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<bool>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<string>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addProperty(const string &name, const vector<const Filename*>& value)
{
   return StepImp::addProperty(name, value);
}

bool StepAdapter::addBooleanProperty(const string &name, bool value)
{
   return StepImp::addBooleanProperty(name, value);
}

string StepAdapter::getComponent() const
{
   return StepImp::getComponent();
}

string StepAdapter::getKey() const
{
   return StepImp::getKey();
}

string StepAdapter::getAction() const
{
   return StepImp::getAction();
}

string StepAdapter::propertyToString(const string &type, void *pValue) const
{
   return StepImp::propertyToString(type, pValue);
}

string StepAdapter::getStringId()
{
   return StepImp::getStringId();
}

NumChain &StepAdapter::getId()
{
   return StepImp::getId();
}

void StepAdapter::setId(NumChain *v)
{
   StepImp::setId(v);
}

void StepAdapter::serializeDate(string &date, string &time) const
{
   StepImp::serializeDate(date, time);
}

bool StepAdapter::toXml(XMLWriter* pXml) const
{
   return StepImp::toXml(pXml);
}

const string& StepAdapter::getObjectType() const
{
   static string type("StepAdapter");
   return type;
}

bool StepAdapter::isKindOf(const string& name) const
{
   if ((name == getObjectType()) || (name == "Step"))
   {
      return true;
   }

   return StepImp::isKindOf(name);
}
