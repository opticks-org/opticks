/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPURESOURCEMANAGER_H
#define GPURESOURCEMANAGER_H

#include "glCommon.h"
#include "Service.h"

#include <map>
#include <vector>

class ImageBuffer;
class PixelBufferObject;

/**
 *  Manages the Gpu resources such as texture memory and off-screen buffers.
 *
 *  This class manages texture memory and off-screen buffers. The class provides
 *  a way to check and see if all the current texture objects are resident on the
 *  graphics card. If one of the texture objects is not resident, the user will
 *  receive a warning saying "Gpu resources are running low, please close some of
 *  the current datasets to free up memory".
 */
class GpuResourceManager
{
public:
   /**
    *  Returns a pointer to the PixelBufferObject.
    *
    *  This method creates a PixelBufferObject from the specified parameters.
    *  The caller takes ownership of the PixelBufferObject.
    *
    *    @param   numBytes
    *             The size of the pixel buffer object.
    *
    *    @param   accessMode
    *             The i/o access to the pixel buffer object.
    *
    *    @return  A pointer to a PixelBufferObject.
    *
    *    @see     PixelBufferObject
    */
   PixelBufferObject *getPixelBufferObject(int numBytes, GLenum accessMode);

   /**
    *  Allocates texture memory on the graphics card and returns the texture object id to access the memory.
    *  If the texture memory cannot be allocated, a zero is returned as the texture object id. Two reasons 
    *  why texture memory cannot be allocated are the following:
    *
    *    1) The OpenGL call to glGenTextures failed.
    *    2) Not enough available texture memory left to keep all current textures resident on the GPU
    *
    *    @param   textureTarget
    *             The target of the texture to create (GL_TEXTURE_2D, GL_TEXTURE_3D, etc.).
    *
    *    @param   internalFormat
    *             The internal format of the texture to create (GL_FLOAT_R32_NV, GL_RGB16, etc.).
    *
    *    @param   width
    *             The width of the texture to create.
    *
    *    @param   height
    *             The height of the texture to create.
    *
    *    @param   textureFormat
    *             The format of the texture to create (GL_RGB, GL_RED, etc.).
    *
    *    @param   dataType
    *             The type of the data used to create the texture (GL_UNSIGNED_SHORT, GL_FLOAT, etc.).
    *
    *    @return  GLuint
    *             The OpenGL texture object id.
    */
   GLuint allocateTexture(GLenum textureTarget, GLint internalFormat, GLsizei width, GLsizei height, 
                          GLenum textureFormat, GLenum dataType);

   /**
    *  Deallocates the specified texture object.
    *
    *  This method searches through the vector of texture objects and
    *  deallocates the specified one.
    *
    *  @return  void
    */
   void deallocateTexture(GLuint textureId);

   /**
    *  Returns a pointer to an ImageBuffer object that can contain an ImageColorBuffer of the specified size.
    *
    *  This method returns an ImageBuffer object that has available color buffer attachments for an ImageColorBuffer
    *  of the specified size.
    *
    *  @return  A pointer to an ImageBuffer object. The caller takes ownership of the ImageBuffer*
    */
   ImageBuffer *allocateImageBuffer();

   /**
    *  Returns scaling factor used by video driver.
    *
    *  This method returns the scaling factor used by the video driver when writing values to the frame buffer.
    *  The value was 3.0 for ForceWare versions 94.22 and earlier. The value is 2.0 for more current versions (so far). 
    *
    *  @param   textureFormat
    *           The texture format, including alpha, for which the scaling
    *           factor is to be determined.
    *
    *  @return  The value of the scaling factor.
    */
   float getGpuScalingFactor(GLenum textureFormat);

private:
   GpuResourceManager();
   ~GpuResourceManager();

   friend class DesktopServicesImp;
   std::vector<GLuint> mTextures;
   std::map<GLenum,float> mGpuScalingFactors;
   bool determineScalingFactor(float& scalingFactor, GLenum textureFormat);
};

template<>
GpuResourceManager* Service<GpuResourceManager>::get() const;

#endif
