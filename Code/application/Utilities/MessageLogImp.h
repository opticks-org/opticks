/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGE_LOG_IMP_H
#define MESSAGE_LOG_IMP_H

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include "MessageLog.h"
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
   int val;
   NumChain *next;

public:
   NumChain() : val(0), next(NULL) {}
   NumChain(NumChain &other) : val(other.val)
   {
      if(other.next != NULL)
         next = new NumChain(*other.next);
      else
         next = NULL;
   }
   ~NumChain() {
         if(next != NULL)
            delete next;
      }
   void operator()(int v) { val = v; }
   int append(int v) {
         if(next == NULL)
         {
            next = new NumChain();
            (*next)(v);
            return 1;
         }
         else
         {
            return next->append(v) + 1;
         }
      }

   std::string toString() {
         QString str(toQString());
         return str.toStdString();
      }

   QString toQString() {
         if(next == NULL)
         {
            return QString::number(val);
         }
         else
         {
            return QString::number(val) + "." + next->toString().c_str();
         }
      }
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
                                  bool finalizeOnCreate=false,
                                  bool recurse=true);
   virtual Step *createStep(const std::string &action,
                            const std::string &component,
                            const std::string &key,
                            bool recurse=true);

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
   QFile *mpLogFile;

protected: // Observer
   void messageDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void messageHidden(Subject &subject, const std::string &signal, const boost::any &v);
   void messageModified(Subject &subject, const std::string &signal, const boost::any &v);
   void messageAdded(Subject &subject, const std::string &signal, const boost::any &v);

private:
   std::string mpLogName;
   Filename* mpFilename;
   std::vector<Message*> mMessageList;
   Step *mpCurrentStep;
   QFile *mpJournal;
   QTextStream *mpJournalWriter;
   XMLWriter *mpWriter;
};

class MessageImp : public SubjectImp, public Serializable
{
public:
   /**
    *  Create a new Message
    */
   MessageImp(const std::string &action,
              const std::string &component,
              const std::string &key,
              DateTime *timestamp=NULL,
              const Step *pParent=NULL);
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
   virtual bool addProperty(const std::string &name, const Filename *value);
   virtual bool addProperty(const std::string &name, const DateTime *value);
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
   virtual std::string getStringId() { return getId().toString(); }
   NumChain &getId() { return *id; }
   void setId(NumChain *v) { delete id; id = v; }
   const Step *getParent() const { return mpParent; }

public: // serialize functionality
   virtual bool toXml(XMLWriter* pXml) const;
   virtual void serializeDate(std::string &date, std::string &time) const;

public: // SubjectAdapter functionality
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& name) const;

protected:
   void serializeReal(XMLWriter* pXml, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pNode) const;
   friend class MessageLogImp;

protected:
   DateTime *mpTimeStamp;
   bool mFinalized;

private:
   std::string mAction;
   std::string mComponent;
   std::string mKey;
   NumChain *id;
   DynamicObject *mpProperties;
   const Step *mpParent;
};

class StepImp : public MessageImp
{
public:
   /**
    *  Construct a Step
    */
   StepImp(const std::string &action,
           const std::string &component,
           const std::string &key,
           DateTime *timestamp=NULL,
           const Step *pParent=NULL);
   virtual ~StepImp();

   virtual Step *addStep(const std::string &action,
                         const std::string &component,
                         const std::string &key,
                         bool recurse=true);
   virtual Message *addMessage(const std::string &action,
                               const std::string &component,
                               const std::string &key,
                               bool finalizeOnCreate=false,
                               bool recurse=true);

   virtual bool finalize(Message::Result result = Message::Success);
   virtual bool finalize(Message::Result result = Message::Success, const std::string& failureReason = "");
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

   virtual std::string getComponent() const { return MessageImp::getComponent(); }
   virtual std::string getKey() const { return MessageImp::getKey(); }

protected: // Observer
   void messageDeleted(Subject &subject, const std::string &signal, const boost::any &v);
   void messageHidden(Subject &subject, const std::string &signal, const boost::any &v);
   void messageModified(Subject &subject, const std::string &signal, const boost::any &v);
   void messageAdded(Subject &subject, const std::string &signal, const boost::any &v);

private:
   Message::Result mResult;
   std::string mFailureReason;
   std::vector<Message*> mMessageList;
   Step *mpCurrentStep;
};

#define MESSAGEADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
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
bool toXml(XMLWriter* pXml) const \
{ \
   return impClass::toXml(pXml); \
} \
 \
const std::string& getObjectType() const \
{ \
   static std::string type("MessageAdapter"); \
   return type; \
} \
 \
bool isKindOf(const std::string& name) const \
{ \
   if ((name == getObjectType()) || (name == "Message")) \
   { \
      return true; \
   } \
 \
   return impClass::isKindOf(name); \
} \
   \
bool fromXml(DOMNode* pDocument, unsigned int version) \
{ \
   return false; \
} \
 \
const DynamicObject *getProperties() const \
{ \
   return MessageImp::getProperties(); \
} \

#endif
