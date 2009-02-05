/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGELOGIMP_H
#define MESSAGELOGIMP_H

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include "MessageLog.h"
#include "SerializableImp.h"
#include "SubjectImp.h"
#include "SubjectAdapter.h"
#include "TypesFile.h"
#include "xmlwriter.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "XercesIncludes.h"

class MessageImp;
class StepImp;

class NumChain
{
public:
   NumChain() :
      mVal(0),
      mpNext(NULL)
   {
   }

   NumChain(NumChain& other) :
      mVal(other.mVal),
      mpNext(NULL)
   {
      if (other.mpNext != NULL)
      {
         mpNext = new NumChain(*other.mpNext);
      }
   }

   ~NumChain()
   {
      if (mpNext != NULL)
      {
         delete mpNext;
      }
   }

   void operator()(int v)
   {
      mVal = v;
   }

   int append(int v)
   {
      if (mpNext == NULL)
      {
         mpNext = new NumChain();
         (*mpNext)(v);
         return 1;
      }

      return mpNext->append(v) + 1;
   }

   std::string toString()
   {
      QString str(toQString());
      return str.toStdString();
   }

   QString toQString()
   {
      if (mpNext == NULL)
      {
         return QString::number(mVal);
      }

      return QString::number(mVal) + "." + mpNext->toString().c_str();
   }

private:
   int mVal;
   NumChain* mpNext;
};

class MessageLogImp : public SubjectImp
{
public:
   /**
    *  Construct a new message log
    */
   MessageLogImp(const char* name, const char* path, QFile *journal);
   virtual ~MessageLogImp();

   virtual Message *createMessage(const std::string &action,
                                  const std::string &component,
                                  const std::string &key,
                                  bool finalizeOnCreate = false,
                                  bool recurse = true);
   virtual Step *createStep(const std::string &action,
                            const std::string &component,
                            const std::string &key,
                            bool recurse = true);

   virtual MessageLog::size_t size() const;
   virtual MessageLog::iterator begin();
   virtual MessageLog::const_iterator begin() const;
   virtual MessageLog::iterator end();
   virtual MessageLog::const_iterator end() const;
   virtual Message *operator[](MessageLog::size_t i);
   virtual const Message *operator[](MessageLog::size_t i) const;

public: // Subject functionality
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& name) const;
   void messageDetached(Subject &subject, const std::string &signal, const boost::any &v);
      
public: // serialize functionality
   virtual std::string serialize();

public:
   const std::string& getLogName();
   QFile* mpLogFile;

protected: // Observer
   void messageDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void messageHidden(Subject &subject, const std::string &signal, const boost::any &v);
   void messageModified(Subject &subject, const std::string &signal, const boost::any &v);
   void messageAdded(Subject &subject, const std::string &signal, const boost::any &v);

private:
   std::string mpLogName;
   Filename* mpFilename;
   std::vector<Message*> mMessageList;
   Step* mpCurrentStep;
   QFile* mpJournal;
   QTextStream* mpJournalWriter;
   XMLWriter* mpWriter;
};

#define MESSAGELOGADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define MESSAGELOGADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
Message* createMessage(const std::string &action, \
                       const std::string &component, \
                       const std::string &key, \
                       bool finalizeOnCreate = false, \
                       bool recurse = true) \
{ \
   return impClass::createMessage(action, component, key, finalizeOnCreate, recurse); \
} \
Step* createStep(const std::string &action, \
       const std::string &component, \
       const std::string &key, \
       bool recurse = true) \
{ \
   return impClass::createStep(action, component, key, recurse); \
} \
MessageLog::size_t size() const  \
{  \
   return impClass::size();  \
} \
MessageLog::iterator begin() \
{ \
   return impClass::begin(); \
} \
MessageLog::const_iterator begin() const \
{ \
   return impClass::begin(); \
} \
MessageLog::iterator end() \
{ \
   return impClass::end(); \
} \
MessageLog::const_iterator end() const \
{ \
   return impClass::end(); \
} \
Message *operator[](MessageLog::size_t i)  \
{  \
   return impClass::operator[](i);  \
} \
const Message *operator[](MessageLog::size_t i) const  \
{  \
   return impClass::operator[](i);  \
} \
const std::string& getLogName() \
{ \
   return impClass::getLogName(); \
}

