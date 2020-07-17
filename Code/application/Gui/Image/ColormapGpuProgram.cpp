/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColormapGpuProgram.h"
#include "GpuProgramDescriptorAdapter.h"


/**
* Constructor
*/
ColormapGpuProgram::ColormapGpuProgram()
{
   GpuProgramDescriptorAdapter* pGpuDescriptor = new GpuProgramDescriptorAdapter();
   pGpuDescriptor->setName("ColormapDisplay.glsl");
   pGpuDescriptor->setType(GpuProgramDescriptor::FRAGMENT_PROGRAM);
   setGpuDescriptor(pGpuDescriptor);

}

/**
* Initialize the program. This must be called in the EventDispatchThread.
* 
* @throws IOException
*            If the descriptor source is invalid.
*/
void ColormapGpuProgram::initialize()
{
   DataVariant value(std::string("0"));
   mpGpuProgramDescriptor->setParameter("colorMap", value);
   mpGpuProgramDescriptor->setParameter("inputImage", value);
   mpGpuProgramDescriptor->setParameter("colorMap", value);
   mpGpuProgramDescriptor->setParameter("numColors", value);

   DataVariant defaultStretch(std::string("0.0"));
   mpGpuProgramDescriptor->setParameter("dataMax", defaultStretch);
   mpGpuProgramDescriptor->setParameter("lowerValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("upperValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("alphaValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("minBound", defaultStretch);
   mpGpuProgramDescriptor->setParameter("maxBound", defaultStretch);
   GlShaderGpuProgram::initialize();
}
