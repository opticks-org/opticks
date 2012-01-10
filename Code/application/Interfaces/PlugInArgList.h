/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINARGLIST_H
#define PLUGINARGLIST_H

#include "PlugInArg.h"
#include "PlugInManagerServices.h"
#include "TypeConverter.h"

#include <string>
#include <vector>

/**
 *  All input/output arguments for a plug-in
 *
 *  This class is responsible for managing all plug-in arguments that are required 
 *  for a plug-in argument list.
 *
 *  @see     PlugInArg
 */
class PlugInArgList
{
public:
   /**
    *  Empty the Plug-In Argument List.
    *
    *  The emptyList() method clears the vector of Plug-In Arguments, 
    *  but does NOT destruct the individual Plug-In Arguments.  It 
    *  simply removes the references to them from the vector.
    */
   virtual void emptyList() = 0;

   /**
    * Add an argument to a PlugInArgList with a default.
    *
    * @param name
    *        The name for the argument.
    * @param defaultValue
    *        The default value for the argument.
    * @param description
    *        The user-centric description for the argument.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   template<typename T>
   bool addArg(const std::string& name, const T& defaultValue, const std::string &description = std::string())
   {
      Service<PlugInManagerServices> pPims;
      
      PlugInArg* pArg = pPims->getPlugInArg();
      if (pArg != NULL)
      {
         pArg->setName(name);
         pArg->setType(TypeConverter::toString<T>());
         pArg->setDefaultValue(&defaultValue);
         pArg->setDescription(description);
         return addArg(*pArg);
      }

      return false;
   }

   /**
    * Add an argument to a PlugInArgList with a default.
    *
    * This version allows NULL defaults.
    *
    * @param name
    *        The name for the argument.
    * @param pDefaultValue
    *        The default value for the argument as a pointer.
    * @param description
    *        The user-centric description for the argument.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   template<typename T>
   bool addArg(const std::string& name, const T* pDefaultValue, const std::string &description = std::string())
   {
      Service<PlugInManagerServices> pPims;
      
      PlugInArg* pArg = pPims->getPlugInArg();
      if (pArg != NULL)
      {
         pArg->setName(name);
         pArg->setType(TypeConverter::toString<T>());
         pArg->setDefaultValue(pDefaultValue);
         pArg->setDescription(description);
         return addArg(*pArg);
      }

      return false;
   }

   /**
    * Add an argument to a PlugInArgList without a default.
    *
    * @param name
    *        The name for the argument.
    * @param description
    *        The user-centric description for the argument.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   template<typename T>
   bool addArg(const std::string &name, const std::string &description = std::string())
   {
      Service<PlugInManagerServices> pPims;
      
      PlugInArg* pArg = pPims->getPlugInArg();
      if (pArg != NULL)
      {
         pArg->setName(name);
         pArg->setType(TypeConverter::toString<T>());
         pArg->setDescription(description);
         return addArg(*pArg);
      }

      return false;
   }

   /**
    *  Add a Plug-In Argument to the list.
    *
    *  The addArg() method adds the given Plug-In Argument to the and of 
    *  the existing Argument list, and returns a flag indicating whether 
    *  it was successful.
    *
    *  @param   arg
    *           Reference to the Plug-In Argument to be added to the list.
    *
    *  @return  The method returns true if the Plug-In Argument was 
    *           successfully added to the list, otherwise returns false.
    */
   virtual bool addArg(const PlugInArg& arg) = 0;

   /**
    *  Returns the actual or default value for this plug-in argument.
    *
    *  This version returns a pointer to the type requested. This is useful for model types
    *  which must always be used as pointers.
    *
    *  @param   name
    *           The name of the plug-in argument to retrive.
    *
    *  @return  A pointer to the actual value for this plug-in argument
    *           if the actual value has been set.  Otherwise the default
    *           value is returned if set. If neither is set or the argument
    *           is not present in the arg list or the argument type does 
    *           not match that provided exactly, NULL is returned
    */
   template<typename T>
   T* getPlugInArgValue(const std::string &name) const
   {
      PlugInArg *pArg = NULL;
      if(!getArg(name, pArg) || (pArg == NULL))
      {
         return NULL;
      }
      return pArg->getPlugInArgValue<T>();
   }

   /**
    *  Returns the actual or default value for this plug-in argument.
    *
    *  This version returns a pointer to the type requested. This is useful for model types
    *  which must always be used as pointers.
    *
    *  @param   name
    *           The name of the plug-in argument to retrive.
    *
    *  @warning This function does not do any type checking.
    *           If the type of the argument does not match the
    *           template argument, the return value is undefined.
    *
    *  @return  A pointer to the actual value for this plug-in argument
    *           if the actual value has been set.  Otherwise the default
    *           value is returned if set. If neither is set or the argument
    *           is not present in the arg list, NULL is returned
    */
   template<typename T>
   T* getPlugInArgValueUnsafe(const std::string &name) const
   {
      PlugInArg *pArg = NULL;
      if(!getArg(name, pArg) || (pArg == NULL))
      {
         return NULL;
      }
      return pArg->getPlugInArgValueUnsafe<T>();
   }

