/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINARGIMP_H
#define PLUGINARGIMP_H

#include "PlugInArg.h"

#include "DataVariant.h"

#include <string>
#include <vector>

class QDataStream;

/**
 * PlugIn Argument
 *
 * This class is responsible for managing the information for a single argument
 * to be passed to a PlugIn.  It maintains the data type and name, default 
 * value (if any), and actual value.
 *
 *  @see     PlugInArgList
 */
class PlugInArgImp  : public PlugInArg
{
public:
   /**
    *  Default constructor for the Plug-In Argument.
    *
    *  This is the default constructor which initializes
    *  to create a blank Plug-In. 
    */
   PlugInArgImp();

   /**
    *  Constructor for the Plug-In Argument.
    *
    *  This constructor initializes to create a Plug-In with a given 
    *  data type and name.  The default and actual values are not set.
    *
    *  @param   type
    *           String representation of the data type for this Argument.
    *  @param   name
    *           Name of this Argument.
    */
   PlugInArgImp(const std::string& type, const std::string& name);

   /**
    *  Constructor for the Plug-In Argument.
    *
    *  This constructor initializes to create a Plug-In with a given 
    *  data type, name, and default value.  The actual value is not set.
    *
    *  @param   type
    *           String representation of the data type for this Argument.
    *  @param   name
    *           Name of this Argument.
    *  @param   pDefaultValue
    *           Pointer to the default value for this Argument.
    */
   PlugInArgImp(const std::string& type, const std::string& name, const void* pDefaultValue);

   /**
    *  Constructor for the Plug-In Argument.
    *
    *  This constructor initializes to create a Plug-In with a given 
    *  data type, name, default value, and actual value.
    *
    *  @param   type
    *           String representation of the data type for this Argument.
    *  @param   name
    *           Name of this Argument.
    *  @param   pDefaultValue
    *           Pointer to the default value for this Argument.
    *  @param   pActualValue
    *           Pointer to the actual value for this Argument.
    */
   PlugInArgImp(const std::string& type, const std::string& name, const void* pDefaultValue, const void* pActualValue);

   /**
    *  Destructor for the Plug-In Argument.
    *
    *  This destructor removes all dynamic references.
    */
   ~PlugInArgImp();

   const std::string& getName() const;
   const std::string& getType() const;
   void* getDefaultValue() const;
   bool isDefaultSet() const;
   void* getActualValue() const;
   bool isActualSet() const;
   void setDefaultValue(const void* pValue, bool tryDeepCopy = true);
   void setActualValue(const void* pValue, bool tryDeepCopy = true);
   void setName(const std::string& name);
   bool setType(const std::string& type);

   void setDescription(const std::string& description);
   const std::string& getDescription() const;

   /**
    *  Get this object's type.
    *
    *  The taoGetType() method is an implementation of a virtual method in 
    *  TypeAwareObject.  It returns a string indicating the type of object, 
    *  in this case "PlugInArg".
    *
    *  @return  This method returns a string specifying the type of object.
    */
   const std::string& getObjectType() const;

   /**
    *  Determine if this object is of the given type.
    *
    *  The isKindOf() method is an implementation of a virtual method in 
    *  TypeAwareObject.  It returns a flag indicating whether this object
    *  is of the specified type.
    *
    *  @param   className
    *           String containing the object type to check.
    *
    *  @return  This method returns true if the input string is "PlugInArg",
    *           otherwise calls isKind on the parent class.
    */
   bool isKindOf(const std::string& className) const;

   /**
    *  Get the names of the known arg types.
    *
    *  This method is returns the names of each recognized arg type.  Plug-ins
    *  can retrieve this list to query if a data type is in the known list.  The
    *  wizard builder uses this list connect plug-in in sequence according to their
    *  type names.  If an arg type exists that is not in this list, the wizard builder
    *  indicates it as an unknown type.
    *
    *  @return   A vector of strings specifying the name of each known arg type.
    */
   static const std::vector<std::string>& getArgTypes();

   static PlugInArgImp* fromSettings(QDataStream& reader);
   bool updateSettings(QDataStream& writer) const;

protected:
   static void initArgTypes();
   static bool isArgType(const std::string& type);

private:
   /**
    * The copy constructor and operator= are private
    * and have no implementation defined because this
    * class should not support those operations.  Making
    * these functions private prevents the compiler
    * from generating incorrect versions.
    */
   PlugInArgImp(const PlugInArgImp& rhs);
   PlugInArgImp(const PlugInArg& rhs);
   PlugInArgImp& operator=(const PlugInArgImp& rhs);

   std::string mType;
   std::string mName;
   bool mDefaultSet;
   void* mpDefaultValueShallowCopy;
   DataVariant mDefaultValueDeepCopy;
   bool mActualSet;
   void* mpActualValueShallowCopy;
   DataVariant mActualValueDeepCopy;
   std::string mDescription;

   static std::vector<std::string> mArgTypes;
};

#endif
