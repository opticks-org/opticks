/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationController.h"
#include "AppVersion.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "DesktopServices.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "MovieExporter.h"
#include "OptionsMovieExporter.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "UtilityServices.h" 

#include <boost/rational.hpp>
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QImage>
#include <string>
#include <vector>
#include <stdio.h>
using boost::rational;
using boost::rational_cast;
using namespace std;

template<>
Motion_Est_ID StringUtilities::fromXmlString<Motion_Est_ID>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }
   if(value == "Zero")        return ME_ZERO;
   else if(value == "PHODS")  return ME_PHODS;
   else if(value == "Log")    return ME_LOG;
   else if(value == "X1")     return ME_X1;
   else if(value == "EPZS")   return ME_EPZS;
   else if(value == "FULL")   return ME_FULL;

   if (pError != NULL)
   {
      *pError = true;
   }
   return Motion_Est_ID();
}

namespace
{
   const unsigned int sDefaultBitrate = 4500;
   QString logBuffer;
};

extern "C" static void av_log_callback(void *pPtr, int level, const char *pFmt, va_list vl)
{
   if(level > av_log_level)
   {
      return;
   }
   logBuffer.vsprintf(pFmt, vl);
}

MovieExporter::MovieExporter() :
   mIsBatch(true),
   mAbort(false),
   mpStep(NULL),
   mpPicture(NULL),
   mpVideoOutbuf(NULL),
   mFrameCount(0),
   mVideoOutbufSize(0)
{
   logBuffer.clear();
   av_log_set_callback(av_log_callback);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   av_register_all();
   setDescription("Export the contents of a view as it is updated during animation playback.");
   setSubtype(TypeConverter::toString<View>());
   setDescriptorId("{99F399A8-B003-4fdc-99A0-F45261144FF4}");
   allowMultipleInstances(true);
}

MovieExporter::~MovieExporter()
{
}

bool MovieExporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pInArgList != NULL);

   PlugInArg *pArg;
   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("FileDescriptor");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName(ExportItemArg());
   pArg->setType("View");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName("Resolution X");
   pArg->setType("unsigned int");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName("Resolution Y");
   pArg->setType("unsigned int");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName("Framerate Numerator");
   pArg->setType("int");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName("Framerate Denominator");
   pArg->setType("int");
   pInArgList->addArg(*pArg);

   VERIFY((pArg = mpPlugInManager->getPlugInArg()) != NULL);
   pArg->setName("Bitrate");
   pArg->setType("unsigned int");
   pArg->setDefaultValue(&sDefaultBitrate);
   pInArgList->addArg(*pArg);

   return true;
}

bool MovieExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

void MovieExporter::log_error(const string &msg)
{
   if(!logBuffer.isEmpty())
   {
      MessageResource pMsg("Detailed Message", "app", "E526E292-0713-41E6-92C1-4C9A2FA9C776");
      pMsg->addProperty("Message", logBuffer.toStdString());
   }
   if(mpProgress != NULL)
   {
      string progressMessage = msg;
      if(!logBuffer.isEmpty())
      {
         progressMessage += "\n";
         progressMessage += logBuffer.toStdString();
      }
      mpProgress->updateProgress(progressMessage, 100, ERRORS);
   }
   mpStep->finalize(Message::Failure, msg);
   logBuffer.clear();
}

