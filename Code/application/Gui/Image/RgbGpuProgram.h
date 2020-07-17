/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
