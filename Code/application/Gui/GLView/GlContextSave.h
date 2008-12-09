/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GLCONTEXTSAVE_H
#define GLCONTEXTSAVE_H

#include "glCommon.h"

#include <QtOpenGL/QGLContext>

class GlContextSave
{
public:
   GlContextSave() :
      mpCurrentContext(const_cast<QGLContext*>(QGLContext::currentContext()))
   {
   }

   ~GlContextSave()
   {
      if (mpCurrentContext != NULL)
      {
         mpCurrentContext->makeCurrent();
      }
   }

private:
   QGLContext* mpCurrentContext;
};

#endif
