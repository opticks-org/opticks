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

class QGLContext;
class QGLWidget;

/**
 * A class for saving and restoring the current OpenGL context.
 * When created, the GlContextSave object records the current OpenGL context.
 * When destroyed, it makes the recorded context current again, regardless of
 * changes that have occurred while the GlContextSave object existed.
 */
class GlContextSave
{
public:
   /**
    * Records the current OpenGL context.
    */
   GlContextSave();

   /**
    * Records the current OpenGL context and makes the specified context current.
    *
    * @param   pWidget
    *          The widget containing the context to make current.
    *          If \c NULL is specified, the current context will not be changed.
    */
   GlContextSave(QGLWidget* pWidget);

   /**
    * Records the current OpenGL context and makes the specified context current.
    *
    * @param   pContext
    *          The context to make current. If \c NULL is specified, the current context will not be changed.
    */
   GlContextSave(QGLContext* pContext);

   /**
    * Restores the current OpenGL context to the one that the constructor
    * recorded.
    */
   ~GlContextSave();

private:
   GlContextSave(const GlContextSave& rhs);
   GlContextSave& operator=(const GlContextSave& rhs);

   QGLContext* const mpCurrentContext;
};

#endif
