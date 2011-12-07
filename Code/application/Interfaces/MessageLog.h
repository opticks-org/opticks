/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGE_LOG_H
#define MESSAGE_LOG_H

#include "EnumWrapper.h"
#include "Int64.h"
#include "Serializable.h"
#include "Subject.h"
#include "UInt64.h"

#include <string>
#include <vector>

class DateTime;
class DynamicObject;
class Filename;
class MessageLog;
class Message;
class Step;

/**
 *  \ingroup ServiceModule
 *  This class provides an interface to the message logging system.
 *  A message log is displayed as a tab in the message log window and
 *  saved to a file of the same name. New messages and steps can be
 *  added with this interface. This interface can also be attached to receive
 *  Subject/Observer messages as data is added to the message log.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: createMessage(), createStep().
 *  - A new Message is added to the log; data = any<Message*>.
 *  - A new property is added to a Message: signalMessageModified with data = any<Message*>.
 *  - A Message is finalized with data = any<Message*>.
 *  - A Message is detached or deleted with data = any<Message*>.
 *  - The MessageLog is deleted. This is called after all cleanup has occured.
 *  - Everything else documented in Subject.
 */
class MessageLog : public Subject
{
public: // types   
   /**
    *  An STL iterator type. Has the semantics of an std::vector
    */
   typedef std::vector<Message*>::iterator iterator;

   /**
    *  An STL iterator type. Has the semantics of an std::vector
    */
   typedef std::vector<Message*>::const_iterator const_iterator;

   /**
    *  A type representing sizes and indices
    */
   typedef ::size_t size_t;

public: // methods
   /**
    *  Emitted with any<Message*> when a message is added to the log.
    */
   SIGNAL_METHOD(MessageLog, MessageAdded)
   /**
    *  Emitted with any<Message*> when a message in the log changes.
    */
   SIGNAL_METHOD(MessageLog, MessageModified)
   /**
    *  Emitted with any<Message*> when a message in the log is finalized.
    */
   SIGNAL_METHOD(MessageLog, MessageHidden)
   /**
    *  Emitted with any<Message*> when a message is deleted from the log.
    */
   SIGNAL_METHOD(MessageLog, MessageDeleted)
   /**
    *  Emitted with any<Message*> when a message is detached from the log.
    */
   SIGNAL_METHOD(MessageLog, MessageDetached)

   /**
    *  Creates a new message to be added to a log.
    *
    *  %Properties can be added to the message until it is finalized.
    *  Messages should be used to log information
    *  that is not necessary to repeat a process, but is informative only. This could
    *  include intermediate results from an algorithm or detailed information about
    *  a failure.  A component and key must be provided to uniquely
    *  identify a message.  The purpose of the component and key is to allow
    *  messages to be uniquely identified between versions of the software, so that
    *  any code that may subscribe to messages or parses the message log does
    *  not need to be regularly updated.
    *
    *  @param   action
    *           A string describing the nature of the event that triggered the message.
    *  @param   component
    *           A string specifying the component that is requesting a message
    *           be created, for example the name of the plug-in suite logging the message.
    *  @param   key
    *           A string that specifies a identifier for this message being
    *           created.  The key combined with the component shall uniquely
    *           identify a message.
    *  @param   finalizeOnCreate
    *           If this is \c true, the Message will be automatically finalized.
    *  @param   recurse
    *           If this is \c true, steps are recursed and the Message
    *           is added to the deepest un-finalized step.
    *
    *  @return  A pointer to the message created. This pointer will be valid until it
    *           is finalized.
    *
    *  @notify  This method will notify signalMessageAdded() with
    *           boost::any<\link Message \endlink*>.
    *
    *  @see     Message::finalize()
    */
   virtual Message* createMessage(const std::string& action, const std::string& component, const std::string& key,
      bool finalizeOnCreate = false, bool recurse = true) = 0;

