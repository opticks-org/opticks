/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOVIEEXPORTER_H
#define MOVIEEXPORTER_H

#include "ExporterShell.h"
#include "MessageLogResource.h"

#pragma warning(push, 1)
#pragma warning(disable: 4244)
#if defined(LINUX)
#undef OFFSET_T_DEFINED
#endif
extern "C" {
#include "avformat.h"
#include "avutil.h"
}
#pragma warning(pop)

#if !defined(CODEC_FLAG_TRELLIS_QUANT)
typedef uint64_t codec_flag_t;
#define OPTICKS_CODEC_FLAG_TRELLIS_QUANT  0xf00000000LL    // CODEC_FLAG_CLOSED_GOP is 0x80000000, we need twice that
#else
typedef uint32_t codec_flag_t;
#define OPTICKS_CODEC_FLAG_TRELLIS_QUANT  CODEC_FLAG_TRELLIS_QUANT
#endif

#include <boost/rational.hpp>
#include <memory>

class AnimationController;
class MovieExportOptionsWidget;
class Progress;
class QWidget;

class AvFormatContextObject
{
public:
   class Args
   {
   public:
      Args(AVOutputFormat* pOutputFormat = NULL) :
         mpOutputFormat(pOutputFormat)
      {
      }

      AVOutputFormat* mpOutputFormat;
   };

   AVFormatContext* obtainResource(const Args& args) const
   {
      AVFormatContext* pFormat(NULL);
      if (args.mpOutputFormat == NULL)
      {
         return pFormat;
      }
      pFormat = av_alloc_format_context();
      VERIFYRV(pFormat, NULL);
      pFormat->oformat = args.mpOutputFormat;
      return pFormat;
   }

   void releaseResource(const Args& args, AVFormatContext* pFormat) const
   {
      if (pFormat != NULL)
      {
         for (int i = 0; i < pFormat->nb_streams; ++i)
         {
            if (pFormat->streams[i]->codec != NULL)
            {
               av_freep(&pFormat->streams[i]->codec);
               pFormat->streams[i]->codec = NULL;
            }
            av_freep(&pFormat->streams[i]);
         }
#if (LIBAVFORMAT_VERSION_INT < ((52<<16)))
         if (pFormat->pb.opaque != NULL)
         {
            url_fclose(&pFormat->pb);
         }
#else
         if (pFormat->pb->opaque != NULL)
         {
            url_fclose(pFormat->pb);
         }
#endif
         av_free(pFormat);
      }
   }
};

class AvFormatContextResource : public Resource<AVFormatContext, AvFormatContextObject>
{
public:
   AvFormatContextResource(AVOutputFormat* pOutputFormat) :
      Resource<AVFormatContext, AvFormatContextObject>(AvFormatContextObject::Args(pOutputFormat))
   {
   }

   operator AVFormatContext*()
   {
      return get();
   }
};

class AvStreamObject
{
public:
   class Args
   {
   public:
      Args(AVFormatContext* pFormat = NULL, CodecID videoCodec = CODEC_ID_NONE) :
         mpFormat(pFormat),
         mVideoCodec(videoCodec)
      {
      }

      AVFormatContext* mpFormat;
      CodecID mVideoCodec;
   };

   AVStream* obtainResource(const Args& args) const
   {
      AVStream* pVideoStream(NULL);
      if (args.mpFormat == NULL || args.mVideoCodec == CODEC_ID_NONE)
      {
         return pVideoStream;
      }
      pVideoStream = av_new_stream(args.mpFormat, 0);
      VERIFYRV(pVideoStream, NULL);

      AVCodecContext* pCodecContext = pVideoStream->codec;
      pCodecContext->codec_id = args.mVideoCodec;
      pCodecContext->codec_type = CODEC_TYPE_VIDEO;

      pCodecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
      pCodecContext->pix_fmt = PIX_FMT_YUV420P;
      if (args.mpFormat->oformat->flags & AVFMT_GLOBALHEADER)
      {
         pCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
      }
      if (pCodecContext->codec_id == CODEC_ID_MPEG1VIDEO)
      {
         /* needed to avoid using macroblocks in which some coeffs overflow
         this doesn't happen with normal video, it just happens here as the
         motion of the chroma plane doesn't match the luma plane */
         pCodecContext->mb_decision = 2;
      }
      // some formats want stream headers to be separate
      std::string format = args.mpFormat->oformat->name;
      if (format == "mp4" || format == "mov" || format == "3gp")
      {
         pCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
      }

      return pVideoStream;
   }

   void releaseResource(const Args& args, AVStream* pStream) const
   {
      // need to check codec and codec->codec in case there is a partial initialization
      // if we don't avcodec_close will crash
      if (pStream != NULL && pStream->codec != NULL && pStream->codec->codec != NULL)
      {
         avcodec_close(pStream->codec);
      }
   }
};

class AvStreamResource : public Resource<AVStream, AvStreamObject>
{
public:
   AvStreamResource(AVFormatContext* pFormat = NULL, CodecID videoCodec = CODEC_ID_NONE) :
      Resource<AVStream, AvStreamObject>(AvStreamObject::Args(pFormat, videoCodec))
   {
   }

   operator AVStream*()
   {
      return get();
   }
};

/**
 *  Movie Exporter
 *
 *  This plug-in exports core movie sequences to movie files.
 */
class MovieExporter : public ExporterShell
{
public:
   MovieExporter();
   virtual ~MovieExporter();

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;
   QWidget* getExportOptionsWidget(const PlugInArgList* pInArgList);

protected:
   virtual bool setAvCodecOptions(AVCodecContext* pContext);
   virtual bool open_video(AVFormatContext* pFormat, AVStream* pVideoStream);
   virtual AVFrame* alloc_picture(PixelFormat pixFmt, int width, int height);
   virtual bool write_video_frame(AVFormatContext* pFormat, AVStream* pVideoStream);

   virtual AVOutputFormat* getOutputFormat() const = 0;

   virtual boost::rational<int> getFrameRate(const AnimationController* pController) const;
   virtual boost::rational<int> convertToValidFrameRate(const boost::rational<int>& frameRate) const;
   virtual bool convertToValidResolution(int& resolutionX, int& resolutionY) const;

private:
   void log_error(const std::string& msg);

   Progress* mpProgress;
   StepResource mpStep;
   AVFrame* mpPicture;
   uint8_t* mpVideoOutbuf;
   int mFrameCount;
   int mVideoOutbufSize;

   std::auto_ptr<MovieExportOptionsWidget> mpOptionWidget;
};

#endif
