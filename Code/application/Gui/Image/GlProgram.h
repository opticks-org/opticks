#ifndef GlProgram_H
#define GlProgram_H

#include <string>
#include <vector>

#include "GpuProgramDescriptor.h"

/**
* This is a compiled program in either GLSL or CG.
* 
* @author smangan
* 
*/
class GlProgram 
{

public:

   /**
   * Default Constructor.
   */
   GlProgram();

   ~GlProgram();
   /**
   * Get the program object id.
   * 
   * @return the programObject
   */
   int getProgramObject();

   /**
   * Set the program object id.
   * 
   * @param id
   *           the programObject to set
   */
   void setProgramObject(int id);

   /**
   * Get the shader object id.
   * 
   * @return the shaderObject
   */
   int getShaderObject();

   /**
   * Set the shader object id.
   * 
   * @param id
   *           the shaderObject to set
   */
   void setShaderObject(int id);

   /**
   * Get the parameters for the program.
   * 
   * @return the mParameters
   */
   const DynamicObject *getParameters();

   /**
   * Set the parameters for the program.
   * 
   * @param params
   *           the mParameters to set
   */
   void setParameters( DynamicObject *pParameters);

   /**
   * Compile this program based on the given descriptor.
   * 
   * @param descriptor
   *           The descriptor that this program is based on.
   * 
   * @throws IOException
   *            if the descriptor source is invalid.
   */
   void compileProgram(GpuProgramDescriptor* descriptor);

private:

   /**
   * Retrieves the parameters of a specified gpu program.
   * 
   * GLSL doesn't allow us to query the parameter types. So we need our GS
   * file to tell us the types.
   * 
   * @return A Collection containing the parameters for the program. An empty
   *         Collection is returned if the program does not take any
   *         parameters.
   */
   const DynamicObject *setupParameters(GpuProgramDescriptor* descriptor);

   int mProgramObject; // Same as glsl CGprogram (mProgramId)

   int mShaderObject;

   const DynamicObject* mpParameters;

};

#endif