   /**
    *  Creates a new 'step' message to be added to a log.
    *
    *  %Properties can be added to the message until it is finalized. 'Step'  
    *  messages should be used to log information that is necessary to repeat 
    *  a process. Steps can have sub-steps. After creating a 'step' message,
    *  all properties necessary to duplicate that step should be added to the
    *  message. A component and key must be provided to uniquely
    *  identify a step.  The purpose of the component and key is to allow
    *  steps to be uniquely identified between versions of the software, so that
    *  any code that may subscribe to steps or parses the message log does
    *  not need to be regularly updated.
    *
    *  @param   action
    *           A string describing the nature of the event that triggered the message.
    *  @param   component
    *           A string specifying the component that is requesting a step
    *           be created, for example the name of the plug-in suite logging the step.
    *  @param   key
    *           A string that specifies a identifier for this step being
    *           created.  The key combined with the component shall uniquely
    *           identify a step.
    *  @param   recurse
    *           If this is \c true, steps are recursed and the new Step
    *           is added to the deepest un-finalized step.
    *
    *  @return  A pointer to the message created. This pointer will be valid until it
    *           is finalized.
    *
    *  @notify  This method will notify signalMessageAdded() with
    *           boost::any<\link Message \endlink*>.
    *
    *  @see     Step::finalize()
    */
   virtual Step* createStep(const std::string& action, const std::string& component, const std::string& key,
      bool recurse = true) = 0;

   /**
    *  Get the name of this message log
    *
    *  @return The name of this log
    */
   virtual const std::string& getLogName() const = 0;

   /**
    *  Obtain the count of Messages in this %MessageLog.
    *
    *  @return the number of Messages
    */
   virtual MessageLog::size_t size() const = 0;

   /**
    *  Obtain an STL iterator over Messages in this %MessageLog
    *  starting at the begining of the vector.
    *
    *  This iterator has the behavior of a std::vector::iterator
    *
    *  @return  an iterator over the %MessageLog's Messages
    */
   virtual MessageLog::iterator begin() = 0;

   /**
    *  Obtain an STL const_iterator over Messages in this %MessageLog
    *  starting at the begining of the vector.
    *
    *  This iterator has the behavior of a std::vector::const_iterator
    *
    *  @return  a const_iterator over the %MessageLog's Messages
    */
   virtual MessageLog::const_iterator begin() const = 0;

   /**
    *  Obtain an STL iterator over Messages in this %MessageLog
    *  starting at the end of the vector.
    *
    *  This iterator has the behavior of a std::vector::iterator
    *
    *  @return  an iterator over the %MessageLog's Messages
    */
   virtual MessageLog::iterator end() = 0;

   /**
    *  Obtain an STL const_iterator over Messages in this %MessageLog
    *  starting at the end of the vector.
    *
    *  This iterator has the behavior of a std::vector::const_iterator
    *
    *  @return  a const_iterator over the %MessageLog's Messages
    */
   virtual MessageLog::const_iterator end() const = 0;

   /**
    *  Get the %Message at index \a i.
    *
    *  This function has the same semantics as std::vector::operator[]. It
    *  does not ensure that \a i is a valid index.
    *
    *  @param i The index to return.
    *
    *  @return The %Message requested
    */
   virtual Message *operator[](MessageLog::size_t i) = 0;
   virtual const Message *operator[](MessageLog::size_t i) const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~MessageLog() {}
};

/**
 *  This interface represents an individual message in a message log.
 *  These messages can contain properties which describe the message in
 *  greater detail or provide useful parameters associated with a message.
 *
 *  Message is a subclass of Subject It will notify upon the following conditions:
 *  - The following methods are called: addProperty(), addBooleanProperty(),
 *    finalize(), 
 *  - Everything else documented in Subject.
 */
class Message : public Serializable, public Subject
{
public: // types
   /**
    *  Specifies the result state of a step
    */
   enum ResultEnum 
   { 
      Success,    /**< The step completed successfully. */
      Failure,    /**< The step failed to complete. */
      Abort,      /**< The step was aborted by the user. */
      Unresolved  /**< The step has not yet completed. */
   };

   /**
    * @EnumWrapper Message::ResultEnum.
    */
   typedef EnumWrapper<ResultEnum> Result;

public: // methods
   /**
    *  Emitted with any<Message*> when a property is added to the message.
    */
   SIGNAL_METHOD(Message, MessageModified)
   /**
    *  Emitted with any<Message*> when a message is finalized.
    */
   SIGNAL_METHOD(Message, Hidden)

