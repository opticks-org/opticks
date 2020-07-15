#ifndef ColormapGpuProgram_H
#define ColormapGpuProgram_H

#include <string>

#include "GlShaderGpuProgram.h"

/**
 * Program that represents a grayscale rendering program.
 * 
 * @author smangan
 * 
 */
class ColormapGpuProgram: public GlShaderGpuProgram 
{
public:
   
   /**
    * Constructor
    */
   ColormapGpuProgram();

   /**
    * Initialize the program. This must be called in the EventDispatchThread.
    * 
    * @throws IOException
    *            If the descriptor source is invalid.
    */
   void initialize();

private:

};

#endif