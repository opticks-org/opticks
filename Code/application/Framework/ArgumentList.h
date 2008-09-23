/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARGUMENTLIST_H
#define ARGUMENTLIST_H

#include <string>
#include <vector>
#include <map>

/**
 * Argument List
 *
 * This is a singleton class.  Only one instance of this class exists at
 * a given time.  Use the instance() method to get a reference to the class.
 * ArgumentList is responsible for managing the Argument List passed into 
 * the application.
 */
class ArgumentList
{
public:
   /**
    *  Returns the instance of this singleton class.
    *
    *  The instance() method controls the instantiation of this class  
    *  and returns a reference to the singleton instance.  If the class 
    *  has not been instantiated, it creates the instance, stores
    *  it internally, and then returns a reference to the new 
    *  reference.
    *
    *  @return  This method returns the singleton class instance.
    */
   static ArgumentList* instance();

   /**
    *  Sets the command line string for the application.
    *  The arguments are read from a default system call.
    */
   void clear()
   {
      arguments.clear();
   }

   /**
    *  Sets the command line string for the application.
    *  This takes the standard main() C-style arguments and 
    *  parses them to create a 
    *
    *  @param   argc
    *           The number of arguments in the argv array, including
    *           the executable name as argv[0].
    *  @param   argv
    *           The array of arguments, including the executable name 
    *           as argv[0].
    */
   void set( int argc, char* argv[] );

   /**
    *  Registers an argument for processing and validation.
    *
    *  @param   name
    *           Name of the new option/argument.
    */
   void registerOption( std::string name )
   {
      options.insert(std::map<std::string,bool>::value_type(delimiter+name,false));
   }

   /**
    *  Removes an argument for processing and validation.
    *
    *  @param   name
    *           Name of the old option/argument.
    */
   void unregisterOption( std::string name )
   {
      std::map<std::string,bool>::iterator option;
      option = options.find( name );
      if (option!=options.end()) options.erase(option);
   }

   /**
    *  Retrieves the executable name from the command line 
    *  argument list.
    *
    *  @param   name
    *           Name of the executable including path
    */
   std::string getExecutableName()
   {
      return executableName;
   }

   /**
    *  Retrieves the base executable name from the command line 
    *  argument list.  This returns the executable name without
    *  the path to the executable included.
    *
    *  @param   name
    *           Name of the executable base name only.
    */
   std::string getExecutableBaseName();

   /**
    *  Filter the list of arguments.
    *
    *  The method getOptions() takes an option without the delimiter
    *  searches the argument list and returns corresponding argument
    *  values.  For example, searching for "Jvm" in the argument list 
    *  containing "/Jvm:-Dhost=localhost /Jvm:-Dserver=true" returns 
    *  "-Dhost=localhost" and "-Dserver=true".
    *
    *  @param   optionName
    *           The string name for the option without the delimiter.
    *  @return  The list of option values.
    */
   std::vector<std::string> getOptions( std::string optionName );

   /**
    *  Filter the list of arguments.
    *
    *  The method getOption() takes an option without the delimiter
    *  searches the argument list and returns the first corresponding 
    *  argument value.  For example, searching for "Jvm" in the argument list 
    *  containing "/Jvm:-Dhost=localhost /Jvm:-Dserver=true" returns 
    *  "-Dhost=localhost".
    *
    *  @param   optionName
    *           The string name for the option without the delimiter.
    *  @return  The first option value.
    */
   std::string getOption( std::string optionName );

   /**
    *  Does the option exist in the argument list?
    *
    *  @param   name
    *           Name of the argument value.
    *  @return  Returns true if the argument exists in the argument list.
    */
   bool exists( std::string name );

   /** 
    *  Is the current list of options valid registered options?
    *
    *  @return  Returns true if the arguments are valid.
    */
   bool valid()
   {
      return validArgumentList;
   }

   /**
    *  Gets the delimiter used to denote arguments.  "/" is the common
    *  for windows applications, and "-" is common for UNIX applications.
    *
    *  @return  value
    *           Delimiter string value
    */
   std::string getDelimiter()
   {
      return delimiter;
   }

protected:
   /**
    *  Constructor which can NOT be called outside this class.
    *
    *  The default constructor is protected.  The allows the instance()
    *  method to control the number of instances of this class.  To
    *  instantiate this class the ArgumentList::instance() 
    *  method must be called.
    */
   ArgumentList(); 

   /**
    * Destructor which can not be invoked outside this class.
    *
    * This destructor deletes all dynamic memory associated with the
    * class.
    */
   ~ArgumentList();

private:
   /**
    *  Sets the delimiter used to denote arguments.  "/" is the common
    *  for windows applications, and "-" is common for UNIX applications.
    *
    *  @param   value
    *           Delimiter string value
    */
   void setDelimiter( std::string value )
   {
      delimiter = value;
   }

   /**
    *  Goes throught the list of options and determine if only valid
    *  options have been used.
    */
   void determineValidOptions();

   static ArgumentList* singleton;

   std::string delimiter;
   std::vector<std::string> arguments;
   std::map<std::string,bool> options;
   bool validArgumentList;
   std::string executableName;
};

#endif   // ARGUMENTLIST_H

