/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>

#include "AppVerify.h"
#include "AppVersion.h"
#include "Classification.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DimensionDescriptor.h"
#include "Endian.h"
#include "EnviExporter.h"
#include "FileResource.h"
#include "LabeledSection.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpecialMetadata.h"
#include "TypesFile.h"
#include "Units.h"

#include <stdio.h>

#include <string>
#include <vector>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksENVI, EnviExporter);

EnviExporter::EnviExporter() :
   mpProgress(NULL),
   mpRaster(NULL),
   mpFileDescriptor(NULL),
   mExportDataFile(true),
   mpOptionsWidget(NULL),
   mpDataFileRadio(NULL),
   mpStep(NULL)
{
   setName("ENVI Exporter");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setExtensions("ENVI Header Files (*.hdr)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{08D313EC-2AB2-4e66-B840-6102A2C8E30A}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

EnviExporter::~EnviExporter()
{
   delete mpOptionsWidget;
}

QWidget* EnviExporter::getExportOptionsWidget(const PlugInArgList* pArgList)
{
   if (mpOptionsWidget == NULL)
   {
      mpOptionsWidget = new QWidget();

      QWidget* pSectionWidget = new QWidget(mpOptionsWidget);
      mpDataFileRadio = new QRadioButton("Export data and corresponding header", pSectionWidget);
      QRadioButton* pHeaderOnlyRadio = new QRadioButton("Generate header file for loaded data set", pSectionWidget);
      QLabel* pHeaderOnlyLabel = new QLabel("When generating a header file for an existing data set, "
         "the original data file is presumed to be in a format that can be represented by an ENVI header.  "
         "If you are unsure whether the original data file can be represented by an ENVI header, choose "
         "the option to export a data file instead.\n\nSince this option generates a header based on the "
         "original data file, any edits to export a subset will be ignored.", pSectionWidget);
      pHeaderOnlyLabel->setWordWrap(true);

      LabeledSection* pExportSection = new LabeledSection(pSectionWidget, "Export Type", mpOptionsWidget);

      // Layout
      QGridLayout* pSectionLayout = new QGridLayout(pSectionWidget);
      pSectionLayout->setMargin(0);
      pSectionLayout->setSpacing(5);
      pSectionLayout->addWidget(mpDataFileRadio, 0, 0, 1, 2);
      pSectionLayout->addWidget(pHeaderOnlyRadio, 1, 0, 1, 2);
      pSectionLayout->setColumnMinimumWidth(0, 15);
      pSectionLayout->addWidget(pHeaderOnlyLabel, 2, 1);
      pSectionLayout->setRowStretch(3, 10);
      pSectionLayout->setColumnStretch(1, 10);

      QVBoxLayout* pLayout = new QVBoxLayout(mpOptionsWidget);
      pLayout->setMargin(0);
      pLayout->setSpacing(10);
      pLayout->addWidget(pExportSection, 10);

      // Initialization
      mpDataFileRadio->setChecked(true);

      if (pArgList != NULL)
      {
         // Disable the header generation for a data set if the data set was not originally imported
         RasterElement* pRaster = pArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
         if (pRaster != NULL)
         {
            const DataDescriptor* pDescriptor = pRaster->getDataDescriptor();
            if (pDescriptor != NULL)
            {
               const RasterFileDescriptor* pFileDescriptor =
                  dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
               if (pFileDescriptor == NULL)
               {
                  pHeaderOnlyRadio->setEnabled(false);
                  pHeaderOnlyLabel->setEnabled(false);
               }
            }
         }

         // Set the default export type to generate a header file for a data set if the value is set in the arg list
         if (pHeaderOnlyRadio->isEnabled() == true)
         {
            bool exportDataFile = false;
            if (pArgList->getPlugInArgValue<bool>("Export Header and Data", exportDataFile) == true)
            {
               if (exportDataFile == false)
               {
                  pHeaderOnlyRadio->setChecked(true);
               }
            }
         }
      }
   }

   return mpOptionsWidget;
}

bool EnviExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), "Element to be exported."));
   VERIFY(pArgList->addArg<RasterFileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the exported element."));
   VERIFY(pArgList->addArg<bool>("Export Header and Data", mExportDataFile, "Export the original file header and data.  If false, "
      "a new ENVI-compatible header will be generated."));

   return true;
}

bool EnviExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool EnviExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute ENVI Exporter", "app", "AFDB9430-4D60-447b-8406-6B1C58F8A027");
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   string message = "";

   // Test for incompatible data
   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      message = "Could not get the data descriptor!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   if (pDescriptor->getDataType() == INT4SCOMPLEX)
   {
      message = "An ENVI header cannot represent a data set with complex integer data.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   // Export the header file
   if (mpProgress != NULL)
   {
      message = "Starting the ENVI export...";
      mpProgress->updateProgress(message, 0, NORMAL);
   }

   bool success = exportHeaderFile();

   // Export the data file
   if ((success == true) && (mExportDataFile == true))
   {
      success = exportDataFile();
   }

   if (success == true)
   {
      message = "ENVI export complete.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 100, NORMAL);
      }

      pStep->finalize(Message::Success);
   }
   else if (isAborted() == true)
   {
      pStep->finalize(Message::Abort);
   }
   else
   {
      pStep->finalize(Message::Failure);
   }

   return success;
}

