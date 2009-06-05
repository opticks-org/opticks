/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "Filename.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PostScriptExporter.h"
#include "Progress.h"
#include "View.h"

#include <QtGui/QImage>
#include <cstdio>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPictures, PostScriptExporter);

PostScriptExporter::PostScriptExporter() : mpOutFile(NULL), mWidth(72), mPos(0), mTuple(0), mCount(0)
{
   setName("PostScript Exporter");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("PostScript Exporter");
   setDescription("PostScript file format exporter");
   setExtensions("PostScript Files (*.ps *.eps)");
   setSubtype(TypeConverter::toString<View>());
   setDescriptorId("{E0FA4761-9FEB-4c77-A4F0-A449E1031C87}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PostScriptExporter::~PostScriptExporter()
{}

bool PostScriptExporter::getInputSpecification(PlugInArgList *&pArgList)
{
   bool success = ExporterShell::getInputSpecification(pArgList);
   success = success && pArgList->addArg<View>(ExportItemArg());
   success = success && pArgList->addArg<unsigned int>("Output Width");
   success = success && pArgList->addArg<unsigned int>("Output Height");
   return success;
}

bool PostScriptExporter::execute(PlugInArgList *pInArgList, PlugInArgList *pOutArgList)
{
   StepResource pStep("Execute PostScript Exporter", "app", "5BE7D170-BFB5-43C9-A980-06C8C376D558");

   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
   View* pView = pInArgList->getPlugInArgValue<View>(ExportItemArg());
   if (pView == NULL)
   {
      string msg = "No view specified.";
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, msg);
      return false;
   }
   string outPath;
   string viewName = pView->getName();
   pStep->addProperty("View Name", viewName);

   FileDescriptor* pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(ExportDescriptorArg());
   if (pFileDescriptor != NULL)
   {
      outPath = pFileDescriptor->getFilename().getFullPathAndName();
   }
   if (outPath.empty())
   {
      string msg = "The destination path is invalid.";
      if (pProgress)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, msg);
      return false;
   }
   pStep->addProperty("Filename", outPath);

   QSize outputSize;
   unsigned int outputWidth;
   unsigned int outputHeight;
   if (pInArgList->getPlugInArgValue("Output Width", outputWidth) &&
      pInArgList->getPlugInArgValue("Output Height", outputHeight))
   {
      pStep->addProperty("Output Width", outputWidth);
      pStep->addProperty("Output Height", outputHeight);
      outputSize = QSize(outputWidth, outputHeight);
   }

   // Get the current image from the viewer
   FileResource outFile(outPath.c_str(), "wb");
   VERIFY(outFile.get() != NULL);
   mpOutFile = outFile.get();
   if (outputSize == QSize())
   {
      QImage image;
      pView->getCurrentImage(image);
      writePostScriptHeader(outPath, QPoint(0, 0), image.size());
      writeImageSegment(image, QPoint(0, 0));
   }
   else
   {
      writePostScriptHeader(outPath, QPoint(0, 0), outputSize);
      QSize subImageSize(512, 512);
      QPoint origin(0, 0);
      int segment = 0;
      View::SubImageIterator* pSubImage = pView->getSubImageIterator(outputSize, subImageSize);
      int totalX;
      int totalTiles;
      pSubImage->count(totalX, totalTiles);
      totalTiles *= totalX;
      while (pSubImage->hasNext())
      {
         QImage subImage;
         if (!pSubImage->next(subImage))
         {
            break;
         }

         writeImageSegment(subImage, origin);

         int newX = origin.x() + subImage.width();
         int newY = origin.y();
         if (newX >= outputSize.width())
         {
            newY += subImage.height();
            newX = 0;
         }
         origin = QPoint(newX, newY);
         if (pProgress != NULL)
         {
            int x;
            int y;
            pSubImage->location(x, y);
            int tileNumber = y * totalX + x;
            QString msg = QString("Processing sub-image %1 of %2...").arg(y * totalX + x + 1).arg(totalTiles);
            pProgress->updateProgress(msg.toStdString(), 100 * tileNumber / totalTiles - 1, NORMAL);
         }
      }
      delete pSubImage;
   }
   writePostScriptFooter();
   if (pProgress != NULL)
   {
      pProgress->updateProgress("Saving product...", 99, NORMAL);
   }
   mpOutFile = NULL;

   pStep->finalize(Message::Success);
   return true;
}

