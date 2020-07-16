#include "RgbGpuProgram.h"
#include "GpuProgramDescriptorAdapter.h"


/**
* Constructor
*/
RgbGpuProgram::RgbGpuProgram()
{
   GpuProgramDescriptorAdapter* pGpuDescriptor = new GpuProgramDescriptorAdapter();
   pGpuDescriptor->setName("RgbDisplay.glsl");
   pGpuDescriptor->setType(GpuProgramDescriptor::FRAGMENT_PROGRAM);
   setGpuDescriptor(pGpuDescriptor);

}

/**
* Initialize the program. This must be called in the EventDispatchThread.
* 
* @throws IOException
*            If the descriptor source is invalid.
*/
void RgbGpuProgram::initialize()
{
   DataVariant value(std::string("0"));
   mpGpuProgramDescriptor->setParameter("inputImage", value);

   DataVariant defaultStretch(std::string("0.0"));
   mpGpuProgramDescriptor->setParameter("dataMax", defaultStretch);
   mpGpuProgramDescriptor->setParameter("redDataMax", defaultStretch);
   mpGpuProgramDescriptor->setParameter("greenDataMax", defaultStretch);
   mpGpuProgramDescriptor->setParameter("blueDataMax", defaultStretch);
   mpGpuProgramDescriptor->setParameter("redLowerValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("redUpperValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("greenLowerValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("greenUpperValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("blueLowerValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("blueUpperValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("alphaValue", defaultStretch);
   mpGpuProgramDescriptor->setParameter("minBound", defaultStretch);
   mpGpuProgramDescriptor->setParameter("maxBound", defaultStretch);
   GlShaderGpuProgram::initialize();
}