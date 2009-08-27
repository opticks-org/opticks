/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef INTERPRETERSHELL_H
#define INTERPRETERSHELL_H

#include "ExecutableShell.h"
#include "Interpreter.h"

#include <string>

/**
 *  \ingroup ShellModule
 *  %Interpreter Shell
 *
 *  This class represents the shell for an interpreter plug-in.  %Interpreter
 *  developers would take this class and extend it to support their 
 *  interpreter specific code.
 *
 *  @see     ExecutableShell, Interpreter, InterpreterExt1
 */
class InterpreterShell : public ExecutableShell, public Interpreter, public InterpreterExt1
{
public:
   /**
    *  Creates an interpreter plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::InterpreterType() and sets the
    *  plug-in to not be destroyed after execution.
    *
    *  @see     getType(), isDestroyedAfterExecute()
    */
   InterpreterShell();

   /**
    *  Destroys the interpreter plug-in.
    */
   virtual ~InterpreterShell();

   /**
    *  @copydoc Executable::getInputSpecification()
    *
    *  @default Adds the Interpreter::CommandArg() string argument.
    */
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default Adds the Interpreter::OutputTextArg() string argument
    *           and the Interpeter::ReturnTypeArg() string argument with a default value of "Output".
    */
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

   /**
    *  @copydoc InterpreterExt1::getPrompt()
    *
    *  @default Returns "interpreter_name> ".
    */
   virtual std::string getPrompt() const;

   /**
    *  @copydoc Interpreter::getKeywordList()
    *
    *  @default Clears the list for interpreters without special keywords.
    */
   virtual void getKeywordList(std::vector<std::string>& list) const;

   /**
    *  @copydoc Interpreter::getKeywordDescription()
    *
    *  @default Returns false.
    */
   virtual bool getKeywordDescription(const std::string& keyword, std::string& description) const;

   /**
    *  @copydoc Interpreter::getUserDefinedTypes()
    *
    *  @default Clears the list for interpreters without user defined types.
    */
   virtual void getUserDefinedTypes(std::vector<std::string>& list) const;

   /**
    *  @copydoc Interpreter::getKeywordDescription()
    *
    *  @default Returns false.
    */
   virtual bool getTypeDescription(const std::string& type, std::string& description) const;

   /**
    *  @copydoc Interpreter::getFileExtensions()
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

private:
   std::string mExtensions;
};

#endif
