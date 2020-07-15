/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef GlSlContext_H
#define GlSlContext_H

#include <map>
#include <string>
#include <vector>

#include "glCommon.h"
#include "GlShaderGpuProgram.h"
#include "GpuProgramDescriptor.h"
#include "GlProgram.h"


/**
 *  Manages the GlSl context, profiles, and programs.
 *
 *  This class initializes a GL context.
 */
class GlSlContext
{
public:
   /**
    *  Destroys the Cg context along with its profiles and programs.
    */
   ~GlSlContext();

   /**
    *  Returns a pointer to the only instance of the GlSlContext class.
    *
    *  This method creates and initializes the GlSlContext class if it has not already been created.
    *
    *  @return  A pointer to the Cg context.
    */
   static GlSlContext* instance();

   /**
    * Get a GlProgram based on its descriptor. This must must be called inside
    * the display loop to ensure the GlProgram may be compiled if necessary.
    * 
    * @param descriptor
    *           The descriptor of the program to retrieve.
    * 
    * @return The GlProgram for the given name, or null if no program exists.
    * 
    * @throws IOException
    *            if the descriptor source is invalid.
    */
   GlProgram* getGlProgram( GpuProgramDescriptor* mpGpuProgramDescriptor);

   /**
    *  Unloads a Cg program from the context.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    */
   bool destroyGlProgram(const std::string& programName);

protected:
   /**
    *  Creates and initializes the Cg context.
    */
   GlSlContext();

private:
   static GlSlContext* mpContext;
   
   std::map<std::string, std::pair<GlProgram*, unsigned int> > mGlPrograms;
   bool mbGpuSupported;                   // Used to check whether or not the system has the necessary OpenGL
                                          // extensions and that the graphics card can support the Cg programs
};

#endif