bool EnviExporter::extractInputArgs(const PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   // Progress
   mpProgress = pArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());

   // Data set
   mpRaster = pArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   if (mpRaster == NULL)
   {
      string message = "The raster element input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   // File descriptor
   mpFileDescriptor = pArgList->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   if (mpFileDescriptor == NULL)
   {
      string message = "The file descriptor input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, message);
      return false;
   }

   // Generate existing data set header flag
   if (mpOptionsWidget != NULL)
   {
      if (mpDataFileRadio != NULL)
      {
         mExportDataFile = mpDataFileRadio->isChecked();
      }
   }
   else
   {
      pArgList->getPlugInArgValue<bool>("Export Header and Data", mExportDataFile);
   }

   return true;
}

bool EnviExporter::exportHeaderFile() const
{
   VERIFY(mpRaster != NULL);
   VERIFY(mpFileDescriptor != NULL);

   StepResource pStep("Export header file", "app", "E2DA0E42-1386-46F3-80EC-7F7A745C0621");

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   const RasterFileDescriptor* pFileDescriptor = mpFileDescriptor;
   if (mExportDataFile == false)
   {
      pFileDescriptor = dynamic_cast<const RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      if (pFileDescriptor == NULL)
      {
         string message = "The ENVI Exporter cannot generate a header file for data "
            "that was not originally imported from a file.";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(message, 0, ERRORS);
         }

         pStep->finalize(Message::Failure, message);
         return false;
      }
   }

   // Get the header filename from the export file descriptor
   string headerFilename = mpFileDescriptor->getFilename();
   if (headerFilename.empty() == true)
   {
      string message = "The header filename is invalid.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   const string ext = ".hdr";
   if (QString::fromStdString(headerFilename).endsWith(QString::fromStdString(ext), Qt::CaseInsensitive) == false)
   {
      headerFilename += ext;
   }

   pStep->addProperty("Header filename", headerFilename);

   FILE* pStream = fopen(headerFilename.c_str(), "wt");
   if (pStream == NULL)
   {
      string message = "Unable to write to header file:\n" + headerFilename;
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      return false;
   }

   string message = "Exporting header file...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(message, 0, NORMAL);
   }

   // ENVI keyword
   int i = fprintf(pStream, "ENVI\n");

   // Description
   if (i > 0)
   {
      i = fprintf(pStream, "description = {\n");
   }

   if ((i > 0) && (mExportDataFile == false))
   {
      string dataFilename = mpRaster->getFilename();
      if (dataFilename.empty() == false)
      {
         i = fprintf(pStream, "    FILENAME = %s\n", dataFilename.c_str());
      }
   }

   if (i > 0)
   {
      const Classification* pClass = mpRaster->getClassification();
      if (pClass != NULL)
      {
         string classLevel = pClass->getLevel();
         if (classLevel != "U")
         {
            string level = "U";
            if (classLevel == "U")
            {
               level = "UNCLASSIFIED";
            }
            else if (classLevel == "C")
            {
               level = "CONFIDENTIAL";
            }
            else if (classLevel == "R")
            {
               level = "RESTRICTED";
            }
            else if (classLevel == "S")
            {
               level = "SECRET";
            }
            else if (classLevel == "T")
            {
               level = "TOP SECRET";
            }

            i = fprintf(pStream, "    CLASSIFICATION = %s\n", level.c_str());
         }
      }
   }

   if (i > 0)
   {
      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         vector<string> metadataKeys;
         pMetadata->getAttributeNames(metadataKeys);

         string fieldType;
         for (vector<string>::iterator it = metadataKeys.begin(); it != metadataKeys.end(); ++it)
         {
            if (isAborted() == true)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(headerFilename.c_str());
               return false;
            }

            if (i > 0)
            {
               string key = *it;
               if (!key.empty())
               {
                  string value = pMetadata->getAttribute(key).toXmlString();
                  if (!value.empty())
                  {
                     if (i > 0)
                     {
                        i = fprintf(pStream, "    %s = %s\n", key.c_str(), value.c_str());
                     }
                  }
               }
            }
         }
      }
   }

   if (i > 0)
   {
      i = fprintf(pStream, "}\n");
   }

   if (isAborted() == true)
   {
      message = "ENVI export aborted!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ABORT);
      }

      pStep->finalize(Message::Abort);

      fclose(pStream);
      remove(headerFilename.c_str());
      return false;
   }

   // Columns
   if (i > 0)
   {
      i = fprintf(pStream, "samples = %d\n", pFileDescriptor->getColumnCount());
   }

   // Rows
   if (i > 0)
   {
      i = fprintf(pStream, "lines = %d\n", pFileDescriptor->getRowCount());
   }

   // Bands
   if (i > 0)
   {
      i = fprintf(pStream, "bands = %d\n", pFileDescriptor->getBandCount());
   }

   // Header bytes
   if (i > 0)
   {
      i = fprintf(pStream, "header offset = %d\n", pFileDescriptor->getHeaderBytes());
   }

   // File type
   if (i > 0)
   {
      i = fprintf(pStream, "file type = ENVI Standard\n");
   }

   // Data type
   if (i > 0)
   {
      int dataType = 0;
      switch (pDescriptor->getDataType())
      {
         case INT1UBYTE:
         case INT1SBYTE:
            dataType = 1;
            break;
         case INT2UBYTES:
            dataType = 12;
            break;
         case INT2SBYTES:
            dataType = 2;
            break;
         case INT4UBYTES:
            dataType = 13;
            break;
         case INT4SBYTES:
            dataType = 3;
            break;
         case FLT4BYTES:
            dataType = 4;
            break;
         case FLT8COMPLEX:
            dataType = 6;
            break;
         case FLT8BYTES:
            dataType = 5;
            break;
         default:
            break;
      }

      VERIFY(dataType != 0);
      i = fprintf(pStream, "data type = %d\n", dataType);
   }

   // Interleave format
   if (i > 0)
   {
      InterleaveFormatType interleaveFormat = pFileDescriptor->getInterleaveFormat();
      string interleaveText = convertInterleaveToText(interleaveFormat);
      i = fprintf(pStream, "interleave = %s\n", interleaveText.c_str());
   }

   // Byte order
   if (i > 0)
   {
      bool bMsb = false;
      if (mExportDataFile == true)
      {
         if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
         {
            bMsb = true;
         }
      }
      else if (pFileDescriptor->getEndian() == BIG_ENDIAN_ORDER)
      {
         bMsb = true;
      }

      i = fprintf(pStream, "byte order = %d\n", bMsb);
   }

   // Offset
   if (i > 0)
   {
      const vector<DimensionDescriptor>& columns = pFileDescriptor->getColumns();
      DimensionDescriptor startColumn = columns.front();
      DimensionDescriptor endColumn = columns.back();

      if ((startColumn.isOriginalNumberValid() == true) && (endColumn.isOriginalNumberValid() == true))
      {
         if (columns.size() == endColumn.getOriginalNumber() - startColumn.getOriginalNumber() + 1)
         {
            if (startColumn.getOriginalNumber() > 0)
            {
               // ENVI uses one-based numbers
               i = fprintf(pStream, "x start = %d\n", startColumn.getOriginalNumber() + 1);
            }
         }
      }
   }

   if (i > 0)
   {
      const vector<DimensionDescriptor>& rows = pFileDescriptor->getRows();
      DimensionDescriptor startRow = rows.front();
      DimensionDescriptor endRow = rows.back();

      if ((startRow.isOriginalNumberValid() == true) && (endRow.isOriginalNumberValid() == true))
      {
         if (rows.size() == endRow.getOriginalNumber() - startRow.getOriginalNumber() + 1)
         {
            if (startRow.getOriginalNumber() > 0)
            {
               // ENVI uses one-based numbers
               i = fprintf(pStream, "y start = %d\n", startRow.getOriginalNumber() + 1);
            }
         }
      }
   }

   // Geo points
   if (i > 0)
   {
      if (mpRaster->isGeoreferenced())
      {
         const vector<DimensionDescriptor>& rows = pFileDescriptor->getRows();
         const vector<DimensionDescriptor>& cols = pFileDescriptor->getColumns();
         if (!rows.empty() && !cols.empty())
         {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : This functionality should be moved into a method in a " \
   "new RasterElementExporterShell class (dsulgrov)")
            list<GcpPoint> gcps;
            unsigned int startRow = rows.front().getActiveNumber();
            unsigned int endRow = rows.back().getActiveNumber();
            unsigned int startCol = cols.front().getActiveNumber();
            unsigned int endCol = cols.back().getActiveNumber();

            GcpPoint urPoint;
            GcpPoint ulPoint;
            GcpPoint lrPoint;
            GcpPoint llPoint;
            GcpPoint centerPoint;
            ulPoint.mPixel = LocationType(startCol, startRow);
            urPoint.mPixel = LocationType(endCol, startRow);
            llPoint.mPixel = LocationType(startCol, endRow);
            lrPoint.mPixel = LocationType(endCol, endRow);

            ulPoint.mCoordinate = mpRaster->convertPixelToGeocoord(ulPoint.mPixel);
            urPoint.mCoordinate = mpRaster->convertPixelToGeocoord(urPoint.mPixel);
            llPoint.mCoordinate = mpRaster->convertPixelToGeocoord(llPoint.mPixel);
            lrPoint.mCoordinate = mpRaster->convertPixelToGeocoord(lrPoint.mPixel);

            //reset the coordinates, because on import they are required to be in
            //on-disk numbers not active numbers
            unsigned int diskStartRow = rows.front().getOnDiskNumber();
            unsigned int diskEndRow = rows.back().getOnDiskNumber();
            unsigned int diskStartCol = cols.front().getOnDiskNumber();
            unsigned int diskEndCol = cols.back().getOnDiskNumber();
            ulPoint.mPixel = LocationType(diskStartCol, diskStartRow);
            urPoint.mPixel = LocationType(diskEndCol, diskStartRow);
            llPoint.mPixel = LocationType(diskStartCol, diskEndRow);
            lrPoint.mPixel = LocationType(diskEndCol, diskEndRow);

            gcps.push_back(ulPoint);
            gcps.push_back(urPoint);
            gcps.push_back(llPoint);
            gcps.push_back(lrPoint);

            fprintf(pStream, "geo points = {");
            for (list<GcpPoint>::const_iterator it = gcps.begin(); it != gcps.end(); )
            {
               GcpPoint gcp = *it;
               // ENVI uses a 1-based pixel coordinate system, with each coordinate referring
               // to the top-left corner of the pixel, e.g. (1,1) is the top-left
               // corner of the pixel in the top-left of the raster cube
               // The ENVI pixel coordinate format is described on p. 1126 of the ENVI 4.2 User's Guide
               i = fprintf(pStream, "\n %.4f, %.4f, %.8f, %.8f", gcp.mPixel.mX + 1.0, 
                  gcp.mPixel.mY + 1.0, gcp.mCoordinate.mX, gcp.mCoordinate.mY);

               ++it;
               if (it != gcps.end())
               {
                  i = fprintf(pStream, ",");
               }
            }

            i = fprintf(pStream, "}\n");
         }
      }
   }

   // Reflectance scale factor
   if (i > 0)
   {
      const Units* pUnits = pDescriptor->getUnits();
      if (pUnits != NULL)
      {
         if (pUnits->getUnitType() == REFLECTANCE && abs(pUnits->getScaleFromStandard() - 1.0) > 0.0000001)
         {
            float fltVal = static_cast<float>(1.0/pUnits->getScaleFromStandard());
            i = fprintf(pStream, "reflectance scale factor = %f\n", fltVal);
         }
      }
   }

   // Bad bands
   const vector<DimensionDescriptor>& allBands = pFileDescriptor->getBands();
   vector<DimensionDescriptor>::const_iterator allBandsIter;
   if ((i > 0) && (mExportDataFile == false))
   {
      const vector<DimensionDescriptor>& activeBands = pDescriptor->getBands();
      if (allBands.size() != activeBands.size())
      {
         if (i > 0)
         {
            i = fprintf(pStream, "bbl = {\n");
         }

         unsigned int count = 0;
         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter, ++count)
         {
            if (isAborted() == true)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(headerFilename.c_str());
               return false;
            }

            if (allBandsIter->isActiveNumberValid())
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "1");
               }
            }
            else
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "0");
               }
            }

            if (allBandsIter == (allBands.end() - 1))
            {
               if (i > 0)
               {
                  i = fprintf(pStream, "}\n");
               }
            }
            else
            {
               if (i > 0)
               {
                  i = fprintf(pStream, ",");
               }
               if (i > 0)
               {
                  if ((count + 1) % 40 == 0)
                  {
                     i = fprintf(pStream, "\n");
                  }
               }
            }
         }
      }
   }

   // Wavelengths
   if (i > 0)
   {
      vector<double> centerWavelengths;
      vector<double> startWavelengths;
      vector<double> endWavelengths;

      const DynamicObject* pMetadata = pDescriptor->getMetadata();
      if (pMetadata != NULL)
      {
         string pCenterPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, CENTER_WAVELENGTHS_METADATA_NAME,
            END_METADATA_NAME };
         const vector<double>* pWavelengthData = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(pCenterPath));
         if (pWavelengthData != NULL)
         {
            centerWavelengths = *pWavelengthData;
         }

         string pStartPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, START_WAVELENGTHS_METADATA_NAME,
            END_METADATA_NAME };
         pWavelengthData = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(pStartPath));
         if (pWavelengthData != NULL)
         {
            startWavelengths = *pWavelengthData;
         }

         string pEndPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, END_WAVELENGTHS_METADATA_NAME,
            END_METADATA_NAME };
         pWavelengthData = dv_cast<vector<double> >(&pMetadata->getAttributeByPath(pEndPath));
         if (pWavelengthData != NULL)
         {
            endWavelengths = *pWavelengthData;
         }
      }

      if (!centerWavelengths.empty())
      {
         i = fprintf(pStream, "wavelength units = Micrometers\n");

         if (i > 0)
         {
            i = fprintf (pStream, "wavelength = {\n");
         }

         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
         {
            if (isAborted() == true)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(headerFilename.c_str());
               return false;
            }
            if ((i > 0) && (allBandsIter != allBands.begin()))
            {
               i = fprintf(pStream, ",\n");
            }
            double wavelength = 0.0;
            if (i > 0)
            {
               if (allBandsIter->isActiveNumberValid())
               {
                  unsigned int bandNumber = allBandsIter->getActiveNumber();
                  if (bandNumber < centerWavelengths.size())
                  {
                     wavelength = centerWavelengths[bandNumber];
                  }
               }
               i = fprintf(pStream, "%1.12g", wavelength);
            }
         }
         if (i > 0)
         {
            i = fprintf(pStream, "}\n");
         }
      }

      if (!centerWavelengths.empty())
      {
         if (i > 0)
         {
            i = fprintf(pStream, "fwhm = {\n");
         }

         if (!startWavelengths.empty() && !endWavelengths.empty())
         {
            for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
            {
               if (isAborted() == true)
               {
                  message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);

                  fclose(pStream);
                  remove(headerFilename.c_str());
                  return false;
               }

               if ((i > 0) && (allBandsIter != allBands.begin()))
               {
                  i = fprintf(pStream, ",\n");
               }
               if (i > 0)
               {
                  double fwhm = 0.0;
                  if (allBandsIter->isActiveNumberValid())
                  {
                     unsigned int bandNumber = allBandsIter->getActiveNumber();
                     if ((bandNumber < startWavelengths.size()) && (bandNumber < endWavelengths.size()))
                     {
                        fwhm = endWavelengths[bandNumber] - startWavelengths[bandNumber];
                     }
                  }
                  i = fprintf(pStream, "%1.12g", fwhm);
               }
            }
            if (i > 0)
            {
               i = fprintf(pStream, "}\n");
            }
         }
         else
         {
            vector<double> calculatedFwhm;
            calculatedFwhm.reserve(centerWavelengths.size());
            vector<double>::iterator wave;
            double value = 0.0;
            for (wave = centerWavelengths.begin(); wave != centerWavelengths.end(); ++wave)
            {
               if ((wave + 1) != centerWavelengths.end())
               {
                  double centerWavelength = *wave;
                  double nextCenterWavelength = *(wave + 1);
                  value = 1.3 * (nextCenterWavelength - centerWavelength);
                  calculatedFwhm.push_back(value);
               }
            }
            calculatedFwhm.push_back(value);

            for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
            {
               if (isAborted() == true)
               {
                  message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);

                  fclose(pStream);
                  remove(headerFilename.c_str());
                  return false;
               }

               if ((i > 0) && (allBandsIter != allBands.begin()))
               {
                  i = fprintf(pStream, ",\n");
               }

               if (i > 0)
               {
                  double fwhm = 0.0;
                  if (allBandsIter->isActiveNumberValid())
                  {
                     unsigned int bandNumber = allBandsIter->getActiveNumber();
                     if (bandNumber < calculatedFwhm.size())
                     {
                        fwhm = calculatedFwhm[bandNumber];
                     }
                  }
                  i = fprintf(pStream, "%1.12g", fwhm);
               }
            }
            if (i > 0)
            {
               i = fprintf(pStream, "}\n");
            }
         }
      }
   }

   // Band names
   if (i > 0)
   {
      vector<string> bandNames = RasterUtilities::getBandNames(pDescriptor);
      if (!bandNames.empty())
      {
         if (i > 0)
         {
            i = fprintf (pStream, "band names = {\n");
         }

         for (allBandsIter = allBands.begin(); allBandsIter != allBands.end(); ++allBandsIter)
         {
            if (isAborted() == true)
            {
               message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);

               fclose(pStream);
               remove(headerFilename.c_str());
               return false;
            }
            if ((i > 0) && (allBandsIter != allBands.begin()))
            {
               i = fprintf(pStream, ",\n"); 
            }
            if (i > 0)
            {
               string bandName = "Unknown";
               if (allBandsIter->isActiveNumberValid())
               {
                  unsigned int bandNumber = allBandsIter->getActiveNumber();
                  if (bandNumber < bandNames.size())
                  {
                     bandName = bandNames[bandNumber];
                  }
               }
               i = fprintf(pStream, "%s", bandName.c_str());
            }
         }
         if (i > 0)
         {
            // Add a new line so that ENVI will successfully read the file
            i = fprintf(pStream, "}\n");
         }
      }
   }

   fclose(pStream);

   if (i <= 0)
   {
      message = "Error writing to header file:\n" + headerFilename;
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      remove(headerFilename.c_str());
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Header file export complete", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool EnviExporter::exportDataFile() const
{
   VERIFY(mpRaster != NULL);
   VERIFY(mpFileDescriptor != NULL);

   const string& headerFilename = mpFileDescriptor->getFilename();
   VERIFY(headerFilename.empty() == false);

   const string ext = ".hdr";
   string dataFilename = headerFilename;
   if (QString::fromStdString(dataFilename).endsWith(QString::fromStdString(ext), Qt::CaseInsensitive) == true)
   {
      dataFilename.resize(dataFilename.length() - ext.length());
   }

   StepResource pStep("Export data file", "app", "90DD1ADE-7CFD-4A81-B52E-6AA918A0945F");
   pStep->addProperty("Data filename", dataFilename);

   LargeFileResource dataFile;
   if (dataFile.open(dataFilename, O_WRONLY | O_CREAT | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE | S_IEXEC) == false)
   {
      string message = "Could not open the data file for writing.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      remove(headerFilename.c_str());
      return false;
   }

   const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDescriptor != NULL);

   const vector<DimensionDescriptor>& exportRows = mpFileDescriptor->getRows();
   const vector<DimensionDescriptor>& exportColumns = mpFileDescriptor->getColumns();
   const vector<DimensionDescriptor>& exportBands = mpFileDescriptor->getBands();
   unsigned int bytesPerElement = pDescriptor->getBytesPerElement();

   string progressText = "Exporting the data file...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(progressText, 0, NORMAL);
   }

   int rowIndex = 0;
   InterleaveFormatType interleave = mpFileDescriptor->getInterleaveFormat();
   if (interleave == BIP)
   {
      if (exportBands.size() == pDescriptor->getBandCount())   // All bands
      {
         if (exportColumns.size() ==
            exportColumns.back().getActiveNumber() - exportColumns.front().getActiveNumber() + 1)
         {
            // Export a full contiguous row at a time
            FactoryResource<DataRequest> pDataRequest;
            pDataRequest->setInterleaveFormat(BIP);
            pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
            pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), exportColumns.size());

            DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
            if (dataAccessor.isValid() == false)
            {
               string message = "The data in the data set could not be accessed.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            vector<DimensionDescriptor>::const_iterator iter;
            for (iter = exportRows.begin(); iter != exportRows.end(); ++iter)
            {
               if (isAborted() == true)
               {
                  string message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               DimensionDescriptor row = *iter;
               dataAccessor->toPixel(row.getActiveNumber(), exportColumns.front().getActiveNumber());
               if (dataAccessor.isValid() == false)
               {
                  string message = "An error occurred when reading the data from the data set.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               void* pData = dataAccessor->getRow();
               dataFile.write(pData, bytesPerElement * exportBands.size() * exportColumns.size());

               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
               }
            }
         }
         else
         {
            // Export one full pixel at a time
            FactoryResource<DataRequest> pDataRequest;
            pDataRequest->setInterleaveFormat(BIP);
            pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
            pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), 1);

            DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
            if (dataAccessor.isValid() == false)
            {
               string message = "The data in the data set could not be accessed.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            vector<DimensionDescriptor>::const_iterator rowIter;
            for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
            {
               if (isAborted() == true)
               {
                  string message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               vector<DimensionDescriptor>::const_iterator colIter;
               for (colIter = exportColumns.begin(); colIter != exportColumns.end(); ++colIter)
               {
                  DimensionDescriptor row = *rowIter;
                  DimensionDescriptor column = *colIter;
                  dataAccessor->toPixel(row.getActiveNumber(), column.getActiveNumber());
                  if (dataAccessor.isValid() == false)
                  {
                     string message = "An error occurred when reading the data from the data set.";
                     if (mpProgress != NULL)
                     {
                        mpProgress->updateProgress(message, 0, ERRORS);
                     }

                     pStep->finalize(Message::Failure, message);
                     dataFile.close();
                     remove(headerFilename.c_str());
                     remove(dataFilename.c_str());
                     return false;
                  }

                  void* pData = dataAccessor->getColumn();
                  dataFile.write(pData, bytesPerElement * exportBands.size());
               }

               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
               }
            }
         }
      }
      else
      {
         // Slowest possible copy, one pixel at a time
         FactoryResource<DataRequest> pDataRequest;
         pDataRequest->setInterleaveFormat(BIP);
         pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
         pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), 1);

         DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
         if (dataAccessor.isValid() == false)
         {
            string message = "The data in the data set could not be accessed.";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ERRORS);
            }

            pStep->finalize(Message::Failure, message);
            dataFile.close();
            remove(headerFilename.c_str());
            remove(dataFilename.c_str());
            return false;
         }

         vector<DimensionDescriptor>::const_iterator rowIter;
         for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
         {
            if (isAborted() == true)
            {
               string message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            vector<DimensionDescriptor>::const_iterator colIter;
            for (colIter = exportColumns.begin(); colIter != exportColumns.end(); ++colIter)
            {
               DimensionDescriptor row = *rowIter;
               DimensionDescriptor column = *colIter;
               dataAccessor->toPixel(row.getActiveNumber(), column.getActiveNumber());
               if (dataAccessor.isValid() == false)
               {
                  string message = "An error occurred when reading the data from the data set.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               char* pData = reinterpret_cast<char*>(dataAccessor->getColumn());

               vector<DimensionDescriptor>::const_iterator bandIter;
               for (bandIter = exportBands.begin(); bandIter != exportBands.end(); ++bandIter)
               {
                  DimensionDescriptor band = *bandIter;
                  dataFile.write(pData + bytesPerElement * (band.getActiveNumber()), bytesPerElement);
               }
            }

            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
            }
         }
      }
   }
   else if (interleave == BSQ)
   {
      if (exportColumns.size() == exportColumns.back().getActiveNumber() - exportColumns.front().getActiveNumber() + 1)
      {
         vector<DimensionDescriptor>::const_iterator bandIter;
         for (bandIter = exportBands.begin(); bandIter != exportBands.end(); ++bandIter)
         {
            DimensionDescriptor band = *bandIter;

            // Export a full contiguous row at a time
            FactoryResource<DataRequest> pDataRequest;
            pDataRequest->setInterleaveFormat(BSQ);
            pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
            pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), exportColumns.size());
            pDataRequest->setBands(band, band, 1);

            DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
            if (dataAccessor.isValid() == false)
            {
               string message = "The data in the data set could not be accessed.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            vector<DimensionDescriptor>::const_iterator rowIter;
            for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
            {
               if (isAborted() == true)
               {
                  string message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               if (dataAccessor.isValid() == false)
               {
                  string message = "An error occurred when reading the data from the data set.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               void* pData = dataAccessor->getRow();
               dataFile.write(pData, exportColumns.size() * bytesPerElement);

               dataAccessor->nextRow();

               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(progressText,
                     (rowIndex++ * 100) / (exportRows.size() * exportBands.size()), NORMAL);
               }
            }
         }
      }
      else
      {
         vector<DimensionDescriptor>::const_iterator bandIter;
         for (bandIter = exportBands.begin(); bandIter != exportBands.end(); ++bandIter)
         {
            DimensionDescriptor band = *bandIter;

            // Slowest possible copy, one pixel at a time
            FactoryResource<DataRequest> pDataRequest;
            pDataRequest->setInterleaveFormat(BSQ);
            pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
            pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), 1);
            pDataRequest->setBands(band, band, 1);

            DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
            if (dataAccessor.isValid() == false)
            {
               string message = "The data in the data set could not be accessed.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            vector<DimensionDescriptor>::const_iterator rowIter;
            for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
            {
               if (isAborted() == true)
               {
                  string message = "ENVI export aborted!";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ABORT);
                  }

                  pStep->finalize(Message::Abort);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               DimensionDescriptor row = *rowIter;

               vector<DimensionDescriptor>::const_iterator columnIter;
               for (columnIter = exportColumns.begin(); columnIter != exportColumns.end(); ++columnIter)
               {
                  DimensionDescriptor column = *columnIter;

                  dataAccessor->toPixel(row.getActiveNumber(), column.getActiveNumber());
                  if (dataAccessor.isValid() == false)
                  {
                     string message = "An error occurred when reading the data from the data set.";
                     if (mpProgress != NULL)
                     {
                        mpProgress->updateProgress(message, 0, ERRORS);
                     }

                     pStep->finalize(Message::Failure, message);
                     dataFile.close();
                     remove(headerFilename.c_str());
                     remove(dataFilename.c_str());
                     return false;
                  }

                  void* pData = dataAccessor->getColumn();
                  dataFile.write(pData, bytesPerElement);
               }

               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(progressText,
                     (rowIndex++ * 100) / (exportRows.size() * exportBands.size()), NORMAL);
               }
            }
         }
      }
   }
   else if (interleave == BIL)
   {
      if (exportColumns.size() == pDescriptor->getColumnCount() &&
         (exportBands.size() == exportBands.back().getActiveNumber() - exportBands.front().getActiveNumber() + 1))
      {
         // Export a full contiguous row at a time
         FactoryResource<DataRequest> pDataRequest;
         pDataRequest->setInterleaveFormat(BIL);
         pDataRequest->setRows(exportRows.front(), exportRows.back(), 1);
         pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), exportColumns.size());
         pDataRequest->setBands(exportBands.front(), exportBands.back(), exportBands.size());

         DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
         if (dataAccessor.isValid() == false)
         {
            string message = "The data in the data set could not be accessed.";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(message, 0, ERRORS);
            }

            pStep->finalize(Message::Failure, message);
            dataFile.close();
            remove(headerFilename.c_str());
            remove(dataFilename.c_str());
            return false;
         }

         for (vector<DimensionDescriptor>::const_iterator iter = exportRows.begin(); iter != exportRows.end(); ++iter)
         {
            if (isAborted() == true)
            {
               string message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            DimensionDescriptor row = *iter;
            dataAccessor->toPixel(row.getActiveNumber(), exportColumns.front().getActiveNumber());
            if (dataAccessor.isValid() == false)
            {
               string message = "An error occurred when reading the data from the data set.";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ERRORS);
               }

               pStep->finalize(Message::Failure, message);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            void* pData = dataAccessor->getRow();
            dataFile.write(pData, bytesPerElement * exportBands.size() * exportColumns.size());

            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
            }
         }
      }
      else if (exportColumns.size() ==
         exportColumns.back().getActiveNumber() - exportColumns.front().getActiveNumber() + 1)
      {
         // Export a row from a single band at a time
         vector<DimensionDescriptor>::const_iterator rowIter;
         for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
         {
            if (isAborted() == true)
            {
               string message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            DimensionDescriptor row = *rowIter;

            vector<DimensionDescriptor>::const_iterator bandIter;
            for (bandIter = exportBands.begin(); bandIter != exportBands.end(); ++bandIter)
            {
               DimensionDescriptor band = *bandIter;

               FactoryResource<DataRequest> pDataRequest;
               pDataRequest->setInterleaveFormat(BIL);
               pDataRequest->setRows(row, row, 1);
               pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), exportColumns.size());
               pDataRequest->setBands(band, band, 1);

               DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
               if (dataAccessor.isValid() == false)
               {
                  string message = "The data in the data set could not be accessed.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               dataAccessor->toPixel(row.getActiveNumber(), exportColumns.front().getActiveNumber());
               if (dataAccessor.isValid() == false)
               {
                  string message = "An error occurred when reading the data from the data set.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               void* pData = dataAccessor->getColumn();
               dataFile.write(pData, exportColumns.size() * bytesPerElement);
            }

            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
            }
         }
      }
      else
      {
         // Slowest possible copy, one pixel at a time
         vector<DimensionDescriptor>::const_iterator rowIter;
         for (rowIter = exportRows.begin(); rowIter != exportRows.end(); ++rowIter)
         {
            if (isAborted() == true)
            {
               string message = "ENVI export aborted!";
               if (mpProgress != NULL)
               {
                  mpProgress->updateProgress(message, 0, ABORT);
               }

               pStep->finalize(Message::Abort);
               dataFile.close();
               remove(headerFilename.c_str());
               remove(dataFilename.c_str());
               return false;
            }

            DimensionDescriptor row = *rowIter;

            vector<DimensionDescriptor>::const_iterator bandIter;
            for (bandIter = exportBands.begin(); bandIter != exportBands.end(); ++bandIter)
            {
               DimensionDescriptor band = *bandIter;

               FactoryResource<DataRequest> pDataRequest;
               pDataRequest->setInterleaveFormat(BIL);
               pDataRequest->setRows(row, row, 1);
               pDataRequest->setColumns(exportColumns.front(), exportColumns.back(), 1);
               pDataRequest->setBands(band, band, 1);

               DataAccessor dataAccessor = mpRaster->getDataAccessor(pDataRequest.release());
               if (dataAccessor.isValid() == false)
               {
                  string message = "The data in the data set could not be accessed.";
                  if (mpProgress != NULL)
                  {
                     mpProgress->updateProgress(message, 0, ERRORS);
                  }

                  pStep->finalize(Message::Failure, message);
                  dataFile.close();
                  remove(headerFilename.c_str());
                  remove(dataFilename.c_str());
                  return false;
               }

               vector<DimensionDescriptor>::const_iterator columnIter;
               for (columnIter = exportColumns.begin(); columnIter != exportColumns.end(); ++columnIter)
               {
                  DimensionDescriptor column = *columnIter;

                  dataAccessor->toPixel(row.getActiveNumber(), column.getActiveNumber());
                  if (dataAccessor.isValid() == false)
                  {
                     string message = "An error occurred when reading the data from the data set.";
                     if (mpProgress != NULL)
                     {
                        mpProgress->updateProgress(message, 0, ERRORS);
                     }

                     pStep->finalize(Message::Failure, message);
                     dataFile.close();
                     remove(headerFilename.c_str());
                     remove(dataFilename.c_str());
                     return false;
                  }

                  void* pData = dataAccessor->getColumn();
                  dataFile.write(pData, bytesPerElement);
               }
            }

            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(progressText, (rowIndex++ * 100) / exportRows.size(), NORMAL);
            }
         }
      }
   }
   else
   {
      string message = "The interleave format of the data set is not supported.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(message, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, message);
      dataFile.close();
      remove(headerFilename.c_str());
      remove(dataFilename.c_str());
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Data file export complete", 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

string EnviExporter::convertInterleaveToText(InterleaveFormatType interleave) const
{
   string interleaveText;
   if (interleave == BIP)
   {
      interleaveText = "bip";
   }
   else if (interleave == BSQ)
   {
      interleaveText = "bsq";
   }
   else if (interleave == BIL)
   {
      interleaveText = "bil";
   }

   return interleaveText;
}
