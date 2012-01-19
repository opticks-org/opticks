/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CgContext.h"
#include "ConfigurationSettings.h"
#include "DataVariant.h"
#include "FrameBuffer.h"
#include "ImagePBuffer.h"

#include <QtOpenGL/QGLWidget>

using namespace std;
CgContext* CgContext::mpContext = NULL;

CgContext::~CgContext()
{
   // Destroying the Cg context destroys all Cg programs in it
   cgDestroyContext(mCgContext);

   // Clear the Cg programs map
   mCgProgramsLoaded.clear();
}

CgContext* CgContext::instance()
{
   if (mpContext == NULL)
   {
      mpContext = new CgContext;
   }

   if (mpContext != NULL)
   {
      if (mpContext->mbGpuSupported == false)
      {
         return NULL;
      }
   }

   return mpContext;
}

string CgContext::getCgProgramPath() const
{
   string programPath;
   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   if (pSupportFilesPath != NULL)
   {
      programPath = pSupportFilesPath->getFullPathAndName() + SLASH + "Cg" + SLASH;
   }

   return programPath;
}

bool CgContext::isCgVertexProgram(const string& programName)
{
   if (loadVertexProgram(programName) != 0)
   {
      CgContext::destroyCgProgram(programName);
      return true;
   }
   else
   {
      return false;
   }
}

CGprogram CgContext::loadVertexProgram(const string& programName)
{
   // Check if the program has already been loaded
   map<string, pair<CGprogram, unsigned int> >::iterator iter = mCgProgramsLoaded.find(programName);
   if (iter != mCgProgramsLoaded.end())
   {
      // Increment the number of references to this program
      ++(iter->second.second);
      return (iter->second.first);
   }

   // Get the latest vertex profile for graphics card
   CGprofile vertexProfile = getVertexProfile();

   // Set optimal options
   cgGLSetOptimalOptions(vertexProfile);

   // Set up the vertex program
   string programPath = getCgProgramPath();
   string programFile = programPath + SLASH + programName;

   CGprogram cgProgram = cgCreateProgramFromFile(mCgContext, CG_SOURCE, programFile.c_str(), vertexProfile, 0, 0);
   if (cgProgram != 0)
   {
      // Insert the Cg program into the map
      pair<CGprogram, unsigned int> programPair(cgProgram, 1);
      mCgProgramsLoaded.insert(pair<string, pair<CGprogram, unsigned int> >(programName, programPair));

      // Load the program in the Cg context
      cgGLLoadProgram(cgProgram);
   }

   return cgProgram;
}

bool CgContext::isCgFragmentProgram(const string& programName)
{
   if (loadFragmentProgram(programName) != 0)
   {
      CgContext::destroyCgProgram(programName);
      return true;
   }
   else
   {
      return false;
   }
}

CGprogram CgContext::loadFragmentProgram(const string& programName)
{
   // Check if the program has already been loaded
   map<string, pair<CGprogram, unsigned int> >::iterator iter = mCgProgramsLoaded.find(programName);
   if (iter != mCgProgramsLoaded.end())
   {
      // Increment the number of references to this program
      ++(iter->second.second);
      return (iter->second.first);
   }

   // Get the latest fragment profile for graphics card
   CGprofile fragmentProfile = getFragmentProfile();

   // Set optimal options
   cgGLSetOptimalOptions(fragmentProfile);

   // Set up the fragment program
   string programPath = getCgProgramPath();
   string programFile = programPath + SLASH + programName;

   CGprogram cgProgram = cgCreateProgramFromFile(mCgContext, CG_SOURCE, programFile.c_str(), fragmentProfile, 0, 0);
   if (cgProgram == 0)
   {
      mLastCgErrorMessage = cgGetErrorString(cgGetError());
   }
   else
   {
      // Insert the Cg program into the map
      pair<CGprogram, unsigned int> programPair(cgProgram, 1);
      mCgProgramsLoaded.insert(pair<string, pair<CGprogram, unsigned int> >(programName, programPair));

      // Load the program in the Cg context
      cgGLLoadProgram(cgProgram);
   }

   return cgProgram;
}

vector<CGparameter> CgContext::getParameters(CGprogram cgProgram) const
{
   vector<CGparameter> parameters;

   CGparameter parameter = cgGetFirstParameter(cgProgram, CG_PROGRAM);
   while (parameter != 0)
   {
      parameters.push_back(parameter);
      parameter = cgGetNextParameter(parameter);
   }

   return parameters;
}

