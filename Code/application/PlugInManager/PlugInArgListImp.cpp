/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <memory>
#include <stdio.h>
#include <string.h>

#include "ModuleDescriptor.h"
#include "PlugInArgImp.h"
#include "PlugInArgListImp.h"

#include <QtCore/QDataStream>

using namespace std;

PlugInArgListImp::PlugInArgListImp()
{}

/**
 *  Destructor for the Plug-In Argument List.
 *
 *  This destructor removes all dynamic references.
 */
PlugInArgListImp::~PlugInArgListImp()
{
   int iCount = mArgList.size();
   for (int i = 0; i < iCount; i++)
   {
      PlugInArg* pArg = mArgList[i];
      if (pArg != NULL)
      {
         delete static_cast<PlugInArgImp*>(pArg);
      }
   }
   mArgList.clear();
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
}

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
bool PlugInArgListImp::addArg(const PlugInArg& piArg)
{
   const PlugInArgImp* pTemp = static_cast<const PlugInArgImp*>(&piArg);
   try
   {
      mArgList.push_back(const_cast<PlugInArgImp*>(pTemp));
   }
   catch (...)
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
bool PlugInArgListImp::getArg(const string& name, PlugInArg*& piArg) const
{
   piArg = NULL;

   for (unsigned int i = 0; i < mArgList.size(); ++i)
   {
      string s1 = mArgList[i]->getName();
      if (s1 == name)
      {
         piArg = mArgList[i];
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
bool PlugInArgListImp::getArg(int argNumber, PlugInArg*& piArg) const
{
   piArg = NULL;

   int iArgs = static_cast<int>(mArgList.size());
   if (argNumber < iArgs)
   {
      piArg = mArgList[argNumber];
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
bool PlugInArgListImp::catenateLists(const PlugInArgList& plugInArg)
{
   PlugInArg* pArg = NULL;
   bool success;

   for (int i = 0; i < static_cast<int>(plugInArg.getCount()); ++i)
   {
      success = plugInArg.getArg(i, pArg);
      if (!success)
      {
         return false;
      }

      success = addArg(*pArg);
      if (!success)
      {
         return false;
      }
   }
   return true;
}

PlugInArgListImp* PlugInArgListImp::fromSettings(QDataStream& reader)
{
   unsigned short count;
   READ_FROM_STREAM(count);
   auto_ptr<PlugInArgListImp> pList(new PlugInArgListImp());
   for (unsigned short i = 0; i < count; ++i)
   {
      bool haveArg;
      READ_FROM_STREAM(haveArg);
      if (!haveArg)
      {
         pList->mArgList.push_back(NULL);
      }
      else
      {
         PlugInArgImp* pArg = PlugInArgImp::fromSettings(reader);
         if (pArg == NULL)
         {
            return NULL;
         }
         pList->addArg(*pArg);
      }
   }
   return pList.release();
}

bool PlugInArgListImp::updateSettings(QDataStream& writer) const
{
   unsigned short count = getCount();
   writer << count;
   for (unsigned short i = 0; i < count; ++i)
   {
      PlugInArgImp* pArg = mArgList[i];
      if (pArg == NULL)
      {
         writer << false;
         continue;
      }
      else
      {
         writer << true;
         if (!pArg->updateSettings(writer))
         {
            return false;
         }
      }
   }
   return true;
}