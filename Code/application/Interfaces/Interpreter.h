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

#include <string>
#include <vector>

/**
 *  Interface specific to interpreter plug-ins.
 *
 *  Defines the interpreter specific interface to all interpreter plug-ins.
 *  This interface contains all interpreter specific operations.
 */
class Interpreter
{
public:
   /**
    * The name to use for the command argument.
    *
    * This argument will be populated with the command to run.
    * Arguments with this name should be of the type string.
    */
   static std::string CommandArg() { return "Command"; }

   /**
    * The name to use for the output text argument in the output argument list.
    *
    * This argument should be populated with the output text from running the
    * command specified in CommandArg(). Arguments with this name should be 
    * of the type string.
    */
   static std::string OutputTextArg() { return "Output Text"; }

   /**
    * The name to use for the return type argument in the output argument list.
    *
    * This argument should be populated with the type of return from running the
    * command specified in CommandArg(). Arguments with this name should be 
    * of the type string.
    */
   static std::string ReturnTypeArg() { return "Return Type"; }

   /**
    *  Retrieves the list of keywords for the interpreter.
    *
    *  @param   list
    *           A list that is populated with the keywords for the interpreter.
    */
   virtual void getKeywordList(std::vector<std::string>& list) const = 0;

   /**
    *  Retrieves a description for a given keyword.
    *
    *  @param   keyword
    *           The keyword to lookup.
    *  @param   description
    *           The description of the keyword.
    *
    *  @return  Returns \b true if the description was successfully retrieved,
    *           otherwise returns \b false.
    */
   virtual bool getKeywordDescription(const std::string& keyword, std::string& description) const = 0;

   /**
    *  Retrieves a list of user-defined types.
    *
    *  @param   list
    *           A list that is populated with the user-defined types.
    */
   virtual void getUserDefinedTypes(std::vector<std::string>& list) const = 0;

   /**
    *  Retrieves a description for a given type.
    *
    *  @param   type
    *           The type to lookup.
    *  @param   description
    *           The description of the type.
    *
    *  @return  Returns \b true if the description was successfully retrieved,
    *           otherwise returns \b false.
    */
   virtual bool getTypeDescription(const std::string& type, std::string& description) const = 0;

   /**
    *  Returns the default scripting file extensions recognized by the interpreter.
    *
    *  @return  The file extensions recognized by the interpreter as a string.
    *           The string consists of a description followed by one or more
    *           file extensions separated by a space.  Multiple file
    *           types may be specified with a double semicolon.  Examples
    *           include "Landsat Header Files (*.hdr)", "TIFF Files (*.tif *.tiff)",
    *           and "Source Files (*.c*);;Header Files (*.h)".
    */
   virtual std::string getFileExtensions() const = 0;

protected:
   /**
    *  Since the Interpreter interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~Interpreter() {}
};

#endif
