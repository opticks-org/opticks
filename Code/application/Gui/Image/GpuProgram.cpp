/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "ColorBuffer.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "GpuProgram.h"

#include "GlSlContext.h"

using namespace std;

GpuProgram::GpuProgram() :
   mpGpuProgramDescriptor(NULL)
{
}

GpuProgram::GpuProgram(GpuProgramDescriptor *pGpuDescriptor) :
   mpGpuProgramDescriptor(pGpuDescriptor),
   mInputImage(0)
{
   REQUIRE(mpGpuProgramDescriptor != NULL);
   // get name of program file
   string programFileName = mpGpuProgramDescriptor->getName();
   REQUIRE(programFileName.empty() != true);

   // get type of GPU program
   GpuProgramDescriptor::GpuProgramType programType = mpGpuProgramDescriptor->getType();

   REQUIRE(GlSlContext::instance() != NULL);
}

GpuProgram::~GpuProgram()
{
   // intentionally left blank
}

void GpuProgram::bind()
{
   // set parameters for GPU program
   setGpuParameters();
}

void GpuProgram::disable()
{
   // intentionally left blank
}

bool GpuProgram::setGpuParameters()
{
   // get dynamic object that contains the input parameters for this GPU program
   const DynamicObject* pInputParams = mpGpuProgramDescriptor->getParameters();
   if (pInputParams == NULL)
   {
      return false;
   }

   DataVariant parameterValue;
   vector<string> parameterNames;

   // get necessary input parameters for GPU program
   pInputParams->getAttributeNames(parameterNames);

   // get the entry points for the input parameters into the GPU program
   vector<string>::const_iterator parameterNameIter = parameterNames.begin();
   while (parameterNameIter != parameterNames.end())
   {
      ++parameterNameIter;
   }

   return true;
}

void GpuProgram::setInput(int textureId)
{
   //empty
}
