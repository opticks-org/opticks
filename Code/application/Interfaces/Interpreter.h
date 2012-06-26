/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Subject.h"

#include <string>

class Progress;
class Slot;

/**
 *  Command interface specific to interpreter plug-ins.
 *
 *  Interpreter implementations implement this interface
 *  and this interface is used to execute commands
 *  for a given interpreter implementation.
 *
 *  An instance of this class should be retrieved
 *  by calling InterpreterManager::getInterpreter()
 *  on a InterpreterManager plug-in instance or
 *  using InterpreterUtilities::getInterpreter().
 */
class Interpreter : public Subject
{
public:
   /**
    *  Emitted with boost::any<std::string> with output text generated
    *  from executing commands provided to executeCommand().  May
    *  also be emitted with output text generated from executing
    *  commands provided to executeScopedCommand() if
    *  isGlobalOutputShown() returns \c true.
    */
   SIGNAL_METHOD(Interpreter, OutputText);

   /**
    *  Emitted with boost::any<std::string> with error text generated
    *  from executing commands provided to executeCommand().  May
    *  also be emitted with error text generated from executing
    *  commands provided to executeScopedCommand() if
    *  isGlobalOutputShown() returns \c true.
    */
   SIGNAL_METHOD(Interpreter, ErrorText);

   /**
    *  Retrieves the current prompt.
    *
    *  @return The current prompt that will be displayed to the user.
    */
   virtual std::string getPrompt() const = 0;

   /**
    *  Execute the given command.
    *  Regardless of the return value, future calls can be made to this
    *  function. This function blocks and will not return until all provided
    *  statements have been executed to completion.
    * 
    *  Variables created while executing the command persist after command
    *  execution in a global scope. This global scope should persist for
    *  an Interpreter instance so that later commands can access variables
    *  created during execution of earlier commands.  Please read
    *  executeScopedCommand() which supports different scoping rules.
    *
    *  @notify Output and error text from executing this command will cause
    *          SIGNAL_NAME(Interpreter, OutputText) and SIGNAL_NAME(Interpreter, ErrorText)
    *          to be emitted.  The output and error text is still valid regardless
    *          of the return value of this function.
    *
    *  @param command
    *         The command to execute, this command can be a single statement
    *         or multi-statements. The syntax for single statements and
    *         multi-statements is defined by the given interpreter
    *         implementation.
    *
    *  @return \c False if there was a syntatic problem with the given command,
    *          the interpreter is not running, or an exception.
    *          was thrown from the interpreter. \c True will be returned otherwise.
    */
   virtual bool executeCommand(const std::string& command) = 0;

   /**
    *  Execute the given command.
    *  Regardless of the return value, future calls can be made to this
    *  function. This function blocks and will not return until all provided
    *  statements have been executed to completion.
    * 
    *  Output and error text from executing this command will be be sent
    *  to the provided output and error Slot instances. The output and
    *  error text is still valid regardless of the return value of this function.
    *
    *  Variables created while executing the provided command can be
    *  created in a nested scope that is closed at command completion
    *  causing local variables to be freed.
    *  A given interpreter implementation can support
    *  this nested scope behavior or can share the global and persistent
    *  scope used by executeCommand().
    *
    *  @notify Output and error text from executing this command will cause
    *          SIGNAL_NAME(Interpreter, OutputText) and SIGNAL_NAME(Interpreter, ErrorText)
    *          to be emitted if isGlobalOutputShown() returns \c true.
    *
    *  @param command
    *         The command to execute, this command can be a single statement
    *         or multi-statements. The syntax for single statements and
    *         multi-statements is defined by the given interpreter
    *         implementation.
    *
    *  @param output
    *         This Slot will be called as output text is generated from
    *         executing this command. The boost::any provided to the
    *         slot will contain a std::string with the output text.
    *
    *  @param error
    *         This Slot will be called as error text is generated from
    *         executing this command. The boost::any provided to the
    *         slot will contain a std::string with the error text.
    *
    *  @param pProgress
    *         If a non-NULL progress is provided, the execution of the provided
    *         command may report progress to this instance. A given
    *         interpreter implementation is permitted to ignore this parameter.
    *         An interpreter will only report progress to this progress instance
    *         while executing the provided command.
    *
    *  @return \c False if there was a syntatic problem with the given command,
    *          the interpreter is not running, or an exception.
    *          was thrown from the interpreter. \c True will be returned otherwise.
    */
   virtual bool executeScopedCommand(const std::string& command, const Slot& output,
      const Slot& error, Progress* pProgress) = 0;

   /**
    *  Returns the last value provided to showGlobalOutput(). The default
    *  value for a given interpreter implementation may be \c true or \c false.
    *
    *  @return The last value provided to showGlobalOutput().
    */
   virtual bool isGlobalOutputShown() const = 0;

   /**
    *  Controls whether output from executeScopedCommand() is emitted
    *  from SIGNAL_NAME(Interpreter, OutputText) and SIGNAL_NAME(Interpreter, ErrorText).
    *
    *  @param newValue
    *         A value of \c true will cause any ouput or error text
    *         generated from calling executeScopedCommand() to also be
    *         emitted from SIGNAL_NAME(Interpreter, OutputText) and
    *         SIGNAL_NAME(Interpreter, ErrorText).  A value of \c false
    *         causes output or error text generated from calling
    *         executeScopedCommand() to only be sent to the Slot instances
    *         passed to executeScopedCommand().
    */
   virtual void showGlobalOutput(bool newValue) = 0;

protected:
   /**
    * This Interpreter instance is owned by a InterpreterManager instance.
    */
   virtual ~Interpreter() {}
};

#endif
