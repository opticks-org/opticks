/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCRIPTOR_H
#define SCRIPTOR_H

#include "Interpreter.h"
#include "InterpreterManagerShell.h"
#include "SubjectAdapter.h"

#include <string>

class ScriptorExecutor : public Interpreter, public SubjectImp
{
public:
   ScriptorExecutor();
   virtual std::string getPrompt() const;
   virtual bool executeCommand(const std::string& command);
   virtual bool executeScopedCommand(const std::string& command, const Slot& output,
      const Slot& error, Progress* pProgress);
   virtual bool isGlobalOutputShown() const;
   virtual void showGlobalOutput(bool newValue);

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

private:
   bool executeCommandInternal(const std::string& command, bool scoped);
   void sendOutput(const std::string& text, bool scoped);
   void sendError(const std::string& text, bool scoped);
   SIGNAL_METHOD(ScriptorExecutor, ScopedOutputText);
   SIGNAL_METHOD(ScriptorExecutor, ScopedErrorText);

   void elementCreated(Subject& subject, const std::string& signal, const boost::any& data);

   bool mGlobalOutputShown;
   unsigned int mCommandNumber;
};

class Scriptor : public InterpreterManagerShell, public SubjectImp
{
public:
   Scriptor();
   virtual ~Scriptor() {}

   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool serialize(SessionItemSerializer &serializer) const;
   virtual bool deserialize(SessionItemDeserializer &deserializer);

   virtual bool isStarted() const;
   virtual bool start();
   virtual std::string getStartupMessage() const;
   virtual Interpreter* getInterpreter() const;

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

private:
   std::auto_ptr<ScriptorExecutor> mpInterpreter;
   unsigned int mStartCount;
};

class StartScriptorMenuItem : public ExecutableShell
{
public:
   StartScriptorMenuItem();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
