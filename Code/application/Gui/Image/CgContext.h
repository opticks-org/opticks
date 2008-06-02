/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef CGCONTEXT_H
#define CGCONTEXT_H

#include "glCommon.h"

#include <map>
#include <string>
#include <vector>

class ImageBuffer;

/**
 *  Manages the Cg context, profiles, and programs.
 *
 *  This class manages the Cg context and its programs and profiles.  It is able to compile and load
 *  specified Cg vertex and fragment programs.  It also determines the best Cg profile to use for
 *  those programs.  This class also gets specified OpenGL extensions using the OpenGL Extension
 *  Wrangler Library.
 */
class CgContext
{
public:
   /**
    *  Destroys the Cg context along with its profiles and programs.
    */
   ~CgContext();

   /**
    *  Returns a pointer to the only instance of the CgContext class.
    *
    *  This method creates and initializes the CgContext class if it has not already been created.
    *
    *  @return  A pointer to the Cg context.
    */
   static CgContext* instance();

   /**
    *  Retrieves the path of the Cg programs directory.
    *
    *  @return  The path of the Cg programs
    */
   std::string getCgProgramPath() const;

   /**
    *  Identifies whether or not the file is a Cg vertex program.
    *
    *  This method tries loads a vertex program into the Cg context to identify
    *  whether or not it is a Cg vertex program. If it is a Cg vertex program, the
    *  program is then removed from the Cg context and destroyed.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    *
    *  @return  True, if the file is a Cg vertex program. False otherwise.
    *
    *  @see     isCgFragmentProgram()
    */
    bool isCgVertexProgram(const std::string& programName);

   /**
    *  Compiles and loads a vertex program into the Cg context.
    *
    *  This method loads a vertex program into the Cg context.  The program is
    *  reference counted, so objects calling this method must call destroyCgProgram()
    *  for each call to this method.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    *
    *  @return  The Cg program, whose value is the entry parameter into the program.
    *           Zero is returned if the program could not be loaded.
    *
    *  @see     loadFragmentProgram()
    */
   CGprogram loadVertexProgram(const std::string& programName);

   /**
    *  Identifies whether or not the file is a Cg fragment program.
    *
    *  This method tries loads a fragment program into the Cg context to identify
    *  whether or not it is a Cg fragment program. If it is a Cg fragment program, the
    *  program is then removed from the Cg context and destroyed.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    *
    *  @return  True, if the file is a Cg fragment program. False otherwise.
    *
    *  @see     isCgVertexProgram()
    */
    bool isCgFragmentProgram(const std::string& programName);

   /**
    *  Compiles and loads a fragment program into the Cg context.
    *
    *  This method loads a fragment program into the Cg context.  The program is
    *  reference counted, so objects calling this method must call destroyCgProgram()
    *  for each call to this method.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    *
    *  @return  The Cg program, whose value is the entry parameter into the program.
    *           Zero is returned if the program could not be loaded.
    *
    *  @see     loadVertexProgram()
    */
   CGprogram loadFragmentProgram(const std::string& programName);

   /**
    *  Retrieves the parameters of a specified Cg program.
    *
    *  This method gets the parameters of a specified Cg program that has been loaded
    *  into the Cg context.
    *
    *  @param   cgProgram
    *           The Cg program for which to get its parameters.
    *
    *  @return  A vector containing the parameters for the program.  An empty vector is
    *           returned if the Cg program does not take any parameters.
    */
   std::vector<CGparameter> getParameters(CGprogram cgProgram) const;

   /**
    *  Unloads a Cg program from the context.
    *
    *  @param   programName
    *           The name of the Cg file containing the program (e.g. "MyCgProgram.cg").
    */
   void destroyCgProgram(const std::string& programName);

   /**
    *  Returns the latest vertex profile that the graphics card supports.
    *
    *  @return  The vertex profile.
    *
    *  @see     getFragmentProfile()
    */
   CGprofile getVertexProfile() const;

   /**
    *  Returns the latest fragment profile that the graphics card supports.
    *
    *  @return  The fragment profile.
    *
    *  @see     getVertexProfile()
    */
   CGprofile getFragmentProfile() const;

   /**
    *  Returns the most recent Cg error string.
    *
    *  @return A string with the most recent Cg error message or
    *          an empty string if there has not been a Cg error
    *          since the last invocation of this method.
    */
   std::string getLastCgErrorMessage();

protected:
   /**
    *  Creates and initializes the Cg context.
    */
   CgContext();

private:
   static CgContext* mpContext;

   bool mbGpuSupported;                   // Used to check whether or not the system has the necessary OpenGL
                                          // extensions and that the graphics card can support the Cg programs
   CGcontext mCgContext;                  // Cg context
   CGprofile mVertexIdentityProfile;      // Latest vertex profile
   CGprofile mFragmentIdentityProfile;    // Latest fragment profile

   std::map<std::string, std::pair<CGprogram, unsigned int> > mCgProgramsLoaded;
   std::string mImageBufferType;
   std::string mLastCgErrorMessage;
};

#endif
