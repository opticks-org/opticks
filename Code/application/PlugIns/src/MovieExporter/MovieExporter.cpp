/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AdvancedOptionsWidget.h"
#include "AnimationController.h"
#include "AnimationFrameSubsetWidget.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BitrateWidget.h"
#include "ColorType.h"
#include "FileDescriptor.h"
#include "FramerateWidget.h"
#include "MovieExporter.h"
#include "MovieExportOptionsWidget.h"
#include "OptionsMovieExporter.h"
#include "PerspectiveView.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "Progress.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "View.h"
#include "ViewResolutionWidget.h"

#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
using boost::rational;
using boost::rational_cast;
using namespace std;

namespace StringUtilities
{
template<>
Motion_Est_ID fromXmlString<Motion_Est_ID>(string value, bool* pError)
{
   if (pError != NULL)
   {
      *pError = false;
   }

   if (value == "Zero")
   {
      return ME_ZERO;
   }
   else if (value == "PHODS")
   {
      return ME_PHODS;
   }
   else if (value == "Log")
   {
      return ME_LOG;
   }
   else if (value == "X1")
   {
      return ME_X1;
   }
   else if (value == "EPZS")
   {
      return ME_EPZS;
   }
   else if (value == "FULL")
   {
      return ME_FULL;
   }

   if (pError != NULL)
   {
      *pError = true;
   }

   return Motion_Est_ID();
}
}

namespace
{
   const unsigned int sDefaultBitrate = 4500;
   QString logBuffer;
};

#if defined(LINUX)
extern "C" void av_log_callback(void* pPtr, int level, const char* pFmt, va_list vl)
#else
extern "C" static void av_log_callback(void* pPtr, int level, const char* pFmt, va_list vl)
#endif
{
   if (level > av_log_level)
   {
      return;
   }
   logBuffer.vsprintf(pFmt, vl);
}

MovieExporter::MovieExporter() :
   mpProgress(NULL),
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
   setAbortSupported(true);
}

MovieExporter::~MovieExporter()
{}

bool MovieExporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   pInArgList = pPlugInManager->getPlugInArgList();

   VERIFY(pInArgList != NULL);
   VERIFY(pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pInArgList->addArg<FileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the output file."));
   VERIFY(pInArgList->addArg<View>(Exporter::ExportItemArg(), "View to be exported."));
   VERIFY(pInArgList->addArg<unsigned int>("Resolution X", "X resolution of the output file."));
   VERIFY(pInArgList->addArg<unsigned int>("Resolution Y", "Y resolution of the output file."));
   VERIFY(pInArgList->addArg<int>("Framerate Numerator", "Numerator (frames) in frames/second calculation."));
   VERIFY(pInArgList->addArg<int>("Framerate Denominator", "Denominator (seconds) in frames/second calculation."));
   VERIFY(pInArgList->addArg<unsigned int>("Bitrate", 
      "Bitrate for the output file; affects quality of the output file."));
   string description("Time for time-based animations. "
      "1-based frame number for frame-based animations, i.e., first frame is 1.");
   VERIFY(pInArgList->addArg<double>("Start Export", description));
   VERIFY(pInArgList->addArg<double>("Stop Export", description));

   return true;
}

bool MovieExporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

void MovieExporter::log_error(const string& msg)
{
   if (!logBuffer.isEmpty())
   {
      MessageResource pMsg("Detailed Message", "app", "E526E292-0713-41E6-92C1-4C9A2FA9C776");
      pMsg->addProperty("Message", logBuffer.toStdString());
   }
   if (mpProgress != NULL)
   {
      string progressMessage = msg;
      if (!logBuffer.isEmpty())
      {
         progressMessage += "\n";
         progressMessage += logBuffer.toStdString();
      }
      mpProgress->updateProgress(progressMessage, 100, ERRORS);
   }
   mpStep->finalize(Message::Failure, msg);
   logBuffer.clear();
}