void CgContext::destroyCgProgram(const string& programName)
{
   map<string, pair<CGprogram, unsigned int> >::iterator iter = mCgProgramsLoaded.find(programName);
   if (iter != mCgProgramsLoaded.end())
   {
      // Only destroy the program if there will be no more references to it
      if (--(iter->second.second) == 0)
      {
         cgDestroyProgram(iter->second.first);
         mCgProgramsLoaded.erase(iter);
      }
   }
}

CGprofile CgContext::getVertexProfile() const
{
   CGprofile vertexProfile = CG_PROFILE_UNKNOWN;
   if (cgGLIsProfileSupported(cgGLGetLatestProfile(CG_GL_VERTEX)))
   {
      vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
   }
   else if (cgGLIsProfileSupported(CG_PROFILE_VP30))
   {
      vertexProfile = CG_PROFILE_VP30;
   }

   return vertexProfile;
}

CGprofile CgContext::getFragmentProfile() const
{
   CGprofile fragmentProfile = CG_PROFILE_UNKNOWN;
   if (cgGLIsProfileSupported(cgGLGetLatestProfile(CG_GL_FRAGMENT)))
   {
      fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
   }
   else if (cgGLIsProfileSupported(CG_PROFILE_FP30))
   {
      fragmentProfile = CG_PROFILE_FP30;
   }

   return fragmentProfile;
}

string CgContext::getLastCgErrorMessage()
{
   string rval = mLastCgErrorMessage;
   mLastCgErrorMessage.clear();
   return rval;
}

CgContext::CgContext() :
   mbGpuSupported(false),
   mCgContext(0),
   mVertexIdentityProfile(CG_PROFILE_UNKNOWN),
   mFragmentIdentityProfile(CG_PROFILE_UNKNOWN)
{
   const QGLContext* pContext = QGLContext::currentContext();
   bool validGlContext = ( (pContext != NULL) && (pContext->isValid()) );
   QGLWidget* pGlWidget = NULL;
   if (!validGlContext)
   {
      //create a valid open gl context, so that the Cg
      //context can be initialized correctly
      class tmpGL : public QGLWidget {
      public:
         tmpGL(QWidget *parent) : QGLWidget(parent) {}
      protected:
         virtual void initializeGL() {}
         virtual void resizeGL(int w, int h) {}
         virtual void paintGL() {}
      private:
         tmpGL(const tmpGL& rhs) {}
         tmpGL& operator=(const tmpGL& rhs) { return *this; }
      };
      pGlWidget = new tmpGL(NULL);
      pGlWidget->makeCurrent();
   }

   glewInit();
   mCgContext = cgCreateContext();
   if (mCgContext)
   {
      bool bVertexCgProgramsSupported = true;
      bool bFragmentCgProgramsSupported = true;

      // Get the latest vertex profile or choose the CG_PROFILE_VP30 profile
      if (cgGLIsProfileSupported(cgGLGetLatestProfile(CG_GL_VERTEX)))
      {
         mVertexIdentityProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      }
      else if (cgGLIsProfileSupported(CG_PROFILE_VP30))
      {
         mVertexIdentityProfile = CG_PROFILE_VP30;
      }
      else
      {
         bVertexCgProgramsSupported = false;
      }

      // Set the optimal options
      cgGLSetOptimalOptions(mVertexIdentityProfile);

      // Get the latest fragment profile or choose the CG_PROFILE_FP30 profile
      if (cgGLIsProfileSupported(cgGLGetLatestProfile(CG_GL_FRAGMENT)))
      {
         mFragmentIdentityProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
      }
      else if (cgGLIsProfileSupported(CG_PROFILE_FP30))
      {
         mFragmentIdentityProfile = CG_PROFILE_FP30;
      }
      else
      {
         bFragmentCgProgramsSupported = false;
      }

      // Set the optimal options
      cgGLSetOptimalOptions(mFragmentIdentityProfile);

      if ((bVertexCgProgramsSupported == true) && (bFragmentCgProgramsSupported == true))
      {
         mbGpuSupported = true;
      }
      else
      {
         cgDestroyContext(mCgContext);
         mCgContext = 0;
      }
   }

   if (pGlWidget != NULL)
   {
      delete pGlWidget;
   }
}
