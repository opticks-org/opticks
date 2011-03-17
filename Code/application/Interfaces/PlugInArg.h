/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINARG_H
#define PLUGINARG_H

#include "TypeAwareObject.h"
#include "TypeConverter.h"

#include <string>

/**
 *  Containers for individual input and output plug-in parameters.
 *
 *  Defines the interface that may be used by plug-ins to manipulate 
 *  their input and output arguments.
 *
 *  @see     TypeAwareObject, PlugInArgList
 */
class PlugInArg : virtual public TypeAwareObject
{
public:
   /**
    *  Retrieves the plug-in argument name.
    *
    *  @return  The argument name.
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Retrieves the plug-in argument data type.
    *
    *  @return  A string representing the argument data type.
    */
   virtual const std::string& getType() const = 0;

   /**
    *  Returns the actual or default value for this plug-in argument.
    *
    *  @warning This function does not do any type checking.
    *           If the type of the argument does not match the
    *           template argument, the return value is undefined.
    *
    *  @return  A pointer to the actual value for this plug-in argument
    *           if the actual value has been set.  Otherwise the default
    *           value is returned if set. If neither is set,
    *           NULL is returned.
    */
   template<typename T>
   T* getPlugInArgValueUnsafe()
   {
      T *pVal(NULL);
      if(isActualSet())
      {
         pVal = static_cast<T*>(getActualValue());
      }
      else if(isDefaultSet())
      {
         pVal = static_cast<T*>(getDefaultValue());
      }
      return pVal;
   }

   /**
    *  Returns the actual or default value for this plug-in argument.
    *
    *  @return  A pointer to the actual value for this plug-in argument
    *           if the actual value has been set.  Otherwise the default
    *           value is returned if set. If neither is set, or the type
    *           of this argument does not exactly match that provided, 
    *           NULL is returned.
    */
   template<typename T>
   T* getPlugInArgValue()
   {
      if (TypeConverter::toString<T>() != getType())
      {
         return NULL;
      }

      return getPlugInArgValueUnsafe<T>();
   }

   /**
    *  Returns the default value for this plug-in argument.
    *
    *  @return  A pointer to the default value for this plug-in
    *           argument if a default value has been set.  Otherwise
    *           NULL is returned.
    */
   virtual void* getDefaultValue() const = 0;

   /**
    *  Queries whether the default value for the plug-in argument has
    *  been set.
    *
    *  @return  True if a default value has been set for the argument,
    *           otherwise returns false.
    */
   virtual bool isDefaultSet() const = 0;
    
   /**
    *  Returns the default value for this plug-in argument.
    *
    *  @return  A pointer to the actual value for this plug-in argument
    *           if the actual value has been set.  Otherwise NULL is
    *           returned.
    */
   virtual void* getActualValue() const = 0;

   /**
    *  Queries whether the actual value for the plug-in argument has
    *  been set.
    *
    *  @return  True if the actual value has been set for the argument,
    *           otherwise returns false.
    */
   virtual bool isActualSet() const = 0;

   /**
    *  Sets the actual value for this plug-in argument.
    *
    *  The type of the given value must be the same as the arg type.
    *
    *  @param   pValue
    *           The actual value of the argument.
    *
    *  @return  Returns \c true if the value is successfully set into the arg.
    *           Returns \c false if type of the given value is not the same as
    *           the arg type.
    *
    *  @see     TypeConverter::toString()
    */
   template<typename T>
   bool setPlugInArgValue(T* pValue)
   {
      if (TypeConverter::toString<T>() != getType())
      {
         return false;
      }

      setActualValue(reinterpret_cast<void*>(pValue));
      return true;
   }

   /**
    *  Sets the actual value for this plug-in argument.
    *
    *  This method performs loose type checking, which requires that \c T be a
    *  subclass of TypeAwareObject.  If \c T is not the same or a subclass of
    *  the argument type, this method will fail.
    *
    *  @param   pValue
    *           The actual value of the argument.
    *
    *  @return  Returns \c true if the value is successfully set into the arg.
    *           Returns \c false if the type of the given value is not a kind
    *           of the arg type.
    *
    *  @see     TypeAwareObject::isKindOf()
    */
   template<typename T>
   bool setPlugInArgValueLoose(T* pValue)
   {
      TypeAwareObject* pTypeAwareObject = dynamic_cast<TypeAwareObject*>(pValue);
      if ((pTypeAwareObject != NULL) && (pTypeAwareObject->isKindOf(getType()) == false))
      {
         return false;
      }

      setActualValue(reinterpret_cast<void*>(pValue));
      return true;
   }

   /**
    *  Sets the argument's default value.
    *
    *  This method sets the argument's default value to the given value
    *  and sets a flag indicating that a default value has been set.
    *
    *  @param  pDefValue
    *          A pointer to the default value.  The value type must be
    *          identical to getType().
    *
    *  @param  tryDeepCopy
    *          If true, the PlugInArg will attempt to make a deep copy
    *          of the provided value for types that DataVariant can
    *          hold.  If false or DataVariant cannot hold the given
    *          type, the PlugInArg will directly store
    *          and hold the provided pDefValue and will not take
    *          ownership.
    */
   virtual void setDefaultValue(const void* pDefValue, bool tryDeepCopy = true) = 0;

   /**
    *  Sets the argument's actual value.
    *
    *  This method sets the argument's actual value to the given value
    *  and sets a flag indicating that the actual value has been set.
    *
    *  @param  pValue
    *          A pointer to the actual value.  The value type must be
    *          identical to getType().
    *
    *  @param  tryDeepCopy
    *          If true, the PlugInArg will attempt to make a deep copy
    *          of the provided value for types that DataVariant can
    *          hold.  If false or DataVariant cannot hold the given
    *          type, the PlugInArg will directly store
    *          and hold the provided pValue and will not take
    *          ownership.
    */
   virtual void setActualValue(const void* pValue, bool tryDeepCopy = true) = 0;

   /**
    *  Sets the argument name.
    *
    *  @param   name
    *           A string containing the new argument name, which cannot
    *           be empty.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Sets the argument type.
    *
    *  @param   type
    *           A string containing the new argument type.
    *
    *  @return  True if the type was successfully set, otherwise false.
    */
   virtual bool setType(const std::string& type) = 0;

   /**
    * Set the user-centric description of this argument.
    *
    * @param description
    *        The description of this argument.
    */
   virtual void setDescription(const std::string &description) = 0;

   /**
    * Get the user-centric description of this argument.
    *
    * @return The description of this argument.
    */
   virtual const std::string &getDescription() const = 0;

protected:
   /**
    * This cannot be destroyed directly.  This should be destroyed
    * by destroying the PlugInArgList it belongs to.
    *
    * @see PlugInArgList::~PlugInArgList.
    */
   virtual ~PlugInArg() {}
};

#endif