   /**
    *  Adds a property to a message. Any number of properties can be added
    *  to a message.
    *
    *  @param   name
    *           A string naming the property being added. These must be unique 
    *           within a single message.
    *  @param   value
    *           The value of the property being added. This will be converted to
    *           a textual equivalent and recorded in the log.
    *
    *  @return  true if the property was successfully added; false otherwise.
    *
    *  @notify  This method will notify signalMessageModified with any<Message*>.
    *
    *  @see     DynamicObject::set
    */
   virtual bool addProperty(const std::string &name, const char *value) = 0;
   virtual bool addProperty(const std::string &name, char value) = 0;
   virtual bool addProperty(const std::string &name, unsigned char value) = 0;
   virtual bool addProperty(const std::string &name, short value) = 0;
   virtual bool addProperty(const std::string &name, unsigned short value) = 0;
   virtual bool addProperty(const std::string &name, int value) = 0;
   virtual bool addProperty(const std::string &name, unsigned int value) = 0;
   virtual bool addProperty(const std::string &name, long value) = 0;
   virtual bool addProperty(const std::string &name, unsigned long value) = 0;
   virtual bool addProperty(const std::string &name, Int64 value) = 0;
   virtual bool addProperty(const std::string &name, UInt64 value) = 0;
   virtual bool addProperty(const std::string &name, float value) = 0;
   virtual bool addProperty(const std::string &name, double value) = 0;
   virtual bool addProperty(const std::string &name, const std::string &value) = 0;
   virtual bool addProperty(const std::string &name, const Filename *value) = 0;
   virtual bool addProperty(const std::string &name, const DateTime *value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<char>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<unsigned char>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<short>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<unsigned short>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<int>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<unsigned int>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<long>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<unsigned long>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<Int64>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<UInt64>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<float>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<double>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<bool>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<std::string>& value) = 0;
   virtual bool addProperty(const std::string &name, const std::vector<const Filename*>& value) = 0;

   /**
    *  Adds a property to a message. Any number of properties can be added
    *  to a message.  This method has a different name from the others
    *  because if C++ cannot find a proper overload for the addProperty
    *  method, it will implicitly convert it to a bool.  If this method
    *  was called addProperty, it would be implicitly called by C++, without
    *  displaying a warning of any type during compilation.
    *
    *  @param   name
    *           A string naming the property being added. These must be unique 
    *           within a single message.
    *  @param   value
    *           The value of the property being added. This will be converted to
    *           a textual equivalent and recorded in the log.
    *
    *  @return  true if the property was successfully added; false otherwise.
    *
    *  @notify  This method will notify signalMessageModified with any<Message*>.
    *
    *  @see     DynamicObject::set
    */
   virtual bool addBooleanProperty(const std::string &name, bool value) = 0;

   /**
    *  If the message is a top level message, it is written to the log. If it is a
    *  sub-message, it is closed to further modification. No properties can 
    *  be added to the message or any submessages after finalization. The
    *  message pointer is no longer a valid pointer and must not be 
    *  de-referenced. Upon finalization of a message, the log will notify all
    *  of its observers, passing the message to them.
    *
    *  @return  true if successfully closed or written; false otherwise.
    *
    *  @notify  This method will notify signalHidden() with any<Message*>.
    */
   virtual bool finalize() = 0;

   /**
    *  Returns whether the message has been finalized by calling the finalize() method.
    *
    *  @return true if the message has been finalized; false otherwise.
    */
   virtual bool isFinalized() const = 0;

   /**
    *  Returns the name of the component that created this Message.
    *
    *  @return the name of the component that created this Message.
    */
   virtual std::string getComponent() const = 0;

   /**
    *  Returns the name of the key associated with this Message.
    *
    *  The component and key of this Message should uniquely
    *  identify this particular message instance, even between
    *  new versions of the software.
    *
    *  @return the name of the key associated with this Message.
    */
   virtual std::string getKey() const = 0;

   /**
    *  Get reference to the properties.
    *
    *  @return  A reference to the property vector
    */
   virtual const DynamicObject *getProperties() const = 0;