bool MovieExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   FileDescriptor* pFileDescriptor(NULL);
   string filename;
   View* pView(NULL);
   AnimationController* pController(NULL);
   FrameType eType;
   AVOutputFormat* pOutFormat(NULL);
   int resolutionX(-1);
   int resolutionY(-1);
   rational<int> framerate(0);
   unsigned int bitrate(0);
   double startExport(0.0);
   double stopExport(0.0);
   bool fullResolution(false);

   mpProgress = NULL;
   mpStep = StepResource("Export movie", "app", "2233BFC9-9C51-4e31-A8C5-2512925CBE6D");

   // get input arguments and log some useful info about them
   { // scope the MessageResource
      MessageResource pMsg("Input arguments", "app", "4551F478-E182-4b56-B88F-6682F0E3A2CF");

      mpProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
      pMsg->addBooleanProperty("Progress Present", (mpProgress != NULL));

      pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
      if (pFileDescriptor == NULL)
      {
         log_error("No file specified");
         return false;
      }
      pMsg->addProperty("Destination", pFileDescriptor->getFilename());
      filename = pFileDescriptor->getFilename().getFullPathAndName();

      pView = pInArgList->getPlugInArgValue<View>(Exporter::ExportItemArg());
      if (pView == NULL)
      {
         log_error("No view specified");
         return false;
      }

      pController = pView->getAnimationController();
      if (pController == NULL)
      {
         log_error("No animation controller specified");
         return false;
      }
      pMsg->addProperty("Animation Controller Name", pController->getName());

      pOutFormat = getOutputFormat();
      if (pOutFormat == NULL)
      {
         log_error("Can't determine output format or format not supported.");
         return false;
      }
      pMsg->addProperty("Format", pOutFormat->long_name);

      bool resolutionFromInputArgs(false);
      if (!pInArgList->getPlugInArgValue("Resolution X", resolutionX) ||
         !pInArgList->getPlugInArgValue("Resolution Y", resolutionY))
      {
         if (mpOptionWidget.get() != NULL)
         {
            ViewResolutionWidget* pResolutionWidget = mpOptionWidget->getResolutionWidget();
            VERIFY(pResolutionWidget != NULL);

            switch(pResolutionWidget->getResolutionType())
            {
            case OptionsMovieExporter::VIEW_RESOLUTION:
               resolutionX = -1;
               resolutionY = -1;
               fullResolution = false;
               break;
            case OptionsMovieExporter::FULL_RESOLUTION:
               resolutionX = -1;
               resolutionY = -1;
               fullResolution = true;
               break;
            case OptionsMovieExporter::FIXED_RESOLUTION:
            {
               QSize resolution = pResolutionWidget->getResolution();
               resolutionX = resolution.width();
               resolutionY = resolution.height();
               fullResolution = false;
               break;
            }
            default:
               break; // nothing
            }
         }
         else
         {
            switch(StringUtilities::fromXmlString<OptionsMovieExporter::ResolutionType>(
               OptionsMovieExporter::getSettingResolutionType()))
            {
            case OptionsMovieExporter::VIEW_RESOLUTION:
               resolutionX = -1;
               resolutionY = -1;
               fullResolution = false;
               break;
            case OptionsMovieExporter::FULL_RESOLUTION:
               resolutionX = -1;
               resolutionY = -1;
               fullResolution = true;
               break;
            case OptionsMovieExporter::FIXED_RESOLUTION:
               resolutionX = OptionsMovieExporter::getSettingWidth();
               resolutionY = OptionsMovieExporter::getSettingHeight();
               fullResolution = false;
               break;
            default:
               break; // nothing
            }
         }
      }
      else
      {
         resolutionFromInputArgs = true;
      }

      if (resolutionX <= 0 || resolutionY <= 0)
      {
         QWidget* pWidget = pView->getWidget();
         if (pWidget != NULL)
         {
            resolutionX = pWidget->width();
            resolutionY = pWidget->height();
            resolutionFromInputArgs = false;
            if (fullResolution)
            {
               PerspectiveView* pPersp = dynamic_cast<PerspectiveView*>(pView);
               if (pPersp != NULL && pPersp->getZoomPercentage() > 100.)
               {
                  fullResolution = false;
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress("Full resolution export will be smaller than "
                        "view export, changing export resolution to current view size.", 0, WARNING);
                  }
               }
               else
               {
                  // translate to data coordinate
                  double x1 = 0.0;
                  double y1 = 0.0;
                  double x2 = 0.0;
                  double y2 = 0.0;
                  pView->translateScreenToWorld(0.0, 0.0, x1, y1);
                  pView->translateScreenToWorld(resolutionX, resolutionY, x2, y2);
                  resolutionX = (x2 > x1) ? (x2 - x1 + 0.5) : (x1 - x2 + 0.5);
                  resolutionY = (y2 > y1) ? (y2 - y1 + 0.5) : (y1 - y2 + 0.5);
               }
            }
         }
         else
         {
            log_error("Can't determine output resolution.");
            return false;
         }
      }

      int oldResX = resolutionX;
      int oldResY = resolutionY;
      if (!convertToValidResolution(resolutionX, resolutionY) ||
            (resolutionFromInputArgs && (resolutionX != oldResX || resolutionY != oldResY)))
      {
         stringstream msg;
         msg << "The export resolution does not meet the requirements of the the selected CODEC. "
                "The input arguments were X resolution of "
             << oldResX << " and Y resolution of " << oldResY << "."
             << "The adjusted resolution was (" << resolutionX << ", " << resolutionY << ")";
         log_error(msg.str());
         return false;
      }

      pMsg->addProperty("Resolution", QString("%1 x %2").arg(resolutionX).arg(resolutionY).toStdString());

      int framerateNum = 0;
      int framerateDen = 0;
      // first, get the framerate from the arg list
      // next, try the option widget
      // next, get from the animation controller
      if (pInArgList->getPlugInArgValue("Framerate Numerator", framerateNum) &&
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
      if (framerate == 0)
      {
         if (mpOptionWidget.get() != NULL)
         {
            FramerateWidget* pFramerateWidget = mpOptionWidget->getFramerateWidget();
            VERIFY(pFramerateWidget != NULL);

            framerate = pFramerateWidget->getFramerate();
         }
         else
         {
            framerate = getFrameRate(pController);
         }
      }

      if (framerate == 0)
      {
         log_error("No framerate specified");
         return false;
      }

      // Validate the framerate
      boost::rational<int> validFrameRate = convertToValidFrameRate(framerate);
      if (validFrameRate != framerate)
      {
         QString msg = QString("The selected animation frame rate (%1/%2 fps) can not be represented in the "
                               "selected movie format. A frame rate of %3/%4 fps is being used instead.")
                              .arg(framerate.numerator())
                              .arg(framerate.denominator())
                              .arg(validFrameRate.numerator())
                              .arg(validFrameRate.denominator());
         mpProgress->updateProgress(msg.toStdString(), 0, WARNING);

         framerate = validFrameRate;
      }

      pMsg->addProperty("Framerate",
         QString("%1/%2").arg(framerate.numerator()).arg(framerate.denominator()).toStdString());

      if (!pInArgList->getPlugInArgValue("Bitrate", bitrate))
      {
         if (mpOptionWidget.get() != NULL)
         {
            BitrateWidget* pBitrateWidget = mpOptionWidget->getBitrateWidget();
            VERIFY(pBitrateWidget != NULL);

            bitrate = pBitrateWidget->getBitrate();
         }
         else
         {
            bitrate = OptionsMovieExporter::getSettingBitrate();
         }
      }
      pMsg->addProperty("Bitrate", QString::number(bitrate).toStdString());

      eType = pController->getFrameType();
      if (!pInArgList->getPlugInArgValue("Start Export", startExport))
      {
         if (mpOptionWidget.get() != NULL)
         {
            AnimationFrameSubsetWidget* pSubsetWidget = mpOptionWidget->getSubsetWidget();
            VERIFY(pSubsetWidget != NULL);

            startExport = pSubsetWidget->getStartFrame();
         }
         else
         {
            if (pController->getBumpersEnabled())
            {
               startExport = pController->getStartBumper();
            }
            else
            {
               startExport = pController->getStartFrame();
            }
         }
      }
      else
      {
         // adjust to 0-based since the input arg uses 1-based
         --startExport;
      }

      if (!pInArgList->getPlugInArgValue("Stop Export", stopExport))
      {
         if (mpOptionWidget.get() != NULL)
         {
            AnimationFrameSubsetWidget* pSubsetWidget = mpOptionWidget->getSubsetWidget();
            VERIFY(pSubsetWidget != NULL);

            stopExport = pSubsetWidget->getStopFrame();
         }
         else
         {
            if (pController->getBumpersEnabled())
            {
               stopExport = pController->getStopBumper();
            }
            else
            {
               stopExport = pController->getStopFrame();
            }
         }
      }
      else
      {
         // adjust to 0-based since the input arg users 1-based
         --stopExport;
      }
      string valueType("Time");
      if (eType == FRAME_ID)
      {
         valueType = "Frame";
      }

      pMsg->addProperty("Start "+valueType, QString::number(startExport).toStdString());
      pMsg->addProperty("Stop "+valueType, QString::number(stopExport).toStdString());
   }

   AvFormatContextResource pFormat(pOutFormat);
   VERIFY(pFormat != NULL);
   snprintf(pFormat->filename, sizeof(pFormat->filename), "%s", filename.c_str());
   AvStreamResource pVideoStream(pFormat, pOutFormat->video_codec);
   if (pVideoStream == NULL)
   {
      log_error("Unable to create video stream.");
      return false;
   }

   /**
    * allow changing of:
    *    dia_size/
    */
   AVCodecContext* pCodecContext = pVideoStream->codec;
   if (!setAvCodecOptions(pCodecContext))
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

   if (av_set_parameters(pFormat, NULL) < 0)
   {
      log_error("Invalid output format parameters.");
      return false;
   }
   if (!open_video(pFormat, pVideoStream))
   {
      log_error("Unable to initialize video stream.");
      return false;
   }
   if (url_fopen(&pFormat->pb, filename.c_str(), URL_WRONLY) < 0)
   {
      log_error("Could not open the output file. Ensure the destination directory is writable.");
      return false;
   }
   av_write_header(pFormat);

   // calculate time interval
   if ((framerate < getFrameRate(pController)) && (mpProgress != NULL))
   {
      mpProgress->updateProgress("The selected output frame rate may not encode all the frames in the movie.  "
                                 "Frames may be dropped.", 0, WARNING);
   }

   // do not use the boost::rational<int> overloaded operator '/' since it truncates type double to int
   double interval = pController->getIntervalMultiplier() * framerate.denominator() / framerate.numerator();

   // export the frames
   AVFrame* pTmpPicture = alloc_picture(PIX_FMT_RGBA32, pCodecContext->width, pCodecContext->height);
   if (pTmpPicture == NULL)
   {
      QString msg("Unable to allocate frame buffer of size %1 x %2");
      log_error(msg.arg(pCodecContext->width).arg(pCodecContext->height).toStdString());
      return false;
   }
   QImage image(pTmpPicture->data[0], pCodecContext->width, pCodecContext->height, QImage::Format_ARGB32);

   // For frame id based animation, each band of the data set fills one second of animation. 
   // If the requested frame rate for export is 15 fps, then each band is replicated 15 times. The execution
   // loop uses a pseudo time value, video_pts, to walk through the animation. The interval between 
   // exported frames is the inverse of the frame rate, e.g., for 15 fps the interval is 0.06667.
   // To fully export the last requested frame, we need to add just under an extra pseudo second to
   // the end time - stopExport. If we added a full extra second, we would export one video frame of the band past
   // the last requested frame - could cause crash if the last request frame was the last band.
   if (eType == FRAME_ID)
   {
      stopExport += 0.99;
   }

   bool drawClassMarkings = fullResolution &&
      Service<ConfigurationSettings>()->getSettingDisplayClassificationMarkings();
   QString classText;
   QFont classFont;
   QColor classColor;
   QPoint classPositionTop;
   QPoint classPositionBottom;
   const int shadowOffset = 2;
   if (drawClassMarkings)
   {
      const int topMargin = 1;
      const int bottomMargin = 4;
      const int leftMargin = 5;
      const int rightMargin = 5;

      classText = QString::fromStdString(pView->getClassificationText());
      classColor = COLORTYPE_TO_QCOLOR(pView->getClassificationColor());
      pView->getClassificationFont(classFont);
      QFontMetrics fontMetrics(classFont);
      int classWidth = fontMetrics.width(classText);
      int classHeight = fontMetrics.ascent();
      int topX((pCodecContext->width / 2) - (classWidth / 2) + shadowOffset);
      int bottomX((pCodecContext->width / 2) - (classWidth / 2));
      // determine the classification position
      switch (pView->getClassificationPosition())
      {
      case TOP_LEFT_BOTTOM_LEFT:
         topX = leftMargin + shadowOffset;
         bottomX = leftMargin + shadowOffset;
         break;
      case TOP_LEFT_BOTTOM_RIGHT:
         topX = leftMargin + shadowOffset;
         bottomX = pCodecContext->width - rightMargin - classWidth + shadowOffset;
         break;
      case TOP_RIGHT_BOTTOM_LEFT:
         topX = pCodecContext->width - rightMargin - classWidth + shadowOffset;
         bottomX = leftMargin + shadowOffset;
         break;
      case TOP_RIGHT_BOTTOM_RIGHT:
         topX = pCodecContext->width - rightMargin - classWidth + shadowOffset;
         bottomX = pCodecContext->width - rightMargin - classWidth + shadowOffset;
         break;
      default:
         // nothing to do, markings centered by default
         break;
      }
      int screenY = 1 + classHeight;
      classPositionTop = QPoint(topX, 1 + classHeight);
      classPositionBottom = QPoint(bottomX, pCodecContext->height - 1);
   }

   // make sure controller is not running prior to export. Save current state and restore after export finished
   AnimationState savedAnimationState = pController->getAnimationState();
   pController->setAnimationState(STOP);

   for (double video_pts = startExport; video_pts <= stopExport; video_pts += interval)
   {
      if (isAborted() == true)
      {
         // reset resources to close output file so it can be deleted
         pVideoStream = AvStreamResource();
         pFormat = AvFormatContextResource(NULL);
         mpPicture = NULL;
         free(mpVideoOutbuf);
         mpVideoOutbuf = NULL;
         remove(filename.c_str());

         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Export aborted", 0, ABORT);
         }
         pController->setAnimationState(savedAnimationState);
         mpStep->finalize(Message::Abort);
         return false;
      }

      // generate the next frame
      pController->setCurrentFrame(video_pts);
      if (mpProgress != NULL)
      {
         double progressValue = (video_pts - startExport) / (stopExport - startExport) * 100.0;
         mpProgress->updateProgress("Saving movie", static_cast<int>(progressValue), NORMAL);
      }
      if (fullResolution)
      {
         QSize totalSize(pCodecContext->width, pCodecContext->height);
         QSize subImageSize(pView->getWidget()->width(), pView->getWidget()->height());
         // Make sure that the sub-image and the main image have the same aspect ratio to
         // minimize wasted tile space
         if (pCodecContext->width > pCodecContext->height)
         {
            subImageSize.setWidth(
               totalSize.width() / static_cast<float>(totalSize.height()) * subImageSize.height() + 0.5);
         }
         else
         {
            subImageSize.setHeight(
               totalSize.height() / static_cast<float>(totalSize.width()) * subImageSize.width() + 0.5);
         }
         // Remove pan and zoom limits so they don't interfere with the subimage grab
         // and restore them when done
         SpatialDataView* pSdv = dynamic_cast<SpatialDataView*>(pView);
         PanLimitType panLimit;
         if (pSdv != NULL)
         {
            panLimit = pSdv->getPanLimit();
            pSdv->setPanLimit(NO_LIMIT);
         }
         View::SubImageIterator* pSubImage(pView->getSubImageIterator(totalSize, subImageSize));
         if (pSubImage == NULL)
         {
            if (pSdv != NULL)
            {
               pSdv->setPanLimit(panLimit);
            }
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress("Unable to render full scale data", 0, ERRORS);
            }
            pController->setAnimationState(savedAnimationState);
            mpStep->finalize(Message::Failure);
            return false;
         }
         QPainter outPainter(&image);
         QPoint origin(0, image.height() - subImageSize.height());
         while (pSubImage->hasNext())
         {
            QImage subImage;
            if (!pSubImage->next(subImage))
            {
               if (pSdv != NULL)
               {
                  pSdv->setPanLimit(panLimit);
               }
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress("An error occurred when generating the image", 0, ERRORS);
               }
               pController->setAnimationState(savedAnimationState);
               mpStep->finalize(Message::Failure);
               delete pSubImage;
               return false;
            }
            // copy this subimage to the output buffer
            outPainter.drawImage(origin, subImage);

            int newX = origin.x() + subImage.width();
            int newY = origin.y();
            if (newX >= totalSize.width())
            {
               newY -= subImage.height();
               newX = 0;
            }
            origin = QPoint(newX, newY);
         }
         delete pSubImage;
         if (drawClassMarkings)
         {
            outPainter.setFont(classFont);
            outPainter.setPen(Qt::black);
            outPainter.drawText(classPositionTop, classText);
            outPainter.drawText(classPositionBottom, classText);
            outPainter.setPen(classColor);
            outPainter.drawText(classPositionTop - QPoint(shadowOffset, shadowOffset), classText);
            outPainter.drawText(classPositionBottom - QPoint(shadowOffset, shadowOffset), classText);
         }
         if (pSdv != NULL)
         {
            pSdv->setPanLimit(panLimit);
         }
      }
      else
      {
         pView->getCurrentImage(image);
      }
      img_convert(reinterpret_cast<AVPicture*>(mpPicture),
         pCodecContext->pix_fmt,
         reinterpret_cast<AVPicture*>(pTmpPicture),
         PIX_FMT_RGBA32,
         pCodecContext->width,
         pCodecContext->height);
      if (!write_video_frame(pFormat, pVideoStream))
      {
         // reset resources to close output file so it can be deleted
         pVideoStream = AvStreamResource();
         pFormat = AvFormatContextResource(NULL);
         mpPicture = NULL;
         free(mpVideoOutbuf);
         mpVideoOutbuf = NULL;
         remove(filename.c_str());
         string msg = "Can't write frame.";
         log_error(msg.c_str());
         pController->setAnimationState(savedAnimationState);
         return false;
      }
   }
   for (int frame = 0; frame < pCodecContext->delay; ++frame)
   {
      write_video_frame(pFormat, pVideoStream);
   }

   av_write_trailer(pFormat);

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Finished saving movie", 100, NORMAL);
   }

   free(mpVideoOutbuf);
   mpVideoOutbuf = NULL;
   pController->setAnimationState(savedAnimationState);
   mpStep->finalize(Message::Success);
   return true;
}