void PostScriptExporter::writePostScriptHeader(const string &fname,
                                               const QPoint &offset,
                                               const QSize &size)
{
   fprintf(mpOutFile,
                "%%!PS-Adobe-3.0 EPSF-3.0\n" \
                "%%%%Title: (%s)\n" \
                "%%%%Creator: (%s)\n" \
                "%%%%LanguageLevel: 2\n" \
                "%%%%BoundingBox: %i %i %i %i\n" \
                "%%%%DocumentData: Clean7Bit\n" \
                "%%%%EndComments\n", fname.c_str(), APP_NAME, 
                                   offset.x(), offset.y(),
                                   size.width() + offset.x(), size.height() + offset.y());

}

void PostScriptExporter::writeImageSegment(const QImage &image,
                                           const QPoint &offset,
                                           double scale)
{
   int w = image.width();
   int h = image.height();
   int scaledW = w * scale;
   int scaledH = h * scale;
   int x = offset.x() * scale;
   int y = offset.y() * scale;
   fprintf(mpOutFile,
                "gsave\n" \
                "%i %i translate %i %i scale\n" \
                "/DeviceRGB setcolorspace\n" \
                "<<\n" \
                "  /ImageType 1\n" \
                "  /Width %i /Height %i\n" \
                "  /BitsPerComponent 8\n" \
                "  /ImageMatrix [%i 0 0 -%i 0 %i]\n" \
                "  /Decode [0 1 0 1 0 1]\n" \
                "  /DataSource currentfile /ASCII85Decode filter\n" \
                ">>\n" \
                "image\n", x, y,
                           scaledW, scaledH,
                           w, h,
                           w, h, h);
   // write out image data
   init85();
   for (int i = 0; i < image.height(); ++i)
   {
      for (int j = 0; j < image.width(); ++j)
      {
         QRgb pel = image.pixel(j, i);
         put85(qRed(pel));
         put85(qGreen(pel));
         put85(qBlue(pel));
      }
   }
   cleanup85();

   fprintf(mpOutFile, "grestore\n");
}

void PostScriptExporter::writePostScriptFooter()
{
   fprintf(mpOutFile, "showpage\n%%%%EOF");
}

void PostScriptExporter::init85()
{
   mPos = 2;
}

void PostScriptExporter::put85(unsigned c)
{
   switch (mCount++)
   {
   case 0:
      mTuple |= (c << 24);
      break;
   case 1:
      mTuple |= (c << 16);
      break;
   case 2:
      mTuple |= (c << 8);
      break;
   case 3:
      mTuple |= c;
      if (mTuple == 0)
      {
         fputc('z', mpOutFile);
         if (static_cast<unsigned int>(mPos++) >= mWidth)
         {
            mPos = 0;
            fputc('\n', mpOutFile);
         }
      }
      else
      {
         encode(mTuple, mCount);
      }
      mTuple = 0;
      mCount = 0;
      break;
   default:
      break;
   }
}

void PostScriptExporter::cleanup85()
{
   if (mCount > 0)
   {
      encode(mTuple, mCount);
   }
   if (static_cast<unsigned int>(mPos) + 2 > mWidth)
   {
      fputc('\n', mpOutFile);
   }
   fprintf(mpOutFile, "\n~>\n");
}

void PostScriptExporter::encode(unsigned long tuple, int count)
{
   char pBuf[5];
   char* s = pBuf;
   int i = 5;
   do
   {
      *s++ = tuple % 85;
      tuple /= 85;
   }
   while (--i > 0);
   i = count;
   do
   {
      fputc(*--s + '!', mpOutFile);
      if (static_cast<unsigned int>(mPos++) >= mWidth)
      {
         mPos = 0;
         fputc('\n', mpOutFile);
      }
   }
   while (i-- > 0);
}
