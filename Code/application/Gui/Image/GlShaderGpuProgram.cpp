/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GlShaderGpuProgram.h"
#include "GpuImage.h"
#include "GlProgram.h"
#include "MessageLogResource.h"
#include "ColorBuffer.h"
#include "ImageFilter.h"
#include "DynamicObject.h"
#include "GlSlContext.h"


/**
* Default Constructor.
*/
GlShaderGpuProgram::GlShaderGpuProgram()
{
   mInputTexture = 0;
   mTextureUnit = 1;
   mShaderObject = 0;
   mProgramObject = 0;
   mCompiled = false;
   mDisposed = false;
}

/**
* Constructor to the GlShaderGpuProgram class.
* 
* @param pGpuDescriptor
*           GPU Program descriptor.
* 
*/
GlShaderGpuProgram::GlShaderGpuProgram(GpuProgramDescriptor* pGpuDescriptor)
{
   setGpuDescriptor(pGpuDescriptor); 

}

GlShaderGpuProgram::~GlShaderGpuProgram()
{
   // destroy the shader program
   dispose();
}

/**
* Set the program descriptor for this program.
* 
* @param pGpuDescriptor
*           The new descriptor.
*/
void GlShaderGpuProgram::setGpuDescriptor(GpuProgramDescriptor* pGpuDescriptor)
{
   mpGpuProgramDescriptor = pGpuDescriptor;
}

/**
* Initialize the program. This must be called in the EventDispatchThread.
* 
* @throws IOException
*            If the descriptor source is invalid.
*/
void GlShaderGpuProgram::initialize()
{
   GlProgram* glProgram = GlSlContext::instance()->getGlProgram(mpGpuProgramDescriptor);

   // TODO finish this implementation to cache and reuse GlProgram instances.
   mProgramObject = glProgram->getProgramObject();
   setShaderObject(glProgram->getShaderObject());
   mCompiled = true;
}

/**
* Binds the GPU program to the specific processor so that when the data is
* rendered the program will process it.
* 
* @see setGpuParameters()
*/
void GlShaderGpuProgram::bind()
{
   glUseProgram(mProgramObject);

   // set parameters for GPU program
   setGpuParameters();
}

/**
* Disables the specific GPU program.
* 
* @param gl
*           The GL context to draw to.
* 
* @see bind()
*/
void GlShaderGpuProgram::disable()
{
   // disabling the profile disables the program
   // running on the corresponding GPU processor
   glUseProgram(0);
}

/**
* Set the input texture id to enable.
* 
* @param textureId
*           id The id of the texture to enable.
*/
void GlShaderGpuProgram::setInput(int textureId)
{

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureId);
   glUniform1i(mInputTexture, 1);
   glActiveTexture(GL_TEXTURE0);
}

/**
* Returns the GPU program descriptor from which it was created. The Gpu
* program descriptor is used to set the parameters for the GPU program.
* Every time the GPU program gets bound the parameters are set using the GPU
* program descriptor.
* 
* @return The program descriptor for this program.
*/
GpuProgramDescriptor* GlShaderGpuProgram::getProgramDescriptor()
{
   return mpGpuProgramDescriptor;
}


