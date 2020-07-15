#ifndef GlShaderGpuProgram_H
#define GlShaderGpuProgram_H

#include "GpuProgram.h"
#include "GpuProgramDescriptor.h"
#include "Image.h"

#include <string>
#include <map>
#include <vector>

class ColorBuffer;
class GpuImage;
class ImageFilter;

class GlShaderGpuProgram : public GpuProgram
{

public:

   /**
   * Default Constructor.
   */
   GlShaderGpuProgram();

   /**
   * Constructor to the GlShaderGpuProgram class.
   * 
   * @param pGpuDescriptor
   *           GPU Program descriptor.
   * 
   */
   GlShaderGpuProgram(GpuProgramDescriptor* pGpuDescriptor);

   ~GlShaderGpuProgram();

   /**
   * Set the program descriptor for this program.
   * 
   * @param pGpuDescriptor
   *           The new descriptor.
   */
   void setGpuDescriptor(GpuProgramDescriptor* pGpuDescriptor);

   /**
   * Initialize the program. This must be called in the EventDispatchThread.
   * 
   * @throws IOException
   *            If the descriptor source is invalid.
   */
   void initialize();

   /**
   * Binds the GPU program to the specific processor so that when the data is
   * rendered the program will process it.
   * 
   * @see setGpuParameters()
   */
   void bind();

   /**
   * Disables the specific GPU program.
   * 
   * @param gl
   *           The GL context to draw to.
   * 
   * @see bind()
   */
   void disable();

   /**
   * Set the input texture id to enable.
   * 
   * @param textureId
   *           id The id of the texture to enable.
   */
   void setInput(int textureId);

   /**
   * Returns the GPU program descriptor from which it was created. The Gpu
   * program descriptor is used to set the parameters for the GPU program.
   * Every time the GPU program gets bound the parameters are set using the GPU
   * program descriptor.
   * 
   * @return The program descriptor for this program.
   */
   GpuProgramDescriptor* getProgramDescriptor();

   /**
   * Sets the necessary GPU input parameters for the program.
   * 
   * @return true if the operation completes normally.
   * 
   * @see bind()
   */
   bool setGpuParameters();

   /**
   * Sets the parameters for drawing the image using the GPU. This method sets
   * the stretch values, input texture, and transparency for the default gray
   * scale, RGB, and color map CG programs.
   * 
   * @param imageInfo
   *           The ImageData for the image being processed.
   * @param alpha
   *           The alpha value
   * @param mColormapTexture
   *           The id of the texture
   * 
   */
   void setParameterValues( Image::ImageData imageInfo, float alpha, int mColormapTexture);

   /**
   * Get the texture unit id.
   * 
   * @return the textureUnit
   */
   int getTextureUnit();

   /**
   * Set the texture unit id
   * 
   * @param unit
   *           the textureUnit to set
   */
   void setTextureUnit( int unit);

   /**
   * Set the shader object id.
   * 
   * @param shader
   *           the shaderObject to set
   */
   void setShaderObject( int shader);

   /**
   * Return the compiled state of the program.
   * 
   * @return true if the program is compiled, else return false.
   */
   bool isCompiled();

   void dispose();

   int getProgramObjectId();

   /**
   * Helper method that counts a set of existing GPU objects and how many of them are currently on the GPU.
   * @param gl
   */
protected:
   /**
   * If necessary, converts an signed stretch value to an unsigned stretch
   * value.
   * 
   * @param imageInfo
   * @param rawValue
   * @param dataType
   * @return
   */
   float getTextureStretchValue(Image::ImageData& imageInfo, float rawValue, EncodingType dataType);


   /**
   * Image to be displayed. This is the primary input parameter to most of the
   * CG programs for color map, RGB, and gray scale display
   */
   int mInputTexture; // Same as Cg (CGparameter)

   int mTextureUnit;

   /**
   * Id of the compiled shader.
   */
   int mShaderObject;

   /**
   * Id of the compiled program.
   */
   int mProgramObject; // Same as Cg CGprogram (mProgramId)

   /**
   * has the program for this shader been compiled
   */
   bool mCompiled;

   /**
   * Flag indicating if this object has been disposed
   */
   bool mDisposed;


};

#endif