ValidationResultType MovieExporter::validate(const PlugInArgList* pArgList, string& errorMessage) const
{
   /* validate some arguments */
   ValidationResultType result = ExporterShell::validate(pArgList, errorMessage);
   if (result != VALIDATE_SUCCESS)
   {
      return result;
   }
   View* pView = pArgList->getPlugInArgValue<View>(Exporter::ExportItemArg());
   if (pView == NULL)
   {
      errorMessage = "No view specified. Nothing to export.";
      return VALIDATE_FAILURE;
   }
   AnimationController* pController = pView->getAnimationController();
   if (pController == NULL)
   {
      errorMessage = "No animation is associated with this view. Nothing to export.";
      return VALIDATE_FAILURE;
   }

   /* validate the framerate */
   boost::rational<int> expectedFrameRate;

   // first, get the framerate from the arg list
   // next, try the option widget
   // next, get from the animation controller
   // finally, default to the config settings
   int framerateNum = 0;
   int framerateDen = 0;
   if (pArgList->getPlugInArgValue("Framerate Numerator", framerateNum) &&
      pArgList->getPlugInArgValue("Framerate Denominator", framerateDen))
   {
      try
      {
         expectedFrameRate.assign(framerateNum, framerateDen);
      }
      catch (const boost::bad_rational&)
      {
         errorMessage = "Invalid framerate specified";
         return VALIDATE_FAILURE;
      }
   }
   if (expectedFrameRate == 0)
   {
      if (mpOptionWidget.get() != NULL)
      {
         FramerateWidget* pFramerateWidget = mpOptionWidget->getFramerateWidget();
         VERIFYRV(pFramerateWidget != NULL, VALIDATE_FAILURE);

         expectedFrameRate = pFramerateWidget->getFramerate();
      }
      if (expectedFrameRate == 0)
      {
         expectedFrameRate = getFrameRate(pController);
      }
      if (expectedFrameRate == 0)
      {
         errorMessage = "No framerate specified";
         return VALIDATE_FAILURE;
      }
   }

   boost::rational<int> actualFrameRate = convertToValidFrameRate(expectedFrameRate);
   if (actualFrameRate != 0)
   {
      if (actualFrameRate < getFrameRate(pController))
      {
         errorMessage = "The selected output frame rate may not encode all the frames in the movie.  "
                        "Frames may be dropped.";
         result = VALIDATE_INFO;
      }
      if (actualFrameRate != expectedFrameRate)
      {
         QString msg = QString("The selected animation frame rate (%1/%2 fps) can not be represented in the "
            "selected movie format. A frame rate of %3/%4 fps is being used instead.")
            .arg(expectedFrameRate.numerator())
            .arg(expectedFrameRate.denominator())
            .arg(actualFrameRate.numerator())
            .arg(actualFrameRate.denominator());
         if (!errorMessage.empty())
         {
            errorMessage += "\n\n";
         }
         errorMessage += msg.toStdString();
         result = VALIDATE_INFO;
      }
   }

   // check the frame resolution
   int resolutionX(-1);
   int resolutionY(-1);
   bool fullResolution(false);
   bool fixedResolution(false);
   if (!pArgList->getPlugInArgValue("Resolution X", resolutionX) ||
       !pArgList->getPlugInArgValue("Resolution Y", resolutionY))
   {
      if (mpOptionWidget.get() != NULL)
      {
         ViewResolutionWidget* pResolutionWidget = mpOptionWidget->getResolutionWidget();
         VERIFYRV(pResolutionWidget != NULL, VALIDATE_FAILURE);

         switch(pResolutionWidget->getResolutionType())
         {
         case OptionsMovieExporter::VIEW_RESOLUTION:
            resolutionX = -1;
            resolutionY = -1;
            fullResolution = false;
            break;
         case OptionsMovieExporter::FULL_RESOLUTION:
            resolutionX = -1;
            resolutionY = -1;
            fullResolution = true;
            break;
         case OptionsMovieExporter::FIXED_RESOLUTION:
         {
            QSize resolution = pResolutionWidget->getResolution();
            resolutionX = resolution.width();
            resolutionY = resolution.height();
            fullResolution = false;
            fixedResolution = true;
            break;
         }
         default:
            break; // nothing
         }
      }
      else
      {
         switch(StringUtilities::fromXmlString<OptionsMovieExporter::ResolutionType>(
            OptionsMovieExporter::getSettingResolutionType()))
         {
         case OptionsMovieExporter::VIEW_RESOLUTION:
            resolutionX = -1;
            resolutionY = -1;
            fullResolution = false;
            break;
         case OptionsMovieExporter::FULL_RESOLUTION:
            resolutionX = -1;
            resolutionY = -1;
            fullResolution = true;
            break;
         case OptionsMovieExporter::FIXED_RESOLUTION:
            resolutionX = OptionsMovieExporter::getSettingWidth();
            resolutionY = OptionsMovieExporter::getSettingHeight();
            fullResolution = false;
            fixedResolution = true;
            break;
         default:
            break; // nothing
         }
      }
   }

   if (resolutionX <= 0 || resolutionY <= 0)
   {
      QWidget* pWidget = pView->getWidget();
      if (pWidget != NULL)
      {
         resolutionX = pWidget->width();
         resolutionY = pWidget->height();
         if (fullResolution)
         {
            PerspectiveView* pPersp = dynamic_cast<PerspectiveView*>(pView);
            if (pPersp != NULL && pPersp->getZoomPercentage() > 100.)
            {
               fullResolution = false;
               if (mpProgress != NULL)
               {
                  errorMessage += "Full resolution export will be smaller than "
                     "view export, export resolution will be changed to current view size.\n";
                  if (result == VALIDATE_SUCCESS)
                  {
                     result = VALIDATE_INFO;
                  }
               }
            }
            else
            {
               // translate to data coordinate
               double x1 = 0.0;
               double y1 = 0.0;
               double x2 = 0.0;
               double y2 = 0.0;
               pView->translateScreenToWorld(0.0, 0.0, x1, y1);
               pView->translateScreenToWorld(resolutionX, resolutionY, x2, y2);
               resolutionX = (x2 > x1) ? (x2 - x1 + 0.5) : (x1 - x2 + 0.5);
               resolutionY = (y2 > y1) ? (y2 - y1 + 0.5) : (y1 - y2 + 0.5);
            }
         }
      }
      else
      {
         errorMessage += "Can't determine output resolution.\n";
         return VALIDATE_FAILURE;
      }
   }

   int oldResX = resolutionX;
   int oldResY = resolutionY;
   if (!convertToValidResolution(resolutionX, resolutionY) ||
         (fixedResolution && (resolutionX != oldResX || resolutionY != oldResY)))
   {
      if (mpOptionWidget.get() != NULL)
      {
         ViewResolutionWidget* pResolutionWidget = mpOptionWidget->getResolutionWidget();
         VERIFYRV(pResolutionWidget != NULL, VALIDATE_FAILURE);
         pResolutionWidget->setResolution(QSize(resolutionX, resolutionY), pResolutionWidget->getResolutionType());
      }
      errorMessage +=
         "The export resolution does not meet the requirements of the the selected CODEC and will be adjusted.\n";
      if (result != VALIDATE_FAILURE)
      {
         result = VALIDATE_INFO;
      }
   }

   return result;
}

