#ifndef GrayscaleGpuProgram_H
#define GrayscaleGpuProgram_H

#include <string>

#include "GlShaderGpuProgram.h"

/**
 * Program that represents a grayscale rendering program.
 * 
 * @author smangan
 * 
 */
class GrayscaleGpuProgram: public GlShaderGpuProgram 
{
public:
   
   /**
    * Constructor
    */
   GrayscaleGpuProgram();

   /**
    * Initialize the program. This must be called in the EventDispatchThread.
    * 
    * @throws IOException
    *            If the descriptor source is invalid.
    */
   void initialize();

private:
   std::string GLSL_FILE_NAME;

};

#endif