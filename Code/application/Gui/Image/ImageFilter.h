/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include "ImageFilterDescriptor.h"
#include "GpuProgram.h"

#include <string>
#include <vector>
#include <map>

// class forward declarations
class ColorBuffer;
class ImageBuffer;
class GpuResourceManager;

/**
 *  This class is the base class to derive various image filters.
 *
 *  This class contains an ImageFilterDescriptor object that provides
 *  the necessary information to perform the image filter algorithm. This
 *  class expects one GPU program to perform the filtering process and it
 *  uses one offscreen buffer to render the results to. This is one type
 *  of filtering process which can be derived from to implement other
 *  filtering processes.
 *
 *  @see FeedBackFilter
 */
class ImageFilter
{
public:
  /**
   *  Constructor to the ImageFilter class.
   *
   *  @param   pDescriptor
   *           Image Filter descriptor.
   */
   explicit ImageFilter(ImageFilterDescriptor *pDescriptor);

   /**
    *  Destructor to the ImageFilter class.
    */
   virtual ~ImageFilter();

   ImageFilterDescriptor::ImageProcessType getFilterType() const;

   virtual bool setImage(ColorBuffer *pInputColorBuffer);

   virtual void resetBuffer();
   virtual void freezeBuffer(bool toggle = true);

   /**
    *  Returns whether or not the image filter has been initialized.
    *
    *  @return  bool
    *           Returns true if image filter initialized or false 
    *           if image filter not initialized.
    */
   bool isInitialized() { return (mBuffers.empty() != true); }

   /**
    *  Applys the filter to the input image operator
    *  and returns the results in an image buffer.
    *
    *  @return  The results of the image filtering algorithm.
    */
   ColorBuffer *applyFilter();

   /**
    *  Returns the ImageFilterDescriptor of the image filter.
    *
    *  @return  The ImageFilterDescriptor for the image filter.
    */
   ImageFilterDescriptor *getImageFilterDescriptor() { return mpDescriptor; }
   
   /**
    *  Returns the color buffer which contains the results of the image filter.
    *
    *  @return  The ColorBuffer which contains the results of the image filter.
    */
   ColorBuffer *getResultsBuffer() { return mpResultsBuffer; }

   /**
    *  Returns the image buffer from which the input ColorBuffer is attached.
    *
    *  @return  The ImageBuffer that has the input ColorBuffer.
    */
   ImageBuffer* getImageBuffer(ColorBuffer *pColorBuffer);

protected:
   void render(unsigned int width, unsigned int height);
   std::vector<GpuProgram*>& getGpuPrograms() { return mGpuPrograms; }
   std::vector<ImageBuffer*>& getImageBuffers() { return mImageBuffers; }
   ColorBuffer *copyColorBuffer(ColorBuffer *pColorBuffer);
   bool populateTextureParameters(ColorBuffer *pColorBuffer);
   bool attachToImageBuffer(ColorBuffer *pColorBuffer);

private: // member variables
   ColorBuffer *mpInputColorBuffer;
   std::vector<ImageBuffer*> mImageBuffers;
   std::vector<ColorBuffer*> mBuffers;
   ColorBuffer *mpResultsBuffer;
   ImageFilterDescriptor *mpDescriptor;
   std::vector<GpuProgram*> mGpuPrograms;
   bool mFreezeFlag;
};

#endif