QWidget* MovieExporter::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (mpOptionWidget.get() == NULL)
   {
      // Create the options widget
      MovieExportOptionsWidget* pOptionWidget = new MovieExportOptionsWidget();
      mpOptionWidget = auto_ptr<MovieExportOptionsWidget>(pOptionWidget);
      VERIFYRV(mpOptionWidget.get() != NULL, NULL);

      // Resolution
      ViewResolutionWidget* pResolutionWidget = mpOptionWidget->getResolutionWidget();
      VERIFYRV(pResolutionWidget != NULL, NULL);

      QSize resolution(OptionsMovieExporter::getSettingWidth(), OptionsMovieExporter::getSettingHeight());
      pResolutionWidget->setResolution(resolution,
         StringUtilities::fromXmlString<OptionsMovieExporter::ResolutionType>(
            OptionsMovieExporter::getSettingResolutionType()));

      // Bitrate
      BitrateWidget* pBitrateWidget = mpOptionWidget->getBitrateWidget();
      VERIFYRV(pBitrateWidget != NULL, NULL);

      pBitrateWidget->setBitrate(OptionsMovieExporter::getSettingBitrate());

      // Framerates
      AVOutputFormat* pOutFormat = getOutputFormat();
      VERIFYRV(pOutFormat, NULL);
      AVCodec* pCodec = avcodec_find_encoder(pOutFormat->video_codec);
      VERIFYRV(pCodec, NULL);
      if (pCodec->supported_framerates != NULL)
      {
         vector<boost::rational<int> > frameRates;
         try
         {
            for (int idx = 0; ; ++idx)
            {
               boost::rational<int> frameRate(pCodec->supported_framerates[idx].num,
                  pCodec->supported_framerates[idx].den);
               if (frameRate == 0)
               {
                  break;
               }

               frameRates.push_back(frameRate);
            }
         }
         catch (const boost::bad_rational&)
         {
            // intentionally left blank
         }

         FramerateWidget* pFramerateWidget = mpOptionWidget->getFramerateWidget();
         VERIFYRV(pFramerateWidget != NULL, NULL);

         pFramerateWidget->setFramerates(frameRates);
      }

      View* pView = pInArgList->getPlugInArgValue<View>(Exporter::ExportItemArg());
      if (pView != NULL)
      {
         AnimationController* pController = pView->getAnimationController();
         if (pController != NULL)
         {
            // Subset
            AnimationFrameSubsetWidget* pSubsetWidget = mpOptionWidget->getSubsetWidget();
            VERIFYRV(pSubsetWidget != NULL, NULL);

            pSubsetWidget->setFrames(pController);

            double start = pController->getStartFrame();
            double stop = pController->getStopFrame();

            if (pController->getBumpersEnabled())
            {
               // Need to change start and stop to playback bumpers
               start = pController->getStartBumper();
               stop = pController->getStopBumper();
            }

            pSubsetWidget->setStartFrame(start);
            pSubsetWidget->setStopFrame(stop);

            // Framerate
            rational<int> frameRate = getFrameRate(pController);
            frameRate = convertToValidFrameRate(frameRate);

            FramerateWidget* pFramerateWidget = mpOptionWidget->getFramerateWidget();
            VERIFYRV(pFramerateWidget != NULL, NULL);

            pFramerateWidget->setFramerate(frameRate);
         }
      }

      // Advanced options
      AdvancedOptionsWidget* pAdvancedWidget = mpOptionWidget->getAdvancedWidget();
      VERIFYRV(pAdvancedWidget != NULL, NULL);

      pAdvancedWidget->setMeMethod(OptionsMovieExporter::getSettingMeMethod());
      pAdvancedWidget->setGopSize(OptionsMovieExporter::getSettingGopSize());
      pAdvancedWidget->setQCompress(OptionsMovieExporter::getSettingQCompress());
      pAdvancedWidget->setQBlur(OptionsMovieExporter::getSettingQBlur());
      pAdvancedWidget->setQMinimum(OptionsMovieExporter::getSettingQMinimum());
      pAdvancedWidget->setQMaximum(OptionsMovieExporter::getSettingQMaximum());
      pAdvancedWidget->setQDiffMaximum(OptionsMovieExporter::getSettingQDiffMaximum());
      pAdvancedWidget->setMaxBFrames(OptionsMovieExporter::getSettingMaxBFrames());
      pAdvancedWidget->setBQuantFactor(OptionsMovieExporter::getSettingBQuantFactor());
      pAdvancedWidget->setBQuantOffset(OptionsMovieExporter::getSettingBQuantOffset());
      pAdvancedWidget->setDiaSize(OptionsMovieExporter::getSettingDiaSize());
      pAdvancedWidget->setOutputBufferSize(OptionsMovieExporter::getSettingOutputBufferSize());
      pAdvancedWidget->setFlags(OptionsMovieExporter::getSettingFlags());
   }

   return mpOptionWidget.get();
}

