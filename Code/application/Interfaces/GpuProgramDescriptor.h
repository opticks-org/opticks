/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPUPROGRAMDESCRIPTOR_H
#define GPUPROGRAMDESCRIPTOR_H

#include "EnumWrapper.h"
#include <string>
#include <vector>

class DataVariant;
class DynamicObject;

/**
 * This class describes a GPU program including I/O parameters.
 */
class GpuProgramDescriptor
{
public:
   /**
    * This type specifies the GPU shader type
    */
   enum GpuProgramTypeEnum
   {
      VERTEX_PROGRAM,   /**< The program should run on the GPU's vertex processor */
      FRAGMENT_PROGRAM  /**< The program should run on the GPU's fragment processor */
   };

   /**
   * @EnumWrapper ::GpuProgramTypeEnum.
   */
   typedef EnumWrapper<GpuProgramTypeEnum> GpuProgramType;

   /**
    * Mutate the name of this GPU program.
    *
    * @param name
    *        The new name
    */
   virtual void setName(const std::string &name) = 0;

   /**
    * Access the name of this GPU program.
    *
    * @return The name
    */
   virtual const std::string& getName() const = 0;

   /**
    * Mutate the type of this GPU program.
    *
    * @param type
    *        The new type
    */
   virtual void setType(GpuProgramType type) = 0;

   /**
    * Access the type of this GPU program.
    *
    * @return The type
    */
   virtual GpuProgramType getType() const = 0;

   /**
    * Remove a parameter from this GPU program's parameter specification.
    *
    * @param name
    *        The name of the parameter to remove
    *
    * @return True if successful, false if the parameter does not exist
    */
   virtual bool removeParameter(const std::string &name) = 0;

   /**
    * Set the value of a parameter in this GPU program's parameter specification.
    *
    * @param name
    *        The name of the parameter to set
    *
    * @param value
    *        The new value of the parameter
    *
    * @return True if the parameter's value was set, false if the parameter does not exist
    */
   virtual bool setParameter(const std::string &name, const DataVariant &value) = 0;

   /**
    * Get all the parameters and values in this GPU program's parameter specification.
    *
    * @return A DynamicObject containing the parameter names and values
    */
   virtual const DynamicObject *getParameters() const = 0;

   /**
    * Get the value of a parameter in this GPU program's parameter specification.
    *
    * @param name
    *        The name of the parameter to get
    *
    * @return The value of the parameter. If the parameter does not exist, this will
    *         be an invalid DataVariant.
    */
   virtual const DataVariant& getParameter(const std::string &name) const = 0;

   /**
    *  Create a cloned copy of this GpuProgramDescriptor.
    *
    *  @return A copy of this GpuProgramDescriptor or NULL if there was an
    *          error creating the copy.
    */
   virtual GpuProgramDescriptor *copy() const = 0;

protected:
   /**
    * This is automatically destroyed by the application.
    */
   virtual ~GpuProgramDescriptor() {}
};

#endif