   /**
    *  Returns the string that was passed to the original createMessage or
    *  createStep call.
    *
    *  @return  The action string that the message was created with.
    */
   virtual std::string getAction() const = 0;

   /**
    *  Convert a property of an arbitrary (but valid) type to a string.
    *
    *  @param  type
    *          the type of pValue
    *  @param  pValue
    *          pointer to the value
    *  @return a string representation of pValue
    *
    *  @see    addProperty
    */
   virtual std::string propertyToString(const std::string &type, void *pValue) const = 0;

   /**
    *  Return a string representation of the ID number of this Message.
    *
    *  An ID number is a serious of point separated numbers such that
    *  sub-messages of Step 1 are numbered 1.1, 1.2, etc. and sub-messages
    *  of Step 1.3 are numbered 1.3.1, 1.3.2, etc.
    *
    *  @return a string representation of the ID
    */
   virtual std::string getStringId() = 0;

   /**
    *  Returns a string reprsentation of the date and time associated with this Message
    *
    *  @param  date
    *          output parameter which will hold the date string
    *  @param  time
    *          output parameter which will hold the time string
    */
   virtual void serializeDate(std::string &date, std::string &time) const = 0;

protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~Message() {}
};

/**
 *  This interface represents a type of message log Message. Steps represent
 *  individual algorithmic steps. Each can contain properties and substeps
 *  which break the algorithm down further.
 *
 *  Step is a subclass of Subject. It will notify upon the following conditions:
 *  - The following methods are called: addStep(), addMessage(), 
 *  - A sub-Step or Message notifies.  An unfinalized Step will forward any 
 *    notification.  pValue will be forwarded, containing a Message* of the
 *    originally notifying Message.
 *  - Everything else documented in Message.
 */
class Step : public Message
{
public: // types
   /**
    *  An STL iterator type. Has the semantics of an std::vector
    */
   typedef std::vector<Message*>::iterator iterator;

   /**
    *  An STL iterator type. Has the semantics of an std::vector
    */
   typedef std::vector<Message*>::const_iterator const_iterator;

   /**
    *  A type representing sizes and indices
    */
   typedef ::size_t size_t;

public: // methods
   /**
    *  Emitted with any<Message*> when a message is added to the step.
    */
   SIGNAL_METHOD(Step, MessageAdded)
   /**
    *  Emitted with any<Message*> when a message in the step is finalized.
    */
   SIGNAL_METHOD(Step, MessageHidden)
   /**
    *  Emitted with any<Message*> when a message is deleted from the step.
    */
   SIGNAL_METHOD(Step, MessageDeleted)
   /**
    *  Emitted with any<Message*> when a message is detached from the step.
    */
   SIGNAL_METHOD(Step, MessageDetached)

   /**
    *  Creates a new sub-step.
    *
    *  @param   action
    *           A string describing the nature of the event that triggered the message.
    *  @param   component
    *           A string specifying the component that is requesting a step
    *           be created, for example the name of the plug-in suite logging the step.
    *  @param   key
    *           A string that specifies a identifier for this step being
    *           created.  The key combined with the component shall uniquely
    *           identify a step.
    *  @param   recurse
    *           If this is true, steps are recursed and the new Step
    *           is added to the deepest un-finalized step.
    *
    *  @return  A pointer to the message created. This pointer will be valid until it
    *           or a parent message is finalized. This will return NULL if the parent
    *           Message object isn't a step.
    *
    *  @notify  This method will notify signalMessageAdded with any<Message*>.
    */
   virtual Step *addStep(const std::string &action,
                         const std::string &component,
                         const std::string &key, 
                         bool recurse = true) = 0;