bool MovieExporter::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   if(mpOptionWidget.get() != NULL)
   {
      mpOptionWidget->applyChanges();
   }

   FileDescriptor *pFileDescriptor = NULL;
   string filename;
   View *pView = NULL;
   AnimationController *pController = NULL;
   AVOutputFormat *pOutFormat = NULL;
   unsigned int resolutionX = 0;
   unsigned int resolutionY = 0;
   rational<int> framerate(0);
   unsigned int bitrate = 0;

   mpProgress = NULL;
   mpStep = StepResource("Export movie", "app", "2233BFC9-9C51-4e31-A8C5-2512925CBE6D");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "4551F478-E182-4b56-B88F-6682F0E3A2CF");

      mpProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (mpProgress != NULL));

      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
      if(pFileDescriptor == NULL)
      {
         log_error("No file specified");
         return false;
      }
      pMsg->addProperty("Destination", pFileDescriptor->getFilename());
      filename = pFileDescriptor->getFilename().getFullPathAndName();

      pView = pInArgList->getPlugInArgValue<View>(ExportItemArg());
      if(pView == NULL)
      {
         log_error("No view specified");
         return false;
      }

      pController = pView->getAnimationController();
      if(pController == NULL)
      {
         log_error("No animation controller specified");
         return false;
      }
      pMsg->addProperty("Animation Controller Name", pController->getName());

      if((pOutFormat = getOutputFormat()) == NULL)
      {
         log_error("Can't determine output format or format not supported.");
         return false;
      }
      pMsg->addProperty("Format", pOutFormat->long_name);

      if(!pInArgList->getPlugInArgValue("Resolution X", resolutionX) ||
         !pInArgList->getPlugInArgValue("Resolution Y", resolutionY))
      {
         if(mpOptionWidget.get() != NULL)
         {
            mpOptionWidget->getResolution(resolutionX, resolutionY);
         }
         else
         {
            resolutionX = OptionsMovieExporter::getSettingWidth();
            resolutionY = OptionsMovieExporter::getSettingHeight();
         }
         if(resolutionX == 0 || resolutionY == 0)
         {
            QWidget *pWidget = pView->getWidget();
            resolutionX = pWidget->width();
            resolutionY = pWidget->height();
         }
      }
      if((resolutionX % 2) != 0 || (resolutionY % 2) != 0)
      {
         log_error("Resolution must be a multiple of 2");
         return false;
      }
      pMsg->addProperty("Resolution", QString("%1 x %2").arg(resolutionX).arg(resolutionY).toStdString());

      int framerateNum, framerateDen;
      // first, get the framerate from the arg list
      // next, try the option widget
      // next, get from the animation controller
      // finally, default to the config settings
      if(pInArgList->getPlugInArgValue("Framerate Numerator", framerateNum) &&
         pInArgList->getPlugInArgValue("Framerate Denominator", framerateDen))
      {
         try
         {
            framerate.assign(framerateNum, framerateDen);
         }
         catch (const boost::bad_rational&)
         {
            // Do nothing; the code below handles this case
         }
      }
      if(framerate == 0)
      {
         if(mpOptionWidget.get() != NULL)
         {
            framerate = mpOptionWidget->getFramerate();
         }
         if(framerate == 0)
         {
            framerate = pController->getMinimumFrameRate();
         }
         if(framerate == 0)
         {
            try
            {
               framerate.assign(OptionsMovieExporter::getSettingFramerateNum(),
                  OptionsMovieExporter::getSettingFramerateDen());
            }
            catch (const boost::bad_rational&)
            {
               // Do nothing; the code below handles this case
            }
         }
         if(framerate == 0)
         {
            log_error("No framerate specified");
            return false;
         }
      }
      pMsg->addProperty("Framerate",
         QString("%1/%2").arg(framerate.numerator()).arg(framerate.denominator()).toStdString());

      if(!pInArgList->getPlugInArgValue("Bitrate", bitrate))
      {
         if(mpOptionWidget.get() != NULL)
         {
            bitrate = mpOptionWidget->getBitrate();
         }
         else
         {
            bitrate = OptionsMovieExporter::getSettingBitrate();
         }
      }
      pMsg->addProperty("Bitrate", QString::number(bitrate).toStdString());
   }

   AVFormatContext *pFormat = av_alloc_format_context();
   VERIFY(pFormat);
   pFormat->oformat = pOutFormat;
   snprintf(pFormat->filename, sizeof(pFormat->filename), "%s", filename.c_str());
   AVStream *pVideoStream = NULL;
   if(pOutFormat->video_codec != CODEC_ID_NONE)
   {
      pVideoStream = add_video_stream(pFormat, pOutFormat->video_codec);
   }
   if(pVideoStream == NULL)
   {
      log_error("Unable to create video stream.");
      return false;
   }

   /**
    * allow changing of:
    *    dia_size/
    */
   AVCodecContext *pCodecContext = pVideoStream->codec;
   if(!setAvCodecOptions(pCodecContext, pInArgList))
   {
      log_error("Unable to initialize CODEC options");
      return false;
   }
   // set time_base, width, height, and bitrate here since
   // they can be passed in via the input args
   pCodecContext->width = resolutionX;
   pCodecContext->height = resolutionY;
   pCodecContext->bit_rate = bitrate * 1000;
   // the AVCodecContext wants a time_base which is
   // the inverse of fps.
   pCodecContext->time_base.num = framerate.denominator();
   pCodecContext->time_base.den = framerate.numerator();

   if(av_set_parameters(pFormat, NULL) < 0)
   {
      log_error("Invalid output format parameters.");
      return false;
   }
   if(!open_video(pFormat, pVideoStream))
   {
      log_error("Unable to open video.");
      return false;
   }
   if(url_fopen(&pFormat->pb, filename.c_str(), URL_WRONLY) < 0)
   {
      log_error("Could not open the output file.");
      return false;
   }
   av_write_header(pFormat);

   // calculate time interval
   if((framerate < pController->getMinimumFrameRate()) && (mpProgress != NULL))
   {
      mpProgress->updateProgress("The selected output framerate may not\n"
                                 "encode all the frames in the movie. Frames\n"
                                 "may be dropped.", 0, WARNING);
   }
   double interval = rational_cast<double>(1 / framerate);

   // export the frames
   AVFrame *pTmpPicture = alloc_picture(PIX_FMT_RGBA32, pCodecContext->width, pCodecContext->height);
   QImage image(pTmpPicture->data[0], pCodecContext->width, pCodecContext->height, QImage::Format_ARGB32);
   for(double video_pts = pController->getStartFrame(); video_pts <= pController->getStopFrame();
       video_pts += interval)
   {
      if(mAbort)
      {
         if(mpProgress != NULL)
         {
            mpProgress->updateProgress("Export aborted", 0, ABORT);
         }
         mpStep->finalize(Message::Abort);
         return false;
      }
      // generate the next frame
      pController->setCurrentFrame(video_pts);
      if(mpProgress != NULL)
      {
         double progressValue = (video_pts - pController->getStartFrame()) /
                                (pController->getStopFrame() - pController->getStartFrame()) * 100.0;
         mpProgress->updateProgress("Saving movie", static_cast<int>(progressValue), NORMAL);
      }
      pView->getCurrentImage(image);
      img_convert(reinterpret_cast<AVPicture*>(mpPicture),
         pCodecContext->pix_fmt,
         reinterpret_cast<AVPicture*>(pTmpPicture),
         PIX_FMT_RGBA32,
         pCodecContext->width,
         pCodecContext->height);
      if(!write_video_frame(pFormat, pVideoStream))
      {
         string msg = "Can't write frame.";
         log_error(msg.c_str());
         return false;
      }
   }
   for(int frame = 0; frame < pCodecContext->delay; frame++)
   {
      write_video_frame(pFormat, pVideoStream);
   }
   av_write_trailer(pFormat);
   close_video(pFormat, pVideoStream);

   for(int i = 0; i < pFormat->nb_streams; i++)
   {
      av_freep(&pFormat->streams[i]->codec);
      av_freep(&pFormat->streams[i]);
   }
   url_fclose(&pFormat->pb);
   av_free(pFormat);

   if(mpProgress != NULL)
   {
      mpProgress->updateProgress("Finished saving movie", 100, NORMAL);
   }
   mpStep->finalize(Message::Success);
   return true;
}

