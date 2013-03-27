/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "MjpegExporter.h"
#include "PlugInRegistration.h"

REGISTER_PLUGIN_BASIC(OpticksMovieExporter, MjpegExporter);

MjpegExporter::MjpegExporter()
{
   setName("MJPEG Product Exporter");
   setExtensions("MJPEG Movie Files (*.avi)");
   setShortDescription("Export MJPEG Movie Products in an AVI Container");
   setDescriptorId("{4061e456-0c1b-4626-8f48-8028f2dbab11}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

MjpegExporter::~MjpegExporter()
{
}

AVOutputFormat* MjpegExporter::getOutputFormat() const
{
   AVOutputFormat* pFormat = guess_format("avi", NULL, NULL);
   if (pFormat != NULL)
   {
      pFormat->video_codec = CODEC_ID_MJPEG;
   }
   return pFormat;
}

boost::rational<int> MjpegExporter::convertToValidFrameRate(const boost::rational<int>& frameRate) const
{
   boost::rational<int> validFrameRate = MovieExporter::convertToValidFrameRate(frameRate);

   // can't do simple validFrameRate > 31 since (63, 2) is valid but boost::rational > would return true
   if (validFrameRate >= 32)
   {
      validFrameRate.assign(31, 1);
   }

   return validFrameRate;
}

bool MjpegExporter::setAvCodecOptions(AVCodecContext* pContext)
{
   if (!MovieExporter::setAvCodecOptions(pContext))
   {
      return false;
   }
   if (pContext->flags & CODEC_FLAG_TRELLIS_QUANT)
   {
      // Trellis quantization will cause the MJPEG coder to crash
      pContext->trellis = 0;
      pContext->flags = pContext->flags ^ CODEC_FLAG_TRELLIS_QUANT;
   }
   // Need to use the special YUVJ420P color format for MJPEG instead of regular YUV420P
   pContext->pix_fmt = PIX_FMT_YUVJ420P;
   return true;
}

bool MjpegExporter::convertToValidResolution(int& resolutionX, int& resolutionY) const
{
   // A lot of decoders have problems if the MJPEG video size isn't a multiple of 16
   // due to the DCT optimizations
   resolutionX += (16 - (resolutionX % 16)) % 16;
   resolutionY += (16 - (resolutionY % 16)) % 16;
   return true;
}