bool MovieExporter::setAvCodecOptions(AVCodecContext* pContext)
{
   if (pContext == NULL)
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
   mVideoOutbufSize = OptionsMovieExporter::getSettingOutputBufferSize();
   int newFlags = OptionsMovieExporter::getSettingFlags();
   if (mpOptionWidget.get() != NULL)
   {
      AdvancedOptionsWidget* pAdvancedWidget = mpOptionWidget->getAdvancedWidget();
      VERIFY(pAdvancedWidget != NULL);

      meMethod = pAdvancedWidget->getMeMethod();
      pContext->gop_size = pAdvancedWidget->getGopSize();
      pContext->qcompress = pAdvancedWidget->getQCompress();
      pContext->qblur = pAdvancedWidget->getQBlur();
      pContext->qmin = pAdvancedWidget->getQMinimum();
      pContext->qmax = pAdvancedWidget->getQMaximum();
      pContext->max_qdiff = pAdvancedWidget->getQDiffMaximum();
      pContext->max_b_frames = pAdvancedWidget->getMaxBFrames();
      pContext->b_quant_factor = pAdvancedWidget->getBQuantFactor();
      pContext->b_quant_offset = pAdvancedWidget->getBQuantOffset();
      pContext->dia_size = pAdvancedWidget->getDiaSize();
      mVideoOutbufSize = pAdvancedWidget->getOutputBufferSize();
      newFlags = pAdvancedWidget->getFlags();
   }
   pContext->me_method = StringUtilities::fromXmlString<Motion_Est_ID>(meMethod);
   pContext->flags |= newFlags;
   if (pContext->flags & CODEC_FLAG_TRELLIS_QUANT)
   {
      pContext->trellis = 1;
   }
   else
   {
      pContext->trellis = 0;
   }

   return true;
}