ValidationResultType MovieExporter::validate(const PlugInArgList* pArgList, string& errorMessage) const
{
   /* validate some arguments */
   ValidationResultType result = ExporterShell::validate(pArgList, errorMessage);
   if(result != VALIDATE_SUCCESS)
   {
      return result;
   }
   View *pView = pArgList->getPlugInArgValue<View>(ExportItemArg());
   if(pView == NULL)
   {
      errorMessage = "No view specified. Nothing to export.";
      return VALIDATE_FAILURE;
   }
   AnimationController *pController = pView->getAnimationController();
   if(pController == NULL)
   {
      errorMessage = "No animation is associated with this view. Nothing to export.";
      return VALIDATE_FAILURE;
   }

   /* validate the framerate */
   AVOutputFormat *pOutFormat = getOutputFormat();
   VERIFYRV(pOutFormat, VALIDATE_FAILURE);
   AVCodec *pCodec = avcodec_find_encoder(pOutFormat->video_codec);
   VERIFYRV(pCodec, VALIDATE_FAILURE);
   if(pCodec->supported_framerates != NULL)
   {
      boost::rational<int> actualFrameRate;
      boost::rational<int> expectedFrameRate;

      // first, get the framerate from the arg list
      // next, try the option widget
      // next, get from the animation controller
      // finally, default to the config settings
      int framerateNum, framerateDen;
      if(pArgList->getPlugInArgValue("Framerate Numerator", framerateNum) &&
         pArgList->getPlugInArgValue("Framerate Denominator", framerateDen))
      {
         expectedFrameRate.assign(framerateNum, framerateDen);
      }
      if(expectedFrameRate == 0)
      {
         if(mpOptionWidget.get() != NULL)
         {
            expectedFrameRate = mpOptionWidget->getFramerate();
         }
         if(expectedFrameRate == 0)
         {
            expectedFrameRate = pController->getMinimumFrameRate();
         }
         if(expectedFrameRate == 0)
         {
            try
            {
               expectedFrameRate.assign(OptionsMovieExporter::getSettingFramerateNum(),
                  OptionsMovieExporter::getSettingFramerateDen());
            }
            catch (const boost::bad_rational&)
            {
               // Do nothing; the code below handles this case
            }
         }
         if(expectedFrameRate == 0)
         {
            errorMessage = "No framerate specified";
            return VALIDATE_FAILURE;
         }
      }

      try
      {
         for(int idx = 0;; idx++)
         {
            boost::rational<int> frameRate(pCodec->supported_framerates[idx].num,
                                           pCodec->supported_framerates[idx].den);
            if(frameRate == 0)
            {
               break;
            }
            if((frameRate == expectedFrameRate) || // the expected FR matches a valid FR
               (actualFrameRate == 0) ||           // we havn't saved a FR so pick something valid
               // the diff between the expected and current FR is closer than the stored FR
               ((frameRate - expectedFrameRate) < (actualFrameRate - expectedFrameRate)) ||
               // the current FR is greater than the expected and the stored is less than the expected
               ((actualFrameRate - expectedFrameRate) < 0 && (frameRate - expectedFrameRate) > 0))
            {
               actualFrameRate = frameRate;
            }
         }
      }
      catch(const boost::bad_rational&)
      {
         // intentionally left blank
      }
      if(actualFrameRate < pController->getMinimumFrameRate())
      {
         errorMessage = "The selected output framerate may not encode all the frames in the movie. "
                        "Frames may be dropped.";
         result = VALIDATE_INFO;
      }
      if(actualFrameRate != expectedFrameRate)
      {
         QString msg = QString("The frame rate attached to the animation (%1/%2 fps) can not be\n"
                               "represented in the selected movie format. %3/%4 fps is being used instead.\n"
                               "The frame rate stored in the file may be changed using the options dialog.")
                              .arg(expectedFrameRate.numerator())
                              .arg(expectedFrameRate.denominator())
                              .arg(actualFrameRate.numerator())
                              .arg(actualFrameRate.denominator());
         if(!errorMessage.empty())
         {
            errorMessage += "\n\n";
         }
         errorMessage += msg.toStdString();
         result = VALIDATE_INFO;
      }
   }
   return result;
}

