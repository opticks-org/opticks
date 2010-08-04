/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GlContextSave.h"
#include "glCommon.h"

#include <QtOpenGL/QGLContext>
#include <QtOpenGL/QGLWidget>

GlContextSave::GlContextSave() :
   mpCurrentContext(const_cast<QGLContext*>(QGLContext::currentContext()))
{}

GlContextSave::GlContextSave(QGLWidget* pWidget) :
   mpCurrentContext(const_cast<QGLContext*>(QGLContext::currentContext()))
{
   if (pWidget != NULL && pWidget->context() != mpCurrentContext)
   {
      pWidget->makeCurrent();
   }
}

GlContextSave::GlContextSave(QGLContext* pContext) :
   mpCurrentContext(const_cast<QGLContext*>(QGLContext::currentContext()))
{
   if (pContext != NULL && pContext != mpCurrentContext)
   {
      pContext->makeCurrent();
   }
}

GlContextSave::~GlContextSave()
{
   if (mpCurrentContext != NULL)
   {
      mpCurrentContext->makeCurrent();
   }
}