   /**
    *  Returns the actual or default value for this plug-in argument.
    *
    *  This version sets the output argument to the value of the requested plug-in argument. This
    *  version is useful for basic types and other types that have an equals operator.
    *
    *  @param   name
    *           The name of the plug-in argument to retrive.
    *
    *  @param   value
    *           Output argument which will contain the value of the argument. This will
    *           remain unchanged if the argument is not present in the arg list or
    *           there the actual and default values are not set.
    *
    *  @return  False if the argument is not present in the arg list or the actual
    *           and default argument values are not set, or the type of argument
    *           does not exactly match that provided.
    *           True if the actual or default value for the argument is set. If the
    *           actual or default argument value is a NULL pointer, the output argument
    *           will remain unchanged and this function will return true.
    */
   template<typename T>
   bool getPlugInArgValue(const std::string &name, T &value) const
   {
      PlugInArg *pArg = NULL;
      if(!getArg(name, pArg) || (pArg == NULL))
      {
         return false;
      }
      if (TypeConverter::toString<T>() != pArg->getType())
      {
         return false;
      }

      T *pValue = pArg->getPlugInArgValueUnsafe<T>();
      if(pValue == NULL)
      {
         return pArg->isActualSet() || pArg->isDefaultSet();
      }
      value = *pValue;
      return true;
   }

   /**
    *  Sets the actual value for a plug-in argument.
    *
    *  @param   name
    *           The name of the plug-in argument for which to set its value.
    *  @param   pValue
    *           The actual value of the argument.
    *
    *  @return  Returns \c true if the value is successfully set into the arg.
    *           Returns \c false if the argument is not present in the arg list
    *           or if the type of the given value is not the same as the arg
    *           type.
    *
    *  @see     PlugInArg::setPlugInArgValue()
    */
   template<typename T>
   bool setPlugInArgValue(const std::string &name, T *pValue)
   {
      PlugInArg *pArg = NULL;
      if(!getArg(name, pArg) || (pArg == NULL))
      {
         return false;
      }

      return pArg->setPlugInArgValue(pValue);
   }

   /**
    *  Set the actual value for a plug-in argument.
    *
    *  This method performs loose type checking, which requires that \c T be a
    *  subclass of TypeAwareObject.  If \c T is not the same or a subclass of
    *  the argument type, this method will fail.
    *
    *  @param   name
    *           The name of the plug-in argument for which to set its value.
    *  @param   pValue
    *           The actual value of the argument.
    *
    *  @return  Returns \c true if the value is successfully set into the arg.
    *           Returns \c false if the argument is not present in the arg list
    *           or if the type of the given value is not a kind of the arg type.
    *
    *  @see     PlugInArg::setPlugInArgValueLoose()
    */
   template<typename T>
   bool setPlugInArgValueLoose(const std::string &name, T *pValue)
   {
      PlugInArg *pArg = NULL;
      if(!getArg(name, pArg) || (pArg == NULL))
      {
         return false;
      }

      return pArg->setPlugInArgValueLoose(pValue);
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
    *  @param   arg
    *           Pointer that will be set to the desired Plug-In Argument 
    *           if found, or NULL if not found.
    *
    *  @return  The method returns true if the Plug-In Argument was 
    *           successfully found in the list, otherwise returns false.
    */
   virtual bool getArg(const std::string& argName, PlugInArg*& arg) const = 0;

   /**
    *  Get a Plug-In Argument from the list.
    *
    *  This method sets an output parameter to point to the 
    *  desired Plug-In Argument, if it exists, and returns a flag 
    *  indicating success status.
    *
    *  @param   argNumber
    *           Index into the vector containing the Plug-In Argument List.
    *           Since arguments are always added to the end of the vector,
    *           it is possible to pull them back out using an offset 
    *           number, rather than specifying the name of the Argument.
    *           An index of 0 corresponds to the first argument in the list, 
    *           and an index of getCount()-1 corresponds to the last 
    *           argument in the list.
    *  @param   arg
    *           Pointer that will be set to the desired Plug-In Argument 
    *           if found, or NULL if not found.
    *
    *  @return  The method returns true if the index was a valid index in 
    *           the list, otherwise returns false.
    */
   virtual bool getArg(int argNumber, PlugInArg*& arg) const = 0;

   /**
    *  Get the number of Arguments in the Plug-In Argument List.
    *
    *  The getCount() method returns the number of arguments that have 
    *  been added to the list using addArg() or catenateLists().
    *
    *  @return  The method returns the number of elements in the Plug-In 
    *           Argument List, or 0 if none have been added.
    */
   virtual unsigned short getCount() const = 0;

   /**
    *  Concatenate a Plug-In Argument List to the end of this list.
    *
    *  The catenateLists() method adds each Argument in the given Plug-In 
    *  Argument List to the current list using addArg().
    *
    *  @param   plugInArg
    *           Plug-In Argument List whose elements are to be added to 
    *           the end of this List.
    *
    *  @return  The method returns true if all items in argList were 
    *           successfully added to this list, otherwise returns false.
    */
   virtual bool catenateLists(const PlugInArgList& plugInArg) = 0;

protected:
   /**
    * This should be destroyed by calling PlugInManagerServices::destroyPlugInArgList.
    */
   virtual ~PlugInArgList() {}
};

#endif   // PLUGINARGLIST_H