QWidget *MovieExporter::getExportOptionsWidget(const PlugInArgList *pInArgList)
{
   if(mpOptionWidget.get() == NULL)
   {
      mpOptionWidget = auto_ptr<OptionsMovieExporter>(new OptionsMovieExporter());
      VERIFYRV(mpOptionWidget.get() != NULL, NULL);
      mpOptionWidget->setPromptUserToSaveSettings(true);

      View *pView = pInArgList->getPlugInArgValue<View>(ExportItemArg());
      if(pView != NULL)
      {
         QWidget *pWidget = pView->getWidget();
         mpOptionWidget->setResolution(pWidget->width(), pWidget->height());

         AnimationController *pController = pView->getAnimationController();
         if(pController != NULL)
         {
            rational<int> frameRate = pController->getMinimumFrameRate();
            mpOptionWidget->setFramerate(frameRate);
         }
      }

      AVOutputFormat *pOutFormat = getOutputFormat();
      VERIFYRV(pOutFormat, NULL);
      AVCodec *pCodec = avcodec_find_encoder(pOutFormat->video_codec);
      VERIFYRV(pCodec, NULL);
      if(pCodec->supported_framerates != NULL)
      {
         vector<boost::rational<int> > frameRates;
         try
         {
            for(int idx = 0;; idx++)
            {
               boost::rational<int> frameRate(pCodec->supported_framerates[idx].num,
                  pCodec->supported_framerates[idx].den);
               if(frameRate == 0)
               {
                  break;
               }
               frameRates.push_back(frameRate);
            }
         }
         catch(const boost::bad_rational&)
         {
            // intentionally left blank
         }
         mpOptionWidget->setFramerates(frameRates);
      }
   }
   return mpOptionWidget.get();
}

