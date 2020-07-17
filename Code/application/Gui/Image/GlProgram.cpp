/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <iostream>
#include <fstream>

#include "glCommon.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "ConfigurationSettings.h"
#include "MessageLogResource.h"

#include "GlProgram.h"
#include "GpuProgramDescriptor.h"

GlProgram::GlProgram()
{
}

GlProgram::~GlProgram()
{
}

/**
* Get the program object id.
* 
* @return the programObject
*/
int GlProgram::getProgramObject() {
   return mProgramObject;
}

/**
* Set the program object id.
* 
* @param id
*           the programObject to set
*/
void GlProgram::setProgramObject(int id) {
   mProgramObject = id;
}

/**
* Get the shader object id.
* 
* @return the shaderObject
*/
int GlProgram::getShaderObject() {
   return mShaderObject;
}

/**
* Set the shader object id.
* 
* @param id
*           the shaderObject to set
*/
void GlProgram::setShaderObject(int id) {
   mShaderObject = id;
}

/**
* Get the parameters for the program.
* 
* @return the mParameters
*/
const DynamicObject* GlProgram::getParameters() {
   return mpParameters;
}

/**
* Set the parameters for the program.
* 
* @param params
*           the mParameters to set
*/
void GlProgram::setParameters( DynamicObject* pParameters) {
   mpParameters = pParameters;
}

/**
* Compile this program based on the given descriptor.
* 
* @param descriptor
*           The descriptor that this program is based on.
* 
* @throws IOException
*            if the descriptor source is invalid.
*/
void GlProgram::compileProgram(GpuProgramDescriptor* descriptor)
{
   // get name of program file
   std::string programFileName = descriptor->getName();

   // get type of GPU program
   GpuProgramDescriptor::GpuProgramType programType = descriptor->getType();

   // load GPU program and get profile that was used to optimize
   // GPU code
   switch (programType) {
   case GpuProgramDescriptor::VERTEX_PROGRAM:
      // create program object
      mProgramObject = glCreateProgram();
      // create shader object (fragment shader) and attach to program
      mShaderObject = glCreateShader(GL_VERTEX_SHADER);
      break;
   case GpuProgramDescriptor::FRAGMENT_PROGRAM:
      // create program object
      mProgramObject = glCreateProgram();
      // create shader object (fragment shader) and attach to program
      mShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
      break;
   default:
      break;
   }
   // set source to shader object
   const Filename* pSupportFiles = ConfigurationSettings::getSettingSupportFilesPath();
   std::string programFile = pSupportFiles->getFullPathAndName() + SLASH + "glsl" + SLASH + programFileName;
   MessageResource msg1("Compiling shader program:" + programFile, "app", "{8CF2B51B-0A85-4938-BBBA-ACC930281667}");

   std::ifstream file(programFile);
   std::string str;
   // JO - Need to default o generic shader program if file is unavaiable
   std::string programSource;
   while (std::getline(file, str))
   {
      programSource += str;
      programSource.push_back('\n');
   }

   const char *cStr = programSource.c_str();
   glShaderSource(mShaderObject, 1, &cStr, NULL);
   glCompileShader(mShaderObject);
   // link program object together

   int result[2];
   glGetShaderiv(mShaderObject, GL_COMPILE_STATUS, result);
   if (result[0] == GL_FALSE) {
      MessageResource msg("Compiling shader program failed:" + programFile, "app", "{4A6C698A-5D90-4F35-BC51-2B3FE5A8A256}");
   }
   GLint maxLength = 0;
   glGetShaderiv(mShaderObject, GL_INFO_LOG_LENGTH, &maxLength);
   std::vector<char> infoLog;
   infoLog.resize(maxLength);
   glGetShaderInfoLog(mShaderObject, maxLength, nullptr, infoLog.data());
   std::string message(infoLog.begin(), infoLog.end());
   MessageResource msg("Compiling shader program log:" + message, "app", "{3A0BE916-D336-4B1D-8397-5F4292E16F90}");


   glAttachShader(mProgramObject, mShaderObject);
   glLinkProgram(mProgramObject);

   mpParameters = setupParameters(descriptor);
}

/**
* Retrieves the parameters of a specified gpu program.
* 
* GLSL doesn't allow us to query the parameter types. So we need our GIC
* file to tell us the types.
* 
* @return A Collection containing the parameters for the program. An empty
*         Collection is returned if the program does not take any
*         parameters.
*/
const DynamicObject *GlProgram::setupParameters(GpuProgramDescriptor* descriptor) {
   // get dynamic object that contains the input parameters for
   // this GPU program
  const DynamicObject* pParameters = descriptor->getParameters();

   if (pParameters == NULL) {
      return NULL;
   }

   // get necessary input parameters for GPU program
   return pParameters;
}
