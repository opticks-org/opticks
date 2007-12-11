/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
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

#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif

using namespace std;

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

#if defined(CG_SUPPORTED)
   REQUIRE(CgContext::instance() != NULL);

   // load GPU program and get profile that was used to optimize GPU code
   switch(programType)
   {
   case GpuProgramDescriptor::VERTEX_PROGRAM:
      mProgramId = CgContext::instance()->loadVertexProgram(programFileName);
      break;
   case GpuProgramDescriptor::FRAGMENT_PROGRAM:
      mProgramId = CgContext::instance()->loadFragmentProgram(programFileName);
      break;
   default:
      break;
   }

   REQUIRE(mProgramId != 0);
   mProfile = cgGetProgramProfile(mProgramId);
#endif
}

GpuProgram::~GpuProgram()
{
   // unload GPU program to free up memory
#if defined(CG_SUPPORTED)
   if (CgContext::instance() != NULL)
   {
      CgContext::instance()->destroyCgProgram(mpGpuProgramDescriptor->getName());
   }
#endif
}

void GpuProgram::bind()
{
#if defined(CG_SUPPORTED)
   cgGLBindProgram(mProgramId);
   cgGLEnableProfile(mProfile);
#endif

   // set parameters for GPU program
   setGpuParameters();
}

void GpuProgram::disable()
{
#if defined(CG_SUPPORTED)
   // disabling the profile disables the program
   // running on the corresponding GPU processor
   cgGLDisableProfile(mProfile);
#endif
}

bool GpuProgram::setGpuParameters()
{
   // get dynamic object that contains the input parameters for this GPU program
   const DynamicObject *pInputParams = mpGpuProgramDescriptor->getParameters();
   if (pInputParams == NULL)
   {
      return false;
   }

   DataVariant parameterValue;
   vector<string> parameterNames;

   // get necessary input parameters for GPU program
   pInputParams->getAttributeNames(parameterNames);

#if defined(CG_SUPPORTED)
   CGparameter inputParameter = 0;
   CGtype cgParameterType;
#endif

   // get the entry points for the input parameters into the GPU program
   vector<string>::const_iterator parameterNameIter = parameterNames.begin();
   while (parameterNameIter != parameterNames.end())
   {
#if defined(CG_SUPPORTED)
      // get entry point for parameter into GPU program
      inputParameter = cgGetNamedParameter(mProgramId, parameterNameIter->c_str());

      // check to make sure parameter entry point is valid
      if (inputParameter != 0)
      {
         // get the value for the parameter
         parameterValue = pInputParams->getAttribute(*parameterNameIter);
         if (parameterValue.isValid())
         {
            // get the type of the parameter
            cgParameterType = cgGetParameterNamedType(inputParameter);
            // set the parameter's value
            switch(cgParameterType)
            {
            case CG_SAMPLER1D:   // fall through to next case statement
            case CG_SAMPLER2D:   // fall through to next case statement
            case CG_SAMPLER3D:   // fall through to next case statement
            case CG_SAMPLERRECT: // fall through to next case statement
               {
                  // get the name of the Cg input parameter for the program
                  string cgParameterName = cgGetParameterName(inputParameter);

                  // find the color buffer associated with the Cg input parameter
                  map<string, ColorBuffer*>::const_iterator mapIter = 
                     mColorBuffers.find(cgParameterName);
                  if (mapIter != mColorBuffers.end())
                  {
                     // get the color buffer's texture id
                     GLuint textureParameter = mapIter->second->getTextureObjectId();

                     // set the texture id for the Cg program input parameter
                     cgGLSetTextureParameter(inputParameter, textureParameter);
                     cgGLEnableTextureParameter(inputParameter);
                  }
               }
               break;
            case CG_FLOAT:
               {
                  float floatParameter = 0.0;
                  parameterValue.getValue(floatParameter);

                  cgGLSetParameter1f(inputParameter, floatParameter);
               }
               break;
            default:
               break;
            }
         }
      }
#endif
      ++parameterNameIter;
   }

   return true;
}
