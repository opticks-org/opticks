/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPU_PROGRAM_H
#define GPU_PROGRAM_H

#include "glCommon.h"
#include "GpuProgramDescriptor.h"

#include <string>
#include <map>
#include <vector>

class ColorBuffer;

/**
 *  This class is a wrapper class that contains all the necessary information
 *  to apply a Graphical Processor Unit (GPU) program.
 */
class GpuProgram
{
public:
  /**
   *  Constructor to the GpuProgram class.
   *
   *  @param   pGpuDescriptor
   *           GPU Program descriptor.
   */
   explicit GpuProgram(GpuProgramDescriptor* pGpuDescriptor);
   
  /**
   *  Destructor to the GpuProgram class.
   */
   virtual ~GpuProgram();

  /**
   *  Binds the GPU program to the specific processor
   *  so that when the data is rendered the program will
   *  process it.
   *
   *  @see  setGpuParameters()
   */
   void bind();

  /**
   *  Disables the specific GPU program.
   *
   *  @see  bind()
   */
   void disable();

  /**
   *  Returns the GPU program descriptor from which
   *  it was created. The Gpu program descriptor is
   *  used to set the parameters for the GPU program.
   *  Every time the GPU program gets bound the parameters
   *  are set using the GPU program descriptor.
   */
   GpuProgramDescriptor& getGpuProgramDescriptor() { return *mpGpuProgramDescriptor; }

  /**
   * Sets the color buffers to be used when passing in the OpenGL texture ids 
   * to the GPU program's input parameters. The map has the name of the input
   * parameter as its key to get the associated color buffer that has the texture id.
   */
   void setColorBuffers(const std::map<std::string, ColorBuffer*>& colorBuffers)
   {
      mColorBuffers = colorBuffers;
   }

#if defined(CG_SUPPORTED)
   CGprogram getProgramId() { return mProgramId; }
#endif

private: // member methods
  /**
   *  Sets the necessary GPU input parameters for the program.
   *
   *  @see  bind()
   */
   virtual bool setGpuParameters();

#if defined(CG_SUPPORTED)
   CGprogram mProgramId;
   CGprofile mProfile;
#endif
   GpuProgramDescriptor *mpGpuProgramDescriptor;
   GLuint mInputImage;
   std::map<std::string, ColorBuffer*> mColorBuffers;
};

#endif /* GPU_PROGRAM_H */
