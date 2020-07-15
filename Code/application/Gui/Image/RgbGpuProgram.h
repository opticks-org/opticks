#ifndef RgbGpuProgram_H
#define RgbGpuProgram_H

#include <string>

#include "GlShaderGpuProgram.h"

/**
 * Program that represents a grayscale rendering program.
 * 
 * @author smangan
 * 
 */
class RgbGpuProgram: public GlShaderGpuProgram 
{
public:
   
   /**
    * Constructor
    */
   RgbGpuProgram();

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