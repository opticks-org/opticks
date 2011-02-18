/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPRETERMANAGER_H
#define INTERPRETERMANAGER_H

#include "Subject.h"

#include <string>

class Interpreter;

/**
 *  All interpreters must implement this interface
 *  which defines how the application and plug-ins
 *  execute commands with an interpreter implementation.
 *
 *  %Interpreter implementations must also implement
 *  the PlugIn interface.  It is recommended a
 *  interpreter implementation subclass from
 *  InterpreterManagerShell.
 *
 *  An interpreter implementation must also
 *  provide an implementation of the Interpreter
 *  interface.
 *
 *  @see Interpreter
 */
class InterpreterManager : public Subject
{
public:
   /**
    *  Emitted when the interpreter starts.  If
    *  a given interpreter implementation executes
    *  during application start-up, this signal
    *  may never be emitted.  It only suggested
    *  you attach to this signal if calling
    *  isStarted() returns \c false.  This
    *  signal is permitted to be emitted even
    *  if no calls to start() have been
    *  made (i.e. the user entered required configuration
    *  information into a dialog while the application
    *  was running).
    */
   SIGNAL_METHOD(InterpreterManager, InterpreterStarted);

   /**
    *  Returns if the interpreter is started
    *  and running.
    *
    *  If this returns \c true, getInterpreter()
    *  must return \c non-NULL.  If this returns
    *  \c false, getInterpreter() must return \c NULL.
    *
    *  @return \c True if the interpreter is
    *          started and running, \c false
    *          otherwise.
    *
    *  @see getStartupMessage()
    */
   virtual bool isStarted() const = 0;

   /**
    *  Attempt to start the interpreter.
    *  This method can be called multiple times without
    *  any ill effect.  This method can be called even
    *  if the interpreter is already started and no
    *  changes will be made to the already running
    *  interpreter. If this method returns \c true,
    *  subsequent calls to isStarted() must
    *  return \c true
    *
    *  If this result of calling this method will change
    *  the return value of \c isStarted() from \c false
    *  to \c true, then
    *  SIGNAL_NAME(InterpreterManager, InterpreterStarted)
    *  must be emitted.
    *
    *  @return \c True if the interpreter
    *          was started or was already
    *          running.  Returns \c false
    *          if the interpreter was not
    *          already running and could
    *          not be started.
    *
    *  @see getStartupMessage()
    */
   virtual bool start() = 0;

   /**
    *  Returns the startup message from the last
    *  failed start or the message from a 
    *  successful start.
    *
    *  @return The startup message from the last
    *          failed start or the message from a
    *          successful start.
    *
    *  @see start(), isStarted()
    */
   virtual std::string getStartupMessage() const = 0;

   /**
    *  Returns whether the user should be permitted
    *  to enter commands into the Scripting Window.
    *
    *  If \c true is returned, the user will be
    *  permitted to enter commands into the Scripting
    *  Window.  If \c false, the Scripting Window
    *  will be made read-only.  The value returned
    *  from this function must not change between
    *  calls to this function.
    *
    *  The behavior of Interpreter::executeCommand()
    *  and Interpreter::executeScopedCommand() must
    *  NOT be different based upon the return
    *  value of this function.
    *
    *  @return Returns whether the user should be permitted
    *          to enter commands into the Scripting Window.
    */
   virtual bool isInteractiveEnabled() const = 0;

   /**
    *  Returns the Interpreter instance that can be used
    *  to execute commands.
    *
    *  The lifetime of the returned instance
    *  is guaranteed to be at least as long as the lifetime
    *  of the InterpreterManager instance this method was
    *  called on.  However, after the InterpreterManager
    *  instance has been deleted, the returned Interpreter
    *  instance should be considered invalid.
    *
    *  @return The Interpreter instance that can be
    *          used to execute commands.  This must
    *          return NULL if a call to isStarted()
    *          \c false and must return non-NULL if
    *          a call to isStarted() returns \c true.
    */
   virtual Interpreter* getInterpreter() const = 0;

   /**
    *  Returns the default scripting file extensions recognized by the interpreter.
    *
    *  @return  The file extensions recognized by the interpreter as a string.
    *           The string consists of a description followed by one or more
    *           file extensions separated by a space.  Multiple file
    *           types may be specified with a double semicolon.  Examples
    *           include "ENVI Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   virtual std::string getFileExtensions() const = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyPlugIn.
    */
   virtual ~InterpreterManager() {}
};

#endif