/**
* Sets the necessary GPU input parameters for the program.
* 
* @return true if the operation completes normally.
* 
* @see bind()
*/
bool GlShaderGpuProgram::setGpuParameters()
{
   // get dynamic object that contains the input parameters for this GPU program
   const DynamicObject* pInputParams = mpGpuProgramDescriptor->getParameters();

   if ((pInputParams == NULL) || (mColorBuffers.empty())) {
      return true;
   }

   // Start at texture unit 3, since color map is texture unit 2 and
   // the input texture is texture unit 1.
   int textureId = 3;

   std::vector<std::string> parameterNames;
   pInputParams->getAttributeNames(parameterNames);
   std::vector<std::string>::iterator iter;
   for (iter = parameterNames.begin(); iter != parameterNames.end(); iter++) {
      std::string parameterName = *iter;

      // get entry point for parameter into GPU program
      int inputParameter = glGetUniformLocation(mProgramObject,parameterName.c_str());

      // check to make sure parameter entry point is valid
      if (inputParameter != -1) {
         // get the value for the parameter
         DataVariant parameterValue = pInputParams->getAttribute(parameterName);
         if (parameterValue.isValid()) {

            // Find the color buffer associated with the GPU input parameter
            // Is there a color buffer associated with the GPU input
            // parameter?
            // If so, then set the texture id as the input texture.
            std::map<std::string, ColorBuffer*>::const_iterator mapIter =  mColorBuffers.find(parameterName);
            if (mapIter != mColorBuffers.end())
            {
               // get the color buffer's texture id
               GLuint textureParameter = mapIter->second->getTextureObjectId();
               // set the texture id for the program input parameter
               int unit = textureId;
               if (parameterName.compare("inputImage") == 0) {
                  unit = 0;
               } else if (parameterName.compare("colorMap") == 0) {
                  unit = 1;
               }
               glActiveTexture(GL_TEXTURE0 + unit);
               glBindTexture(GL_TEXTURE_RECTANGLE_NV, textureParameter);
               glUniform1i(inputParameter, unit);

               glActiveTexture(GL_TEXTURE0);
               if ((unit != 0) && (unit != 1)) {
                  textureId++;
               }
            }
            else {
               // Only support floats for now.
               float floatParameter = 0.0f;
               parameterValue.getValue(floatParameter);
               glUniform1f(inputParameter, floatParameter);
               GLenum error = glGetError();
               if (error != GL_NO_ERROR) {
                  char tmpChar[20];
                  itoa(error, tmpChar, 10);
                  MessageResource msg("setGpuParameters Error:" + parameterName + std::string(tmpChar), "app", "{4A6C698A-5D90-4F35-BC51-2B3FE5A8A256}");
               }
            } 
         }
      }
   }
   return true;
}

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
void GlShaderGpuProgram::setParameterValues( Image::ImageData imageInfo, float alpha, int mColormapTexture)
{
   // get dynamic object that contains the input parameters for this GPU program
   const DynamicObject* pInputParams = mpGpuProgramDescriptor->getParameters();

   if (pInputParams == NULL) {
      return;
   }

   std::vector<std::string> parameterNames;
   pInputParams->getAttributeNames(parameterNames);

   int inputParameter = 0;
   std::vector<std::string>::iterator iter;
   for (iter = parameterNames.begin(); iter != parameterNames.end(); iter++) {
      std::string parameterName = *iter;

      if (!(parameterName.length() == 0)) {

         inputParameter = glGetUniformLocation(mProgramObject, parameterName.c_str());

         // Input image for the GPU program?
         if (parameterName.compare("inputImage") == 0) {
            mInputTexture = inputParameter;
         } else if (parameterName.compare("colorMap") == 0) {

            // Set the color map?
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_RECTANGLE_NV, mColormapTexture);
            glUniform1i(inputParameter, 2);
            glActiveTexture(GL_TEXTURE0);
         } else if (parameterName.compare("numColors") == 0) {
            // Number of colors in the color map?
            glUniform1f(inputParameter, (float) (imageInfo.mKey.mColorMap.size()));
         } else if ((parameterName.compare("dataMax") == 0)
            || (parameterName.compare("redDataMax") == 0)
            || (parameterName.compare("greenDataMax") == 0)
            || (parameterName.compare("blueDataMax") == 0)) {
               // Set the max value for the data type?

               // Default data type is assumed floating point
               EncodingType dataType = FLT4BYTES;

               // Same data type for each channel?
               if (((imageInfo.mRawType[0] == imageInfo.mRawType[1]) || imageInfo.mRawType[1] == NULL)
                  && ((imageInfo.mRawType[0] == imageInfo.mRawType[2]) || imageInfo.mRawType[2] == NULL)) {
                     if ((parameterName.compare("dataMax") == 0) || (parameterName.compare("redDataMax") == 0)) {
                           dataType = imageInfo.mRawType[0];
                     } else if (parameterName.compare("greenDataMax") == 0) {
                        dataType = imageInfo.mRawType[1];
                     } else if (parameterName.compare("blueDataMax") == 0) {
                        dataType = imageInfo.mRawType[2];
                     }
               }

               if ((dataType == INT1UBYTE) || (dataType == INT1SBYTE)) {
                     // If the data type is a "unsigned byte", must be signed times
                     // two
                     glUniform1f(inputParameter, (float) FLT_MAX);
               } else if ((dataType == INT2UBYTES) || (dataType == INT2SBYTES)) {
                     // If the data type is a "unsigned short", must be signed
                     // times two
                     glUniform1f(inputParameter, (SHRT_MAX * 2.0f) - 1);
               } else if ((dataType == INT4UBYTES) || (dataType == INT4SBYTES)) {
                     glUniform1f(inputParameter, (INT_MAX * 2.0f) - 1);
               } else {
                  // Assume the data type is a "float"
                  glUniform1f(inputParameter, 1.0f);
               }
         } else if ((parameterName.compare("lowerValue") == 0)
            || (parameterName.compare("redLowerValue") == 0)) {
               // Set the minimum stretch value for grey scale or the red
               // channel?
               float lowerValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints1[0]), imageInfo.mRawType[0]);
               glUniform1f(inputParameter, lowerValue);
         } else if ((parameterName.compare("upperValue") == 0)
            || (parameterName.compare("redUpperValue") == 0)) {
               // Set the maximum stretch value for grey scale or the red
               // channel?
               // TODO need to check the MStretchPoints1 size
               float upperValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints1[1]), imageInfo.mRawType[0]);
               glUniform1f(inputParameter, upperValue);
         } else if (parameterName.compare("greenLowerValue") == 0) {
            // Set the minimum stretch value for the green channel?
            float lowerValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints2[0]), imageInfo.mRawType[1]);
            glUniform1f(inputParameter, lowerValue);
         } else if (parameterName.compare("greenUpperValue") == 0) {
            // Set the maximum stretch value for the green channel?
            float upperValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints2[1]),imageInfo.mRawType[1]);
            glUniform1f(inputParameter, upperValue);
         } else if (parameterName.compare("blueLowerValue") == 0) {
            // Set the minimum stretch value for the blue channel?
            float lowerValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints3[0]), imageInfo.mRawType[2]);
            glUniform1f(inputParameter, lowerValue);
         } else if (parameterName.compare("blueUpperValue") == 0) {
            // Set the maximum stretch value for the blue channel?
            float upperValue = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints3[1]), imageInfo.mRawType[2]);
            glUniform1f(inputParameter, upperValue);
         } else if (parameterName.compare("alphaValue") == 0) {
            // Set the transparency value?
            float alpha1 = alpha;
            glUniform1f(inputParameter, alpha1);
         } else if (parameterName.compare("minBound") == 0) {
            //float minBound = 0.0; //?? JO (float) imageInfo.getSettings().getMinStretchPoint(0);
            float minBound = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints1[0]), imageInfo.mRawType[0]);
            glUniform1f(inputParameter, minBound);
         } else if (parameterName.compare("maxBound") == 0) {
            //float maxBound = 256.0; //?? JO (float) imageInfo.getSettings().getMaxStretchPoint(0);
            float maxBound = getTextureStretchValue(imageInfo, static_cast<float>(imageInfo.mKey.mStretchPoints1[1]), imageInfo.mRawType[0]);
            glUniform1f(inputParameter, maxBound);
         }
         
         GLenum error = glGetError();
         if (error != GL_NO_ERROR) {
            char tmpChar[20];
            itoa(error, tmpChar, 10);
            MessageResource msg("setGpuParameters Error:" + parameterName + " " + std::string(tmpChar), "app", "{4A6C698A-5D90-4F35-BC51-2B3FE5A8A256}");
            MessageResource msgVen("Vendor :" + std::string((const char*)glGetString(GL_VENDOR)), "app", "{E98F466A-3574-4F60-9F05-1035B6B16D46}");
            MessageResource msgRen("Renderer :" + std::string((const char*)glGetString(GL_RENDERER)), "app", "{338BF11C-6346-4D5C-A0E0-30B917DB05C2}");
            MessageResource msgVer("Version :" + std::string((const char*)glGetString(GL_VERSION)), "app", "{5CC7A8A6-AA44-4406-999F-5F00B7148EF1}");
         }
      }
   }
   glValidateProgram(mProgramObject);

   int result[2];
   glGetProgramiv(mProgramObject, GL_VALIDATE_STATUS, result);

   if (result[0] == GL_FALSE) {
      MessageResource msg("Shader program error: " + mProgramObject, "app", "{583E4309-919F-4F76-B795-BD9D96D77B5D}");
   }
}

