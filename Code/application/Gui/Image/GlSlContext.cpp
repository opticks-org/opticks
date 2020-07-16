/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GlSlContext.h"

#include <QtOpenGL/QGLWidget>

using namespace std;
GlSlContext* GlSlContext::mpContext = NULL;

GlSlContext::~GlSlContext()
{
   // Clear the GL Shader programs map
   mGlPrograms.clear();
}

GlSlContext* GlSlContext::instance()
{
   if (mpContext == NULL)
   {
      mpContext = new GlSlContext;
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

GlSlContext::GlSlContext() :
   mbGpuSupported(true)
{
   const QGLContext* pContext = QGLContext::currentContext();
   bool validGlContext = ( (pContext != NULL) && (pContext->isValid()) );
   QGLWidget* pGlWidget = NULL;
   if (!validGlContext)
   {
      //create a valid open gl context, so that the GLSL
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

   GLint glewInitResult = glewInit();
   if (GLEW_OK != glewInitResult)
   {
      mbGpuSupported = false;
   }
   if (pGlWidget != NULL)
   {
      delete pGlWidget;
   }
}

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
GlProgram* GlSlContext::getGlProgram(GpuProgramDescriptor* pGpuProgramDescriptor)
{
   std::map<std::string, pair<GlProgram*, unsigned int> >::iterator iter = mGlPrograms.find(pGpuProgramDescriptor->getName());
   if (iter != mGlPrograms.end())
   {
      // Increment the number of references to this program
      ++(iter->second.second);
      // return the GlProgram
      return (iter->second.first);
   }

   // try to create it
   GlProgram* program = new GlProgram();
   program->compileProgram(pGpuProgramDescriptor);
   pair<GlProgram*, unsigned int> programPair(program, 1);
   mGlPrograms.insert(pair<string, pair<GlProgram*, unsigned int> >(pGpuProgramDescriptor->getName(), programPair));

   return program;
}

bool GlSlContext::destroyGlProgram(const string& programName)
{
   std::map<std::string, pair<GlProgram*, unsigned int> >::iterator iter = mGlPrograms.find(programName);
   if (iter != mGlPrograms.end())
   {
      // Only destroy the program if there will be no more references to it
      if (--(iter->second.second) == 0)
      {
         mGlPrograms.erase(iter);
         return true;
      }
   }
   return false;
}