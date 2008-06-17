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

class QObject;

/**
 *  %Interpreter Shell
 *
 *  This class represents the shell for an interpreter plug-in.  %Interpreter
 *  developers would take this class and extend it to support thier 
 *  interpreter specific code.
 *
 *  @see     ExecutableShell, Interpreter
 */
class InterpreterShell : public ExecutableShell, public Interpreter
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
   ~InterpreterShell();

   /**
    *  @copydoc Interpreter::getFileExtensions()
    *
    *  @default The default implementation returns the extension string that
    *           was passed into setFileExtensions().  If setFileExtensions()
    *           has not yet been called, an empty string is returned.
    */
   std::string getFileExtensions() const;

#ifdef HAVE_QSCINTILLA
   virtual QextScintillaLexer *getLexer(QObject *parent) = 0;
#endif

protected:
   /**
    *  Sets the default scripting file extensions recognized by the interpreter.
    *
    *  @param   extensions
    *           The file extensions recognized by the interpreter.  The string
    *           should consist of a description followed by one or more
    *           extensions separated by a space.  Multiple file types may
    *           be specified with a double semicolon.  Examples include
    *           "Landsat Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   void setFileExtensions(const std::string& extensions);

private:
   std::string mExtensions;
};

#endif
