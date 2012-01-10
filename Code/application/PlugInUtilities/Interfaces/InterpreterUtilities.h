/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPRETERUTILITIES_H
#define INTERPRETERUTILITIES_H

#include "Slot.h"

#include <string>
#include <vector>

class Interpreter;
class Progress;

/**
 *  Utility functions to make executing commands with an given Interpreter
 *  implementation easier.
 */
namespace InterpreterUtilities
{
   /**
    *  Executes the provided statement(s) with the given interpreter while
    *  reporting output and error text from command execution.
    *  This function blocks and will not return until all provided
    *  statements have been executed to completion.
    *
    *  Variables created while executing the provided command can be
    *  created in a nested scope that is closed at command completion
    *  causing local variables to be freed.
    *  A given interpreter implementation can support
    *  this nested scope behavior or can share the global and persistent
    *  scope used by Interpreter::executeCommand().
    *
    *  @param interpreterName
    *         The name of the interpreter to execute commands with.
    *         This should correspond to PlugIn::getName() of the given
    *         InterpreterManager instance.
    *
    *  @param command
    *         The command to execute. This may be a single statement
    *         or multiple statements. The syntax for single statements and
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
    *  @return \c False if a InterpreterManager instance with the given name cannot
    *          not be located or is not already started.  Returns \c false if there
    *          was a syntatic problem with the given command, or an exception. 
    *          was thrown from the interpreter. \c true will be returned otherwise.
    *
    *  @see Interpreter::executeScopedCommand()
    */
   bool executeScopedCommand(const std::string& interpreterName, const std::string& command,
                             const Slot& output, const Slot& error, Progress* pProgress);

   /**
    *  Executes the provided statement(s) with the given interpreter while
    *  reporting output and error text from command execution.
    *  This function blocks and will not return until all provided
    *  statements have been executed to completion.
    *
    *  Variables created while executing the provided command can be
    *  created in a nested scope that is closed at command completion
    *  causing local variables to be freed.
    *  A given interpreter implementation can support
    *  this nested scope behavior or can share the global and persistent
    *  scope used by Interpreter::executeCommand().
    *
    *  @param interpreterName
    *         The name of the interpreter to execute commands with.
    *         This should correspond to PlugIn::getName() of the given
    *         InterpreterManager instance.
    *
    *  @param command
    *         The command to execute. This may be a single statement
    *         or multiple statements. The syntax for single statements and
    *         multi-statements is defined by the given interpreter
    *         implementation.
    *
    *  @param outputAndError
    *         The provided string will be populated with all of the
    *         output and error text reported while executing the command.
    *
    *  @param hasErrorText
    *         The provided bool will be set to \c true if any error
    *         text was reported while executing the command will be
    *         set to \c false if only output text was reported.
    *
    *  @param pProgress
    *         If a non-NULL progress is provided, the execution of the provided
    *         command may report progress to this instance. A given
    *         interpreter implementation is permitted to ignore this parameter.
    *         An interpreter will only report progress to this progress instance
    *         while executing the provided command.
    *
    *  @return \c False if a InterpreterManager instance with the given name cannot
    *          not be located or is not already started.  Returns \c false if there
    *          was a syntatic problem with the given command, or an exception. 
    *          was thrown from the interpreter. \c true will be returned otherwise.
    *
    *  @see Interpreter::executeScopedCommand()
    */
   bool executeScopedCommand(const std::string& interpreterName, const std::string& command,
                             std::string& outputAndError, bool& hasErrorText, Progress* pProgress);

   /**
    *  Queries whether an interpreter is available and has been started.
    *
    *  This function will NOT start the interpreter, it must
    *  have already been started using InterpreterManager::start()
    *  for this function to return \c true.
    *
    *  @param interpreterName
    *         The name of the interpreter to execute commands with.
    *         This should correspond to PlugIn::getName() of the given
    *         InterpreterManager instance.
    *
    *  @return \c True if an InterpreterManager instance with the given
    *          name can be located and InterpreterManager::isStarted()
    *          returns \c true. Otherwise \c false is returned.
    */
   bool isInterpreterAvailable(const std::string& interpreterName);

   /**
    *  Returns the names of all Interpreter implementations (e.g.
    *  all plug-ins that return PlugInManagerServices::InterpreterManagerType()
    *  from PlugIn::getType() ).  The implementations will be
    *  added to the list even if InterpreterManager::isStarted()
    *  returns \c false.
    *
    *  @return The names of all Interpreter implementations.
    */
   std::vector<std::string> getInterpreters();

   /**
    *  Returns an Interpreter instance for the given name that can
    *  be used to execute commands.
    *
    *  @param interpreterName
    *         The name of the interpreter to execute commands with.
    *         This should correspond to PlugIn::getName() of the given
    *         InterpreterManager instance.
    *
    *  @return An Interpreter instance if isInterpreterAvailable()
    *          would return \c true, otherwise \c NULL is returned.
    */
   Interpreter* getInterpreter(const std::string& interpreterName);
};

#endif