   /**
    *  Adds a sub-message to a step.
    *
    *  @param   action
    *           A string describing the nature of the event that triggered the message.
    *  @param   component
    *           A string specifying the component that is requesting a message
    *           be created, for example the name of the plug-in suite logging the message.
    *  @param   key
    *           A string that specifies a identifier for this message being
    *           created.  The key combined with the component shall uniquely
    *           identify a message.
    *  @param   finalizeOnCreate
    *           If this is true, the Message will be automatically finalize()'d
    *  @param   recurse
    *           If this is true, steps are recursed and the Message
    *           is added to the deepest un-finalized step.
    *
    *  @return  A pointer to the message created. This pointer will be valid until it
    *           or a parent message is finalized.
    *
    *  @notify  This method will notify signalMessageAdded with any<Message*>.
    */
   virtual Message *addMessage(const std::string &action,
                               const std::string &component,
                               const std::string &key,
                               bool finalizeOnCreate = false,
                               bool recurse = true) = 0;

   /**
    *  If the step is a top-level step (i.e. not a sub-step), this method
    *  writes the message and any submessages to the log. No properties can 
    *  be added to the message or any submessages after finalization. The
    *  message pointer and pointers to all of its sub-messages are no longer 
    *  valid pointers and must not be de-referenced.
    *  This will recursively call finalize on all sub-steps and sub-messages,
    *  passing a Result of Success to the sub-steps.
    *  Calling finalize on a sub-step or sub-message closes the message
    *  or step and all sub-messages or sub-steps to modification, but does not 
    *  cause the sub-step or sub-message to be written to disk. Serialize only 
    *  actually writes to disk when called on a top-level step or message.
    *  Upon finalization of a message, the log will notify all of its observers.
    *
    *  @param   result
    *           An indication of the cause of the completion of the step.
    *
    *  @param   failureReason
    *           An message that indicates the reason for step failure.  This
    *           should ony be used to provided details in the case of
    *           message failure.
    *
    *  @return  true if successfully closed or written; false otherwise.
    */
   virtual bool finalize(Result result, const std::string& failureReason = "") = 0;

   /**
    * This method finalizes the current Step with the result being \link Message::Success\endlink
    * and an empty failure reason string.
    *
    * @return \c true if the step is successfully closed or written; otherwise returns \c false.
    */
   virtual bool finalize() = 0;

   /**
    *  This method returns the failure message passed to the finalize
    *  method or an string of length zero if no failure message was
    *  provided.
    *
    *  @return  the failure message provided to the finalize method.
    *
    *  @see     Step::finalize
    */
   virtual const std::string& getFailureMessage() const = 0;

   /**
    *  Get the result associated with this Step
    *
    *  @return  The Result of this Step.
    */
   virtual Result getResult() const = 0;

   /**
    *  Obtain the count of Messages in this %Step.
    *
    *  @return the number of Messages
    */
   virtual Step::size_t size() const = 0;

   /**
    *  Obtain an STL iterator over Messages in this %Step
    *  starting at the begining of the vector.
    *
    *  This iterator has the behavior of a std::vector::iterator
    *
    *  @return  an iterator over the %Step's Messages
    */
   virtual Step::iterator begin() = 0;

   /**
    *  Obtain an STL const_iterator over Messages in this %Step
    *  starting at the begining of the vector.
    *
    *  This iterator has the behavior of a std::vector::const_iterator
    *
    *  @return  a const_iterator over the %Step's Messages
    */
   virtual Step::const_iterator begin() const = 0;

   /**
    *  Obtain an STL iterator over Messages in this %Step
    *  starting at the end of the vector.
    *
    *  This iterator has the behavior of a std::vector::iterator
    *
    *  @return  an iterator over the %Step's Messages
    */
   virtual Step::iterator end() = 0;

   /**
    *  Obtain an STL const_iterator over Messages in this %Step
    *  starting at the end of the vector.
    *
    *  This iterator has the behavior of a std::vector::const_iterator
    *
    *  @return  a const_iterator over the %Step's Messages
    */
   virtual Step::const_iterator end() const = 0;

   /**
    *  Get the %Message at index \a i.
    *
    *  This function has the same semantics as std::vector::operator[]. It
    *  does not ensure that \a i is a valid index.
    *
    *  @param i The index to return.
    *
    *  @return The %Message requested
    */
   virtual Message *operator[](Step::size_t i) = 0;
   virtual const Message *operator[](Step::size_t i) const = 0;
 
protected:
   /**
    * This will be cleaned up during application close.  Plug-ins do not
    * need to destroy it.
    */
   virtual ~Step() {}
};

#endif
