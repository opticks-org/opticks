/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GLTEXTURERESOURCE_H
#define GLTEXTURERESOURCE_H

#include "AppVerify.h"
#include "Resource.h"

#include <QtOpenGL/QGLContext>

/**
 * A class for managing OpenGL textures.
 * When created, the object records the current OpenGL context and allocates textures.
 * When destroyed, it makes the recorded context current, deletes the textures, and restores the context.
 */
class GlTextureObject
{
public:
   /**
   * This is an implementation detail of the GlTextureObject class. It is used 
   * for storing the parameters required by GlTextureResource.
   */
   class Args
   {
   public:
      Args(GLsizei count) :
         mpContext(count <= 0 ? NULL : const_cast<QGLContext*>(QGLContext::currentContext())),
         mCount(count < 0 ? 0 : count)
      {}

      QGLContext* mpContext;
      GLsizei mCount;
   };

   /**
    * Allocates one or more texture IDs.
    *
    * @param  args
    *         The arguments for obtaining the resource. Should be of type GlTextureObject::Args.
    *
    * @return A pointer to the texture IDs or \c NULL.
    */
   GLuint* obtainResource(const Args& args) const
   {
      if (args.mCount <= 0)
      {
         return NULL;
      }

      GLuint* pTextures = new GLuint[args.mCount];
      glGenTextures(args.mCount, pTextures);
      return pTextures;
   }

   /**
    * Releases one or more texture IDs.
    *
    * @param  args
    *         The arguments for releasing the resource. Should be of type GlTextureObject::Args.
    *
    * @param  pTextures
    *         A pointer to the texture IDs to be freed.
    */
   void releaseResource(const Args& args, GLuint* pTextures) const
   {
      if (pTextures != NULL)
      {
         GlContextSave contextSave(args.mpContext);
         glDeleteTextures(args.mCount, pTextures);
         delete[] pTextures;
      }
   }
};

/**
 * This is a %Resource class that allocates and deletes OpenGL textures.
 *
 * This class has a conversion operator to allow a %GlTextureResource object to be
 * used wherever a GLuint* would normally be used.
*/
class GlTextureResource : public Resource<GLuint, GlTextureObject>
{
public:
   /**
   * Constructs a Resource object that wraps OpenGL textures.
   *
   * @param   count
   *          The number of textures to create.
   */
   GlTextureResource(GLsizei count) :
      Resource<GLuint, GlTextureObject>(GlTextureObject::Args(count))
   {}

   /**
    * Gets the context which was active when the object was created.
    *
    * @return The context.
    *
    * @see GlContextSave
    */
   inline QGLContext* getContext() const
   {
      return getArgs().mpContext;
   }

   /**
    * Gets the number of textures.
    *
    * @return The number of textures.
    */
   inline GLsizei getCount() const
   {
      return getArgs().mCount;
   }

   /**
    * Convenience operator which allows this object to be used wherever a const GLuint* would normally be used.
    *
    * @return A pointer to all textures in the list or \c NULL if no textures exist.
    */
   operator const GLuint*() const
   {
      return get();
   }

   /**
    * Convenience operator which allows this object to be used wherever a GLuint would normally be used.
    *
    * @return The first texture in the list or 0 if no textures exist.
    */
   operator GLuint() const
   {
      return getCount() <= 0 ? 0 : *get();
   }
};

#endif