bool MovieExporter::open_video(AVFormatContext* pFormat, AVStream* pVideoStream)
{
   VERIFY(pFormat && pVideoStream);
   AVCodecContext* pCodecContext = pVideoStream->codec;
   AVCodec* pCodec = avcodec_find_encoder(pCodecContext->codec_id);
   if (pCodec == NULL)
   {
      return false;
   }

   pCodec = avcodec_find_encoder(pCodecContext->codec_id);
   if (pCodec == NULL)
   {
      return false;
   }

   if (avcodec_open(pCodecContext, pCodec) < 0)
   {
      return false;
   }

   if (!(pFormat->oformat->flags & AVFMT_RAWPICTURE))
   {
      if (mVideoOutbufSize < FF_MIN_BUFFER_SIZE)
      {
         mVideoOutbufSize = avpicture_get_size(pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height);
         if (mVideoOutbufSize <= 0)
         {
            return false;
         }
      }
      mpVideoOutbuf = reinterpret_cast<uint8_t*>(malloc(mVideoOutbufSize));
      if (mpVideoOutbuf == NULL)
      {
         return false;
      }
   }

   /* allocate the encoded raw picture */
   mpPicture = alloc_picture(pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height);
   if (mpPicture == NULL)
   {
      return false;
   }

   return true;
}

