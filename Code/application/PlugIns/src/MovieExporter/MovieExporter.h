/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOVIEEXPORTER_H
#define MOVIEEXPORTER_H

#include "ExporterShell.h"
#include "MessageLogResource.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

#pragma warning(push, 1)
#pragma warning(disable: 4244)
#include "avformat.h"
#include "avutil.h"
#pragma warning(pop)

#include <memory>

class OptionsMovieExporter;
class Progress;
class QWidget;

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

   bool setBatch()
   {
      mIsBatch = true;
      return true;
   }
   bool setInteractive()
   {
      mIsBatch = false;
      return true;
   }
   bool hasAbort() { return true; }
   bool abort() { return (mAbort = true); }

   bool getInputSpecification(PlugInArgList*& pInArgList);
   bool getOutputSpecification(PlugInArgList*& pOutArgList);
   bool execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList);
   ValidationResultType validate(const PlugInArgList* pArgList, std::string& errorMessage) const;
   QWidget* getExportOptionsWidget(const PlugInArgList *pInArgList);

protected:
   bool setAvCodecOptions(AVCodecContext *pContext, PlugInArgList *pInArgList);
   AVStream *add_video_stream(AVFormatContext *pFormatContext, CodecID codec_id);
   bool open_video(AVFormatContext *pFormat, AVStream *pVideoStream);
   AVFrame *alloc_picture(int pixFmt, int width, int height);
   bool write_video_frame(AVFormatContext *pFormat, AVStream *pVideoStream);
   void close_video(AVFormatContext *pFormat, AVStream *pVideoStream);

   virtual AVOutputFormat *getOutputFormat() const = 0;

   Service<PlugInManagerServices> mpPlugInManager;
   Service<UtilityServices> mpUtility;

private:
   void log_error(const std::string &msg);

   bool mIsBatch;
   bool mAbort;
   Progress *mpProgress;
   StepResource mpStep;
   AVFrame *mpPicture;
   uint8_t *mpVideoOutbuf;
   int mFrameCount;
   int mVideoOutbufSize;
   
   std::auto_ptr<OptionsMovieExporter> mpOptionWidget;
};

#endif
