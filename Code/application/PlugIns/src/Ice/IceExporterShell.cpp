/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "IceExporterShell.h"
#include "IceUtilities.h"
#include "IceWriter.h"
#include "OptionsIceExporter.h"
#include "PlugInArgList.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterElement.h"

using namespace std;

IceExporterShell::IceExporterShell(IceUtilities::FileType fileType) :
   mpProgress(NULL),
   mpWriter(NULL),
   mFileType(fileType)
{
}

IceExporterShell::~IceExporterShell()
{
}

bool IceExporterShell::abort()
{
   VERIFYNR(ExporterShell::abort());
   if (mpWriter != NULL)
   {
      mpWriter->abort();
   }

   return true;
}

bool IceExporterShell::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = mpPlugInMgr->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(ProgressArg(), NULL));
   return true;
}

bool IceExporterShell::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   string filename;

   try
   {
      // Set up subclasses
      parseInputArgs(pInArgList);

      // Get the RasterElement and RasterFileDescriptor
      RasterElement* pOutputCube = NULL;
      RasterFileDescriptor* pOutputFileDescriptor = NULL;
      getOutputCubeAndFileDescriptor(pOutputCube, pOutputFileDescriptor);
      ICEVERIFY_MSG(pOutputCube != NULL, "No Raster Element specified.");
      ICEVERIFY_MSG(pOutputFileDescriptor != NULL, "No Raster File Descriptor specified.");

      RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pOutputCube->getDataDescriptor());
      ICEVERIFY_MSG(pRasterDescriptor != NULL, "Raster Element is invalid.");

      // Open the file for writing
      filename = pOutputFileDescriptor->getFilename().getFullPathAndName();
      Hdf5FileResource fileResource(H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT));
      ICEVERIFY_MSG(*fileResource >= 0, "Unable to create file " + filename)

      IceWriter writer(*fileResource, mFileType);
      mpWriter = &writer;
      if (mpOptionsWidget.get() != NULL)
      {
         writer.setChunkSize(mpOptionsWidget->getChunkSize() * 1024 * 1024);
         writer.setCompressionType(mpOptionsWidget->getCompressionType());
         writer.setGzipCompressionLevel(mpOptionsWidget->getGzipCompressionLevel());
      }
      if ((pRasterDescriptor->getDataType() == INT4SCOMPLEX || pRasterDescriptor->getDataType() == FLT8COMPLEX)
       && (writer.getCompressionType() == GZIP || writer.getCompressionType() == SHUFFLE_AND_GZIP))
      {
         std::string msgtxt = "Compression not supported with complex data types, data will not be compressed.";
         MessageResource msg(msgtxt, "app", "{7B050EE4-D025-43b2-AD8F-DF8E28FA8551}");
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(msgtxt, 1, WARNING);
         }
      }

      writer.writeFileHeader();
      abortIfNecessary();

      // Write the cube to the file
      writer.writeCube(outputCubePath(), pOutputCube, pOutputFileDescriptor, mpProgress);
      abortIfNecessary();

      // Allow subclasses to write class-specific information to the file
      finishWriting(writer);
      abortIfNecessary();

      // Finished
      mpWriter = NULL;
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Ice export complete.", 100, NORMAL);
      }
   }

   catch (const IceException& ex)
   {
      if (mpProgress != NULL)
      {
         string msg = ex.getFailureMessage();
         if (msg.empty())
         {
            msg = "Unknown error. Failed to create file.";
         }

         mpProgress->updateProgress(msg, 0, ERRORS);
      }

      ex.addToMessageLog();
      if (filename.empty() == false)
      {
         remove(filename.c_str());
      }

      mpWriter = NULL;
      return false;
   }

   catch (const IceAbortException&)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Ice export aborted.", 0, ABORT);
      }

      if (filename.empty() == false)
      {
         remove(filename.c_str());
      }

      mpWriter = NULL;
      return false;
   }

   return true;
}

QWidget* IceExporterShell::getExportOptionsWidget(const PlugInArgList*)
{
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new OptionsIceExporter());
      mpOptionsWidget->setSaveSettings(false);
   }

   return mpOptionsWidget.get();
}

void IceExporterShell::parseInputArgs(PlugInArgList* pInArgList)
{
   ICEVERIFY_MSG(pInArgList != NULL, "Invalid argument list specified.");
   mpProgress = pInArgList->getPlugInArgValue<Progress>(ProgressArg());
}

void IceExporterShell::finishWriting(IceWriter& writer)
{
}

void IceExporterShell::abortIfNecessary()
{
   if (isAborted())
   {
      throw IceAbortException();
   }
}

string IceExporterShell::outputCubePath()
{
   return "/Datasets/Cube1";
}