AVFrame* MovieExporter::alloc_picture(int pixFmt, int width, int height)
{
   AVFrame* pPicture = avcodec_alloc_frame();
   VERIFYRV(pPicture, NULL);
   int size = avpicture_get_size(pixFmt, width, height);
   uint8_t* pPictureBuf = reinterpret_cast<uint8_t*>(malloc(size));
   if (pPictureBuf == NULL)
   {
      av_free(pPicture);
      return NULL;
   }
   avpicture_fill(reinterpret_cast<AVPicture*>(pPicture), pPictureBuf, pixFmt, width, height);
   return pPicture;
}

bool MovieExporter::write_video_frame(AVFormatContext* pFormat, AVStream* pVideoStream)
{
   VERIFY(pFormat && pVideoStream);
   int out_size = 0;
   int ret = 0;
   AVCodecContext* pCodecContext = pVideoStream->codec;

   if (pFormat->oformat->flags & AVFMT_RAWPICTURE)
   {
      /* raw video case. The API will change slightly in the near
      future for that */
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
      /* if zero size, it means the image was buffered; if negative, it means error */
      if (out_size > 0)
      {
         AVPacket pkt;
         av_init_packet(&pkt);

         pkt.pts = av_rescale_q(pCodecContext->coded_frame->pts, pCodecContext->time_base, pVideoStream->time_base);
         if (pCodecContext->coded_frame->key_frame)
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
         ret = out_size;
      }
   }
   if (ret != 0)
   {
      return false;
   }
   ++mFrameCount;
   return true;
}

