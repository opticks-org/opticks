/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <stdio.h>
#include <string.h>

#include "PlugInArgImp.h"
#include "PlugInArgListImp.h"

using namespace std;

PlugInArgListImp::PlugInArgListImp()
{
}

PlugInArgListImp::PlugInArgListImp(const PlugInArgList& rhs)
{
   const PlugInArgList* listInterface = &rhs;
   const PlugInArgListImp* list = reinterpret_cast<const PlugInArgListImp*>(listInterface);
   PlugInArgImp *arg;
   bool success;

   for (int i=0; i<list->getCount(); i++) {
      success = list->getArg( i, (PlugInArg*&)arg);
      if (!success) break;
      success = addArg(*arg);
      if (!success) break;
   }
}

PlugInArgListImp::PlugInArgListImp(const PlugInArgListImp& rhs)
{
    if (&rhs != this) {
      PlugInArgImp *arg;
      bool success;

      for (int i=0; i<rhs.getCount(); i++) {
         success = rhs.getArg( i, (PlugInArg*&)arg);
         if (!success) break;
         success = addArg(*arg);
         if (!success) break;
      }
    }
}

/**
 *  Destructor for the Plug-In Argument List.
 *
 *  This destructor removes all dynamic references.
 */
PlugInArgListImp::~PlugInArgListImp()
{
    emptyList();
}

/**
 *  Empty the Plug-In Argument List.
 *
 *  The emptyList() method clears the vector of Plug-In Arguments, 
 *  but does NOT destruct the individual Plug-In Arguments.  It 
 *  simply removes the references to them from the vector.
 */
void PlugInArgListImp::emptyList()
{
    mArgList.clear();
};

/**
 *  Add a Plug-In Argument to the list.
 *
 *  The addArg() method adds the given Plug-In Argument to the and of 
 *  the existing Argument list, and returns a flag indicating whether 
 *  it was successful.
 *
 *  @param   PlugInArg
 *           Reference to the Plug-In Argument to be added to the list.
 *
 *  @return  The method returns true if the Plug-In Argument was 
 *           successfully added to the list, otherwise returns false.
 */
bool PlugInArgListImp::addArg( const PlugInArg& piArg )
{
    const PlugInArgImp* temp = reinterpret_cast<const PlugInArgImp*>(&piArg);
    try
    {
        mArgList.push_back( (PlugInArgImp*) temp );
    }
    catch(...)
    {
        // An error occured, so return false
        return false;
    }
    return true;
}

/**
 *  Get a Plug-In Argument from the list.
 *
 *  The getArg() method sets an output parameter to point to the 
 *  desired Plug-In Argument, if it exists, and returns a flag 
 *  indicating success status.
 *
 *  @param   argName
 *           Reference to the Plug-In Argument to be added to the list.
 *
 *  @param   arg
 *           Pointer that will be set to the desired Plug-In Argument 
 *           if found, or NULL if not found.
 *
 *  @return  The method returns true if the Plug-In Argument was 
 *           successfully found in the list, otherwise returns false.
 */
bool PlugInArgListImp::getArg( const string& name, PlugInArg*& piArg ) const
{
   piArg = NULL;

   for (unsigned int i = 0; i < mArgList.size(); i++)
   {
      string s1 = mArgList[i]->getName();
      if (s1 == name)
      {
         piArg = (PlugInArg*) mArgList[i];
         return true;
      }
   }

   return false;
}

/**
 *  Get a Plug-In Argument from the list.
 *
 *  The getArg() method sets an output parameter to point to the 
 *  desired Plug-In Argument, if it exists, and returns a flag 
 *  indicating success status.
 *
 *  @param   index
 *           Index into the vector containing the Plug-In Argument List.
 *           Since arguments are always added to the end of the vector,
 *           it is possible to pull them back out using an offset 
 *           number, rather than specifying the name of the Argument.
 *           An index of 0 corresponds to the first argument in the list, 
 *           and an index of getCount()-1 corresponds to the last 
 *           argument in the list.
 *
 *  @param   arg
 *           Pointer that will be set to the desired Plug-In Argument 
 *           if found, or NULL if not found.
 *
 *  @return  The method returns true if the index was a valid index in 
 *           the list, otherwise returns false.
 */
bool PlugInArgListImp::getArg( int argNumber, PlugInArg*& piArg ) const
{
   piArg = NULL;

   int iArgs = 0;
   iArgs = mArgList.size();
   if (argNumber < iArgs)
   {
      piArg = (PlugInArg*) mArgList[argNumber];
      return true;
   }

   return false;
}

/**
 *  Get the number of Arguments in the Plug-In Argument List.
 *
 *  The getCount() method returns the number of arguments that have 
 *  been added to the list using addArg() or catenateLists().
 *
 *  @return  The method returns the number of elements in the Plug-In Argument List, or 0 if none have been added.
 */
unsigned short PlugInArgListImp::getCount() const
{
   return mArgList.size();
}

/**
 *  Concatenate a Plug-In Argument List to the end of this list.
 *
 *  The catenateLists() method adds each Argument in the given Plug-In 
 *  Argument List to the current list using addArg().
 *
 *  @param   argList
 *           Plug-In Argument List whose elements are to be added to 
 *           the end of this List.
 *
 *  @return  The method returns true if all items in argList were 
 *           successfully added to this list, otherwise returns false.
 */
bool PlugInArgListImp::catenateLists( const PlugInArgList& plugInArg )
{
    PlugInArgImp *arg = NULL;
    bool success;

    for (int i=0; i<plugInArg.getCount(); i++)
    {
        success = plugInArg.getArg( i, (PlugInArg*&)arg);
        if (!success) return false;
        success = addArg(*arg);
        if (!success) return false;
    }
    return true;
}