class MessageImp : public SubjectImp, public Serializable
{
public:
   /**
    *  Create a new Message
    */
   MessageImp(const std::string& action, const std::string& component, const std::string& key,
      DateTime* timestamp = NULL, const Step* pParent = NULL);
   virtual ~MessageImp();

   virtual bool addProperty(const std::string &name, const char *value);
   virtual bool addProperty(const std::string &name, char value);
   virtual bool addProperty(const std::string &name, unsigned char value);
   virtual bool addProperty(const std::string &name, short value);
   virtual bool addProperty(const std::string &name, unsigned short value);
   virtual bool addProperty(const std::string &name, int value);
   virtual bool addProperty(const std::string &name, unsigned int value);
   virtual bool addProperty(const std::string &name, long value);
   virtual bool addProperty(const std::string &name, unsigned long value);
   virtual bool addProperty(const std::string &name, float value);
   virtual bool addProperty(const std::string &name, double value);
   virtual bool addProperty(const std::string &name, const std::string &value);
   virtual bool addProperty(const std::string &name, const Filename* pValue);
   virtual bool addProperty(const std::string &name, const DateTime* pValue);
   virtual bool addProperty(const std::string &name, const std::vector<char>& value);
   virtual bool addProperty(const std::string &name, const std::vector<unsigned char>& value);
   virtual bool addProperty(const std::string &name, const std::vector<short>& value);
   virtual bool addProperty(const std::string &name, const std::vector<unsigned short>& value);
   virtual bool addProperty(const std::string &name, const std::vector<int>& value);
   virtual bool addProperty(const std::string &name, const std::vector<unsigned int>& value);
   virtual bool addProperty(const std::string &name, const std::vector<long>& value);
   virtual bool addProperty(const std::string &name, const std::vector<unsigned long>& value);
   virtual bool addProperty(const std::string &name, const std::vector<float>& value);
   virtual bool addProperty(const std::string &name, const std::vector<double>& value);
   virtual bool addProperty(const std::string &name, const std::vector<bool>& value);
   virtual bool addProperty(const std::string &name, const std::vector<std::string>& value);
   virtual bool addProperty(const std::string &name, const std::vector<const Filename*>& value);
   virtual bool addBooleanProperty(const std::string &name, bool value);

   virtual bool finalize(Message::Result result = Message::Success);

   virtual std::string getComponent() const;
   virtual std::string getKey() const;

   virtual const DynamicObject *getProperties() const = 0;

   virtual std::string getAction() const;
   virtual Message::Result getResult() const;
   virtual std::string propertyToString(const std::string &type, void *pValue) const;
   virtual std::string getStringId();
   NumChain& getId();
   void setId(NumChain* v);
   const Step* getParent() const;

public: // serialize functionality
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version) const;
   virtual void serializeDate(std::string &date, std::string &time) const;

public: // SubjectAdapter functionality
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& name) const;

protected:
   void serializeReal(XMLWriter* pXml, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pNode) const;
   friend class MessageLogImp;

protected:
   DateTime* mpTimeStamp;
   bool mFinalized;

private:
   std::string mAction;
   std::string mComponent;
   std::string mKey;
   NumChain* mpId;
   DynamicObject* mpProperties;
   const Step* mpParent;
};

class StepImp : public MessageImp
{
public:
   /**
    *  Construct a Step
    */
   StepImp(const std::string& action, const std::string& component, const std::string& key,
      DateTime* timestamp = NULL, const Step* pParent = NULL);
   virtual ~StepImp();

   virtual Step *addStep(const std::string &action,
                         const std::string &component,
                         const std::string &key,
                         bool recurse = true);
   virtual Message *addMessage(const std::string &action,
                               const std::string &component,
                               const std::string &key,
                               bool finalizeOnCreate = false,
                               bool recurse = true);

   virtual bool finalize(Message::Result result = Message::Success);
   virtual bool finalize(Message::Result result, const std::string& failureReason);
   virtual const std::string& getFailureMessage() const;
   virtual Message::Result getResult() const;

   virtual Step::size_t size() const;
   virtual Step::iterator begin();
   virtual Step::const_iterator begin() const;
   virtual Step::iterator end();
   virtual Step::const_iterator end() const;
   virtual Message *operator[](Step::size_t i);
   virtual const Message *operator[](Step::size_t i) const;

public: // SubjectAdapter functionality
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& name) const;
   void messageDetached(Subject &subject, const std::string &signal, const boost::any &v);