boost::rational<int> MovieExporter::getFrameRate(const AnimationController* pController) const
{
   VERIFYRV(pController != NULL, boost::rational<int>());

   boost::rational<int> minFrameRate = pController->getMinimumFrameRate();
   double frameSpeed = pController->getIntervalMultiplier();

   boost::rational<int> frameRate;
   if (frameSpeed <= 0.0)
   {
      frameRate.assign(0, 1);
   }
   else if (frameSpeed < 1.0)
   {
      frameRate.assign(minFrameRate.numerator(), static_cast<int>(minFrameRate.denominator() / frameSpeed));
   }
   else
   {
      frameRate.assign(static_cast<int>(minFrameRate.numerator() * frameSpeed), minFrameRate.denominator());
   }

   return frameRate;
}

boost::rational<int> MovieExporter::convertToValidFrameRate(const boost::rational<int>& frameRate) const
{
   boost::rational<int> validFrameRate;
   try
   {
      AVOutputFormat* pOutFormat = getOutputFormat();
      VERIFY(pOutFormat != NULL);
      AVCodec* pCodec = avcodec_find_encoder(pOutFormat->video_codec);
      VERIFY(pCodec != NULL);

      if (pCodec->supported_framerates != NULL)
      {
         for (int idx = 0; ; ++idx)
         {
            boost::rational<int> supportedFrameRate(pCodec->supported_framerates[idx].num,
                                                    pCodec->supported_framerates[idx].den);
            if (supportedFrameRate == 0)
            {
               break;
            }

            if ((supportedFrameRate == frameRate) || // The frame rate matches a supported frame rate
               (validFrameRate == 0) ||              // A valid frame rate has not yet been set
               // The difference between supported FR and the FR is closer than the valid FR difference
               ((supportedFrameRate - frameRate) < (validFrameRate - frameRate)) ||
               // The supported FR is greater than the FR and the valid FR is less than the FR
               ((validFrameRate - frameRate) < 0 && (supportedFrameRate - frameRate) > 0))
            {
               validFrameRate = supportedFrameRate;
            }
         }
      }
      else
      {
         validFrameRate = frameRate;
      }
   }
   catch (const boost::bad_rational&)
   {
      // Intentionally left blank
   }

   return validFrameRate;
}

bool MovieExporter::convertToValidResolution(int& resolutionX, int& resolutionY) const
{
   if ((resolutionX % 2) != 0)
   {
      ++resolutionX;
   }
   if ((resolutionY % 2) != 0)
   {
      ++resolutionY;
   }
   return true;
}