/**
* Get the texture unit id.
* 
* @return the textureUnit
*/
int GlShaderGpuProgram::getTextureUnit()
{
   return mTextureUnit;
}

/**
* Set the texture unit id
* 
* @param unit
*           the textureUnit to set
*/
void GlShaderGpuProgram::setTextureUnit( int unit)
{
   mTextureUnit = unit;
}

/**
* Set the shader object id.
* 
* @param shader
*           the shaderObject to set
*/
void GlShaderGpuProgram::setShaderObject( int shader)
{
   mShaderObject = shader;
}

/**
* Return the compiled state of the program.
* 
* @return true if the program is compiled, else return false.
*/
bool GlShaderGpuProgram::isCompiled()
{
   return mCompiled;
}

void GlShaderGpuProgram::dispose()
{
   // only dispose the GL program if there are no more references to it.
   if (GlSlContext::instance()->destroyGlProgram(mpGpuProgramDescriptor->getName())) {
      disable();
      glDetachShader(mProgramObject, mShaderObject);
      glDeleteShader(mShaderObject);
      glDeleteProgram(mProgramObject);
      mDisposed = true;
   }
}

float GlShaderGpuProgram::getTextureStretchValue(Image::ImageData& imageInfo, float rawValue, EncodingType dataType)
{
   float stretchValue = rawValue;
   if ((imageInfo.mRawType[0] == imageInfo.mRawType[1]) && (imageInfo.mRawType[0] == imageInfo.mRawType[2])) {
      if (dataType == INT1SBYTE) {
         stretchValue -= CHAR_MIN;
      } else if (dataType == INT2SBYTES) {
         stretchValue -= SHRT_MIN;
      }
   }

   return stretchValue;
}

int GlShaderGpuProgram::getProgramObjectId()
{
   return mProgramObject;
}
