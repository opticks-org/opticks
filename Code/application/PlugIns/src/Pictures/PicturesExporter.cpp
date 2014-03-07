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
#include "FileDescriptor.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "PicturesExporter.h"
#include "PlotView.h"
#include "PlotWidget.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "ProductView.h"
#include "Progress.h"
#include "View.h"

#include <stdio.h>

#include <QtGui/QPainter>
#include <QtGui/QWidget>

using namespace std;

PicturesExporter::PicturesExporter(PicturesDetails* pDetails) :
   mpProgress(NULL),
   mpItem(NULL),
   mpDetails(pDetails)
{
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright("Copyright 2007, BATC");
   setExtensions(pDetails->extensions());
   allowMultipleInstances(true);
}

PicturesExporter::~PicturesExporter()
{
}

bool PicturesExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   bool success = ExporterShell::getInputSpecification(pArgList);
   success = success && pArgList->addArg<unsigned int>("Output Width", "Width of the exported image.");
   success = success && pArgList->addArg<unsigned int>("Output Height", "Height of the exported image.");
   return success;
}

bool PicturesExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PicturesExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   string exporterName = getName();

   StepResource pStep(string("Execute ") + exporterName, "app", "5075FE69-1972-4b23-BE35-EF185DE35312");
   // Get the values from the plug-in args
   if (extractInputArgs(pInArgList) == false)
   {
      if (mMessage.empty())
      {
         mMessage = "Unable to extract input arguments.";
      }
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   Progress* pProgress = mpProgress;

   // Add the input and output values to the message log
   pStep->addProperty("Export Name", mpItem->getName());

   pStep->addProperty("Filename", mOutPath);

   // get the output height and width
   //
   QSize outputSize;
   unsigned int outputWidth = 0;
   unsigned int outputHeight = 0;
   if (pInArgList->getPlugInArgValue("Output Width", outputWidth) &&
      pInArgList->getPlugInArgValue("Output Height", outputHeight))
   {
      pStep->addProperty("Output Width", outputWidth);
      pStep->addProperty("Output Height", outputHeight);
      outputSize = QSize(outputWidth, outputHeight);
   }

   // Get the current image from the viewer
   // Do not display the progress before calling generateImage
   // because the progress dialog is exported as part of the image on Solaris.
   QImage image(outputSize, QImage::Format_ARGB32);
   View* pView = dynamic_cast<View*>(mpItem);
   ProductView* pProductView = NULL;
   ColorType backColor;
   if (pView != NULL)
   {
      pProductView = dynamic_cast<ProductView*>(pView);
      backColor = pView->getBackgroundColor();
      if (pProductView == NULL && mpDetails->isBackgroundTransparent())
      {
         backColor.mAlpha = 0;
         pView->setBackgroundColor(backColor);
      }
   }
   FAIL_IF(!generateImage(image), "Could not generate image", return false);

   if (image.isNull() == true)
   {
      mMessage = "Unable to get the image to save.";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      //reset the background color to original value
      if (pProductView == NULL && mpDetails->isBackgroundTransparent()
         && pView != NULL)
      {
         backColor.mAlpha = 255;
         pView->setBackgroundColor(backColor);
      }
      return false;
   }

   mMessage = "Saving data to file...";
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessage, 99, NORMAL);
   }

   // Save the image
   bool bSuccess = mpDetails->savePict(QString::fromStdString(mOutPath), image, mpItem);

   //reset the background color to original value
   if (pProductView == NULL && mpDetails->isBackgroundTransparent() && pView != NULL)
   {
      backColor.mAlpha = 255;
      pView->setBackgroundColor(backColor);
   }

   if (bSuccess == false)
   {
      mMessage = "Unable to save the file, check folder permissions";
      if (mpProgress)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);

      remove(mOutPath.c_str());
      return false;
   }

   mMessage = exporterName + " completed.";
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessage, 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

QWidget* PicturesExporter::getExportOptionsWidget(const PlugInArgList *pInArgList)
{
   return mpDetails->getExportOptionsWidget(pInArgList);
}

bool PicturesExporter::extractInputArgs(const PlugInArgList* pInArgList)
{
   string exporter = getName();
   if (exporter.empty())
   {
      exporter = "Pictures Exporter";
   }

   PlugInArg* pArg = NULL;

   // Progress
   if (pInArgList->getArg(Executable::ProgressArg(), pArg) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   // File Descriptor
   mOutPath.erase();
   FileDescriptor* pFileDescriptor = pInArgList->getPlugInArgValue<FileDescriptor>(Exporter::ExportDescriptorArg());
   if (pFileDescriptor != NULL)
   {
      mOutPath = pFileDescriptor->getFilename().getFullPathAndName();
   }
   if (mOutPath.empty())
   {
      mMessage = "The destination path is invalid.";
      return false;
   }

   return true;
}