public: // serialize functionality
   virtual bool toXml(XMLWriter* pXml) const;

   virtual std::string getComponent() const;
   virtual std::string getKey() const;

protected: // Observer
   void messageDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void messageHidden(Subject &subject, const std::string &signal, const boost::any &v);
   void messageModified(Subject &subject, const std::string &signal, const boost::any &v);
   void messageAdded(Subject &subject, const std::string &signal, const boost::any &v);

private:
   Message::Result mResult;
   std::string mFailureReason;
   std::vector<Message*> mMessageList;
   Step* mpCurrentStep;
};

#define MESSAGEADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   SERIALIZABLEADAPTEREXTENSION_CLASSES

#define MESSAGEADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   SERIALIZABLEADAPTER_METHODS(impClass) \
bool addProperty(const std::string &name, const char *value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, char value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
 bool addProperty(const std::string &name, unsigned char value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, short value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, unsigned short value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, int value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, unsigned int value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, long value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, unsigned long value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, float value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, double value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::string &value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const Filename *value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const DateTime *value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<char>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<unsigned char>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<short>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<unsigned short>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<int>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<unsigned int>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<long>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<unsigned long>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<float>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<double>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<bool>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<std::string>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addProperty(const std::string &name, const std::vector<const Filename*>& value) \
{ \
   return impClass::addProperty(name, value); \
} \
 \
bool addBooleanProperty(const std::string &name, bool value) \
{ \
   return impClass::addBooleanProperty(name, value); \
} \
 \
bool finalize(Message::Result result = Message::Success) \
{ \
   return impClass::finalize(result); \
} \
 \
std::string getComponent() const \
{ \
   return impClass::getComponent(); \
} \
 \
std::string getKey() const \
{ \
   return impClass::getKey(); \
} \
 \
std::string getAction() const \
{ \
   return impClass::getAction(); \
} \
 \
Message::Result getResult() const \
{ \
   return impClass::getResult(); \
} \
 \
std::string propertyToString(const std::string &type, void *pValue) const \
{ \
   return impClass::propertyToString(type, pValue); \
} \
 \
std::string getStringId() \
{ \
   return impClass::getStringId(); \
} \
 \
NumChain &getId() \
{ \
   return impClass::getId(); \
} \
 \
void setId(NumChain *v) \
{ \
   impClass::setId(v); \
} \
 \
void serializeDate(std::string &date, std::string &time) const \
{ \
   impClass::serializeDate(date, time); \
} \
 \
const DynamicObject *getProperties() const \
{ \
   return impClass::getProperties(); \
}

#define STEPADAPTEREXTENSION_CLASSES \
   MESSAGEADAPTEREXTENSION_CLASSES

#define STEPADAPTER_METHODS(impClass) \
   MESSAGEADAPTER_METHODS(impClass) \
bool finalize(Message::Result result, const std::string& failureReason) \
{ \
   return impClass::finalize(result, failureReason); \
} \
const std::string& getFailureMessage() const \
{ \
   return impClass::getFailureMessage(); \
} \
Step *StepAdapter::addStep(const std::string &action, \
                           const std::string &component, \
                           const std::string &key, \
                           bool recurse = true) \
{ \
   return impClass::addStep(action, component, key, recurse); \
} \
Message *StepAdapter::addMessage(const std::string &action, \
                                 const std::string &component, \
                                 const std::string &key, \
                                 bool finalizeOnCreate = false, \
                                 bool recurse = true) \
{ \
   return impClass::addMessage(action, component, key, finalizeOnCreate, recurse); \
} \
Step::size_t size() const  \
{  \
   return impClass::size();  \
} \
Step::iterator begin() \
{ \
   return impClass::begin(); \
} \
Step::const_iterator begin() const \
{ \
   return impClass::begin(); \
} \
Step::iterator end() \
{ \
   return impClass::end(); \
} \
Step::const_iterator end() const \
{ \
   return impClass::end(); \
} \
Message *operator[](Step::size_t i)  \
{  \
   return impClass::operator[](i);  \
} \
const Message *operator[](Step::size_t i) const  \
{  \
   return impClass::operator[](i);  \
} \
const Step *getParent() const  \
{  \
   return impClass::getParent();  \
}

#endif