bool MovieExporter::setAvCodecOptions(AVCodecContext *pContext, PlugInArgList *pInArgList)
{
   if(pContext == NULL || pInArgList == NULL)
   {
      return false;
   }
   string meMethod = OptionsMovieExporter::getSettingMeMethod();
   pContext->gop_size = OptionsMovieExporter::getSettingGopSize();
   pContext->qcompress = OptionsMovieExporter::getSettingQCompress();
   pContext->qblur = OptionsMovieExporter::getSettingQBlur();
   pContext->qmin = OptionsMovieExporter::getSettingQMinimum();
   pContext->qmax = OptionsMovieExporter::getSettingQMaximum();
   pContext->max_qdiff = OptionsMovieExporter::getSettingQDiffMaximum();
   pContext->max_b_frames = OptionsMovieExporter::getSettingMaxBFrames();
   pContext->b_quant_factor = OptionsMovieExporter::getSettingBQuantFactor();
   pContext->b_quant_offset = OptionsMovieExporter::getSettingBQuantOffset();
   pContext->dia_size = OptionsMovieExporter::getSettingDiaSize();
   int newFlags = OptionsMovieExporter::getSettingFlags();
   if(mpOptionWidget.get() != NULL)
   {
      meMethod = mpOptionWidget->getMeMethod();
      pContext->gop_size= mpOptionWidget->getGopSize();
      pContext->qcompress = mpOptionWidget->getQCompress();
      pContext->qblur = mpOptionWidget->getQBlur();
      pContext->qmin = mpOptionWidget->getQMinimum();
      pContext->qmax = mpOptionWidget->getQMaximum();
      pContext->max_qdiff = mpOptionWidget->getQDiffMaximum();
      pContext->max_b_frames = mpOptionWidget->getMaxBFrames();
      pContext->b_quant_factor = mpOptionWidget->getBQuantFactor();
      pContext->b_quant_offset = mpOptionWidget->getBQuantOffset();
      pContext->dia_size = mpOptionWidget->getDiaSize();
      newFlags = mpOptionWidget->getFlags();
   }
   pContext->me_method = StringUtilities::fromXmlString<Motion_Est_ID>(meMethod);
   pContext->flags |= newFlags;
   if(pContext->flags & CODEC_FLAG_TRELLIS_QUANT)
   {
      pContext->trellis = 1;
   }
   else
   {
      pContext->trellis = 0;
   }

   return true;
}

AVStream *MovieExporter::add_video_stream(AVFormatContext *pFormat, CodecID codec_id)
{
   AVStream *pVideoStream = av_new_stream(pFormat, 0);
   VERIFYRV(pVideoStream, NULL);

   AVCodecContext *pCodecContext = pVideoStream->codec;
   pCodecContext->codec_id = codec_id;
   pCodecContext->codec_type = CODEC_TYPE_VIDEO;

   pCodecContext->gop_size = 12; /* emit one intra frame every twelve frames at most */
   pCodecContext->pix_fmt = PIX_FMT_YUV420P;
   if(pFormat->oformat->flags & AVFMT_GLOBALHEADER)
   {
      pCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
   }
   if(pCodecContext->codec_id == CODEC_ID_MPEG1VIDEO)
   {
      /* needed to avoid using macroblocks in which some coeffs overflow
      this doesnt happen with normal video, it just happens here as the
      motion of the chroma plane doesnt match the luma plane */
      pCodecContext->mb_decision=2;
   }
   // some formats want stream headers to be seperate
   string format = pFormat->oformat->name;
   if(format == "mp4" || format == "mov" || format == "3gp")
   {
      pCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
   }

   return pVideoStream;
}

