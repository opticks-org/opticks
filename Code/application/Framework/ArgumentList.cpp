/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ArgumentList.h"
#include "AppConfig.h"

using namespace std;

//
// The attribute singleton is the only allowable reference
// to the ArgumentList singleton class.
//
ArgumentList* ArgumentList::singleton = NULL;

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
ArgumentList* ArgumentList::instance()
{
   if (singleton == NULL)
   {
      singleton = new ArgumentList;
   }

   return singleton;
}

/**
 *  Constructor which can NOT be called outside this class.
 *
 *  The default constructor is protected.  The allows the instance()
 *  method to control the number of instances of this class.  To
 *  instantiate this class the ArgumentList::instance() 
 *  method must be called.
 */
ArgumentList::ArgumentList() :
   mValidArgumentList(true)
{
#if defined(WIN_API)
   setDelimiter("/");
#else
   setDelimiter("-");
#endif
}

/**
 * Destructor which can not be invoked outside this class.
 *
 * This destructor deletes all dynamic memory associated with the
 * class.
 */
ArgumentList::~ArgumentList()
{
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
void ArgumentList::set(int argc, char* argv[])
{
   if (argv[0] != NULL)
   {
      mExecutableName = argv[0];
   }

   for (int i = 0; i < argc; ++i)
   {
      if (argv[i] != NULL)
      {
         mArguments.push_back(argv[i]);
      }
   }

   determineValidOptions();
}

/**
 *  Retrieves the base executable name from the command line 
 *  argument list.  This returns the executable name without
 *  the path to the executable included.
 *
 *  @param   name
 *           Name of the executable base name only.
 */
string ArgumentList::getExecutableBaseName()
{
   unsigned int slash;
   string name = getExecutableName();

#if defined(WIN_API)
   slash = name.find_last_of("\\") + 1;
   unsigned int period = name.find_last_of(".");
   name = name.substr(slash, period - slash);
#else
   slash = name.find_last_of("/") + 1;
   name = name.erase(0, slash + 1);
#endif

   return name;
}

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
vector<string> ArgumentList::getOptions(string optionName)
{
   vector<string>::iterator argument;
   vector<string> newArguments;

   if (!optionName.empty())
   {
      optionName = mDelimiter + optionName + ":";
   }

   for (argument = mArguments.begin(); argument != mArguments.end(); ++argument)
   {
      string option(argument->c_str());

      if (optionName.empty())
      {
         if (argument != mArguments.begin() && !option.empty() && option.substr(0, 1) != mDelimiter)
         {
            newArguments.push_back(option);
         }
         continue;
      }

      int pos = option.compare(0, optionName.length(), optionName);
      if (pos == 0)
      {
         option = option.erase(0, optionName.length());
         newArguments.push_back(option);
      }

   }
   return newArguments;
}

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
string ArgumentList::getOption(string optionName)
{
   vector<string> values = getOptions(optionName);
   if (values.empty())
   {
      return "";
   }

   return *(values.begin());
}

/**
 *  Does the option exist in the argument list?
 *
 *  @param   name
 *           Name of the argument value.
 *  @return  Returns true if the argument exists in the argument list.
 */
bool ArgumentList::exists(string name)
{
   bool valid = false;

   map<string, bool>::iterator option = mOptions.find(mDelimiter + name);
   if (option != mOptions.end())
   {
      valid = (option->second);
   }

   return valid;
}

/**
 *  Goes throught the list of options and determine if only valid
 *  options have been used.
 */
void ArgumentList::determineValidOptions()
{
   map<string, bool>::iterator option;
   mValidArgumentList = true;

   for (vector<string>::iterator argument = mArguments.begin(); argument != mArguments.end(); ++argument)
   {
      option = mOptions.find(*argument);
      if (option != mOptions.end())
      {
         option->second = true;
      }
   }
}
