/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGE_FILTER_DESCRIPTOR_H
#define IMAGE_FILTER_DESCRIPTOR_H

#include "EnumWrapper.h"
#include <string>
#include <vector>

class GpuProgramDescriptor;

/**
 * Provides information on image filter.
 *
 * This class provides information on a specific image filter, which 
 * is implemented via Graphical Processor Unit (GPU) programs. It
 * provides the name, description, GPU programs, and parameters
 * for the image filter.
 *
 */
class ImageFilterDescriptor
{
public:
   /**
    * This type specifies how the shader program will be treated by the application.
    */
   enum ImageProcessTypeEnum
   {
      GPU_PROCESS,      /**< Generic process running on Gpu. */
      IMAGE_FILTER,     /**< Image filter process running on Gpu. */
      FEEDBACK_FILTER   /**< Recursive filter process running on Gpu. */
   };

   /**
   * @EnumWrapper ::ImageProcessTypeEnum.
   */
   typedef EnumWrapper<ImageProcessTypeEnum> ImageProcessType;

   /**
    *  Gets the name of the image filter.
    *
    *  @return   Name of the image filter.
    *           
    */
   virtual const std::string& getName() const = 0;

   /**
    *  Gets the description of the image filter.
    *
    *  @return  Description of the image filter.
    *
    */
   virtual const std::string& getDescription() const = 0;

   /**
    *  Gets the type of the image filter.
    *
    *  @return   Type of the image filter.
    */
   virtual ImageProcessType getType() const = 0;

   /**
    *  Queries for one of the image filter's GPU programs in the list.
    *
    *  @param   gpuDescriptor
    *           GPU Program descriptor.
    *
    *  @return  True if GPU program is in image filter descriptor list, 
    *           otherwise false.
    *
    *  @see     addGpuProgram()
    */
   virtual bool hasGpuProgram(const GpuProgramDescriptor& gpuDescriptor) const = 0;

   /**
    *  Gets the image filter's GPU programs list.
    *
    *  @return  The image filter's GPU programs.
    */
   virtual const std::vector<GpuProgramDescriptor*>& getGpuPrograms() const = 0;

protected:
   /**
    * This is automatically destroyed by the application.
    */
   virtual ~ImageFilterDescriptor() {}
};

#endif