bool MovieExporter::open_video(AVFormatContext *pFormat, AVStream *pVideoStream)
{
   VERIFY(pFormat && pVideoStream);
   AVCodecContext *pCodecContext = pVideoStream->codec;
   AVCodec *pCodec = avcodec_find_encoder(pCodecContext->codec_id);
   if(pCodec == NULL)
   {
      return false;
   }

   if((pCodec = avcodec_find_encoder(pCodecContext->codec_id)) == NULL)
   {
      return false;
   }

   if(avcodec_open(pCodecContext, pCodec) < 0)
   {
      return false;
   }

   mpVideoOutbuf = NULL;
   if(!(pFormat->oformat->flags & AVFMT_RAWPICTURE))
   {
      mVideoOutbufSize = 200000;
      mpVideoOutbuf = reinterpret_cast<uint8_t*>(malloc(mVideoOutbufSize));
   }

   /* allocate the encoded raw picture */
   if((mpPicture = alloc_picture(pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height)) == NULL)
   {
      return false;
   }

   return true;
}

AVFrame *MovieExporter::alloc_picture(int pixFmt, int width, int height)
{
   AVFrame *pPicture = avcodec_alloc_frame();
   VERIFYRV(pPicture, NULL);
   int size = avpicture_get_size(pixFmt, width, height);
   uint8_t *pPictureBuf = reinterpret_cast<uint8_t*>(malloc(size));
   if(pPictureBuf == NULL)
   {
      av_free(pPicture);
      return NULL;
   }
   avpicture_fill(reinterpret_cast<AVPicture*>(pPicture), pPictureBuf, pixFmt, width, height);
   return pPicture;
}

bool MovieExporter::write_video_frame(AVFormatContext *pFormat, AVStream *pVideoStream)
{
   VERIFY(pFormat && pVideoStream);
   int out_size = 0;
   int ret = 0;
   AVCodecContext *pCodecContext = pVideoStream->codec;

   if(pFormat->oformat->flags & AVFMT_RAWPICTURE)
   {
      /* raw video case. The API will change slightly in the near
      futur for that */
      AVPacket pkt;
      av_init_packet(&pkt);

      pkt.flags |= PKT_FLAG_KEY;
      pkt.stream_index = pVideoStream->index;
      pkt.data = reinterpret_cast<uint8_t*>(mpPicture);
      pkt.size = sizeof(AVPicture);

      ret = av_write_frame(pFormat, &pkt);
   }
   else
   {
      /* encode the image */
      out_size = avcodec_encode_video(pCodecContext, mpVideoOutbuf, mVideoOutbufSize, mpPicture);
      /* if zero size, it means the image was buffered */
      if(out_size > 0)
      {
         AVPacket pkt;
         av_init_packet(&pkt);

         pkt.pts= av_rescale_q(pCodecContext->coded_frame->pts, pCodecContext->time_base, pVideoStream->time_base);
         if(pCodecContext->coded_frame->key_frame)
         {
            pkt.flags |= PKT_FLAG_KEY;
         }
         pkt.stream_index = pVideoStream->index;
         pkt.data = mpVideoOutbuf;
         pkt.size = out_size;

         /* write the compressed frame in the media file */
         ret = av_write_frame(pFormat, &pkt);
      }
      else
      {
         ret = 0;
      }
   }
   if(ret != 0)
   {
      return false;
   }
   mFrameCount++;
   return true;
}

void MovieExporter::close_video(AVFormatContext *pFormat, AVStream *pVideoStream)
{
   VERIFYNRV(pFormat && pVideoStream);
   avcodec_close(pVideoStream->codec);
   mpPicture = NULL;
   mpVideoOutbuf = NULL;
}
