/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPRETERMANAGERSHELL_H
#define INTERPRETERMANAGERSHELL_H

#include "ExecutableShell.h"
#include "InterpreterManager.h"
#include "SubjectImp.h"

#include <string>

/**
 *  \ingroup ShellModule
 *  %Interpreter Manager Shell
 *
 *  This class represents the shell for an interpreter plug-in.  %Interpreter
 *  developers would take this class and extend it to support their 
 *  interpreter specific code.
 *
 *  An implementation must provide the following additional methods:
 *  - InterpreterManager::isStarted()
 *  - InterpreterManager::start()
 *  - InterpreterManager::getStartupMessage()
 *  - InterpreterManager::getInterpreter()
 *  - Executable::execute()
 *     Your implementation of this method should attempt
 *     to start the interpreter as part of application start-up
 *  - All methods on Subject and its base classes
 *
 *  @see ExecutableShell, InterpreterManager
 */
class InterpreterManagerShell : public ExecutableShell, public InterpreterManager
{
public:
   /**
    *  Creates the interpreter plug-in.
    *
    *  The constructor initializes the plug-in as follows:
    *  - getType() returns PlugInManagerServices::InterpreterManagerType()
    *  - getSubtype() returns an empty std::string.
    *  - isExecutedOnStartup() returns \c true.
    *  - isDestroyedAfterExecute() returns \c false.
    *  - hasAbort() returns \c false.
    *  - hasWizardSupport() returns \c false.
    *  - areMultipleInstancesAllowed() returns \c false
    *  - isInteractiveEnabled() returns \c true
    */
   InterpreterManagerShell();

   /**
    *  Destroys the interpreter plug-in.
    */
   virtual ~InterpreterManagerShell();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default Initializes the pArgList to NULL, indicating no input arguments.
    */
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default Initializes the pArgList to NULL, indicating no output arguments.
    */
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc InterpreterManager::isInteractiveEnabled()
    *
    *  @default Returns the last value passed into setInteractiveEnabled(). If
    *           setInteractiveEnabled() has not yet been called, \c true will
    *           be returned.
    */
   virtual bool isInteractiveEnabled() const;

   /**
    *  @copydoc InterpreterManager::getFileExtensions()
    *
    *  @default The default implementation returns the extension string that
    *           was passed into setFileExtensions().  If setFileExtensions()
    *           has not yet been called, an empty string is returned.
    */
   virtual std::string getFileExtensions() const;

protected:
   /**
    *  Sets the default scripting file extensions recognized by the interpreter.
    *
    *  @param   extensions
    *           The file extensions recognized by the interpreter.  The string
    *           should consist of a description followed by one or more
    *           extensions separated by a space.  Multiple file types may
    *           be specified with a double semicolon.  Examples include
    *           "ENVI Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   void setFileExtensions(const std::string& extensions);

   /**
    *  Sets the default interactive support for the interpreter.
    *
    *  @param enabled
    *         \c True will enable interactive support and \c false
    *         will disable interactive support.
    *
    *  @see InterpreterManager::isInteractiveEnabled()
    */
   void setInteractiveEnabled(bool enabled);

private:
   std::string mExtensions;
   bool mInteractiveEnabled;
};

#endif
