/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "RasterElement.h"
#include "Classification.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataDescriptor.h"
#include "Filename.h"
#include "LayerList.h"
#include "MessageLog.h"
#include "NitfConstants.h"
#include "NitfExporter.h"
#include "NitfMetadataParsing.h"
#include "NitfResource.h"
#include "NitfUtilities.h"
#include "OptionsNitfExporter.h"
#include "OssimAppMemorySource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "MessageLogResource.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"

#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimNitfWriter.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/base/ossimContainerProperty.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

using namespace boost;
using namespace std;

REGISTER_PLUGIN(OpticksNitf, NitfExporter, Nitf::NitfExporter);

Nitf::NitfExporter::NitfExporter() :
   mpRaster(NULL),
   mpRasterLayer(NULL),
   mpProgress(NULL),
   mpDestination(NULL),
   mbAborted(false)
{
   setName("NITF Exporter");
   setExtensions("NITF Files (*.ntf *.NTF *.nitf *.NITF *.r0 *.R0)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{C9FC2ED1-60DF-4f4b-B428-22950FE6F04E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::NitfExporter::~NitfExporter()
{
}

bool Nitf::NitfExporter::hasAbort()
{
   return true;
}

bool Nitf::NitfExporter::abort()
{
   mbAborted = true;
   return true;
}

bool Nitf::NitfExporter::abortRequested()
{
   return mbAborted;
}

bool Nitf::NitfExporter::getInputSpecification(PlugInArgList *&pArgList)
{
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterFileDescriptor>(Exporter::ExportDescriptorArg(), "File descriptor for the output file."));
   VERIFY(pArgList->addArg<RasterElement>(Exporter::ExportItemArg(), "Raster element to be exported."));
   VERIFY(pArgList->addArg<View>(Executable::ViewArg(), "View to be exported."));

   if (isBatch() == true)
   {
      VERIFY(pArgList->addArg<bool>("Classification Must Be Valid", true, "Whether the exported classification must be valid."));
   }

   return true;
}

bool Nitf::NitfExporter::getOutputSpecification(PlugInArgList *&pArgList)
{
   pArgList = NULL;
   return true;
}

bool Nitf::NitfExporter::execute(PlugInArgList *pInParam, PlugInArgList *pOutParam)
{
   StepResource pStep("NITF Exporter", "app", "6CEA3269-8E1A-48d3-AF29-D0C3A182A1AB", "Export failed.");
   VERIFY(pInParam != NULL);

   // The progress object is used to report progress to the user
   mpProgress = pInParam->getPlugInArgValue<Progress>(Executable::ProgressArg());
   VERIFY(mpProgress != NULL);

   // This is the filename to export to
   mpDestination = pInParam->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   VERIFY(mpDestination != NULL);
   mpDestination->addToMessageLog(pStep.get());

   const Filename& filename = mpDestination->getFilename();

   // This is the cube to export data from.
   mpRaster = pInParam->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
   VERIFY(mpRaster != NULL);

   // This DynamicObject has whatever the importer for this cube wanted to put in it.
   // If it was imported as a NITF, it has whatever our NITF importer placed in it, and it should
   // get the file/image header data and TREs from it
   const DynamicObject* pMetadata = mpRaster->getMetadata();

   const string prefix = "imagewriter.";
   ossimKeywordlist kwl;

   kwl.add(prefix.c_str(), ossimKeywordNames::TYPE_KW, "ossimImageFileWriter", true);
   string fn = filename.getFullPathAndName().c_str();
   kwl.add(prefix.c_str(), ossimKeywordNames::FILENAME_KW, fn.c_str());

   ossimRefPtr<ossimImageWriter> pOiw = ossimImageWriterFactoryRegistry::instance()->createWriter(kwl, prefix.c_str());
   ossimNitfWriter* pFileWriter = PTR_CAST(ossimNitfWriter, pOiw.get());
   VERIFY(pFileWriter != NULL);

   ossimNitfFileHeaderV2_1* pFileHeader = pFileWriter->getFileHeader();
   ossimNitfImageHeaderV2_1* pImageHeader = pFileWriter->getImageHeader();
   VERIFY(pFileHeader != NULL && pImageHeader != NULL);

   OssimAppMemorySource source(*mpRaster, *mpDestination);
   source.enableSource();
   source.initialize();

   auto_ptr<ossimBandSelector> pSelector;
   const vector<DimensionDescriptor>& exportBands = mpDestination->getBands();
   const unsigned int numBands = exportBands.size();
   vector<ossim_uint32> bandList;
   for (ossim_uint32 i = 0; i < numBands; ++i)
   {
      bandList.push_back(i);
   }

   ossimBandSelector* pTempSelector = new ossimBandSelector();
   pSelector = auto_ptr<ossimBandSelector>(pTempSelector);
   VERIFY(pSelector.get() != NULL);
   pSelector->connectMyInputTo(&source);
   pSelector->setOutputBandList(bandList);
   pSelector->enableSource();
   pSelector->initialize();
   pFileWriter->connectMyInputTo(0, pSelector.get());

   pFileWriter->initialize();
   pFileWriter->addListener(this);
   pFileWriter->setWriteImageFlag(true);
   pFileWriter->setWriteOverviewFlag(false);
   pFileWriter->setWriteHistogramFlag(false);
   pFileWriter->setWriteEnviHeaderFlag(false);
   pFileWriter->setWriteExternalGeometryFlag(false);
   pFileWriter->setWriteFgdcFlag(false);
   pFileWriter->setWriteJpegWorldFile(false);
   pFileWriter->setWriteReadme(false);
   pFileWriter->setWriteTiffWorldFile(false);

   pFileWriter->setFilename(fn.c_str());

   string normalMsg = "Writing metadata";
   mpProgress->updateProgress(normalMsg, 0, NORMAL);
   pStep->addMessage(normalMsg, "app", "955E6F10-58C5-4e3c-BD36-17099DF933A6", true);

   bool status = true;
   string errorMessage;
   RasterDataDescriptor* pDd = dynamic_cast<RasterDataDescriptor*>(mpRaster->getDataDescriptor());
   VERIFY(pDd != NULL);

   // VQ exports are unsupported
   const string icomPathName[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
      Nitf::ImageSubheaderFieldNames::COMPRESSION, END_METADATA_NAME };

   string icom;
   const DataVariant& dvCom = pMetadata->getAttributeByPath(icomPathName);


   EncodingType dataType = pDd->getDataType();
   if ((dvCom.getValue(icom) == true) && (icom == "M4" || icom == "C4"))
   {
      errorMessage = "NITF export for VQ compression is unsupported. Please use another exporter.";
      status = false;
   }
   else if (dataType == INT4SCOMPLEX || dataType == FLT8COMPLEX)
   {
      errorMessage = "NITF export for complex numbers is unsupported. Please use another exporter.";
      status = false;
   }
   else if (validateExportDescriptor(mpDestination, errorMessage) == false)
   {
      status = false;
   }
   else if (Nitf::exportMetadata(pDd, mpDestination, pFileWriter, mpProgress) == false)
   {
      errorMessage = "NITF metadata export failed";
      status = false;
   }
   else
   {
      // This is the view to export
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pInParam->getPlugInArgValue<View>(Executable::ViewArg()));
      if (pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL && pLayerList->getPrimaryRasterElement() == mpRaster)
         {
            // Set the file background color.
            ColorType backgroundColor = pView->getBackgroundColor();
            pFileHeader->setFileBackgroundColor(backgroundColor.mRed,
               backgroundColor.mGreen, backgroundColor.mBlue);

            // Set the band representations. Note that this only supports single-image export.
            pImageHeader->setNumberOfBands(numBands);

            // Set bits per pixel to 0 to force calculation later in ossimNitfWriter.
            // This is done in order to have the capability to export NITF images that
            // have changed data types since being imported or to export result data
            // that copied metadata from the source where the source happened to be NITF.
            pImageHeader->setBitsPerPixel(0);

            mpRasterLayer = dynamic_cast<RasterLayer*>(pLayerList->getLayer(RASTER, mpRaster));
            VERIFY(mpRasterLayer != NULL);

            // Note that some special cases which are undefined in the spec could occur here:
            // 1) Some (but not all) of Red, Green, and Blue bands are defined.
            // 2) Both RGB and MONO layers are valid (in this case MONO is ignored).
            DimensionDescriptor redBand;
            DimensionDescriptor greenBand;
            DimensionDescriptor blueBand;
            DimensionDescriptor grayBand;
            if (mpRasterLayer->getDisplayMode() == RGB_MODE)
            {
               if (mpRasterLayer->getDisplayedRasterElement(RED) == mpRaster)
               {
                  redBand = mpRasterLayer->getDisplayedBand(RED);
               }

               if (mpRasterLayer->getDisplayedRasterElement(GREEN) == mpRaster)
               {
                  greenBand = mpRasterLayer->getDisplayedBand(GREEN);
               }

               if (mpRasterLayer->getDisplayedRasterElement(BLUE) == mpRaster)
               {
                  blueBand = mpRasterLayer->getDisplayedBand(BLUE);
               }
            }
            else if (mpRasterLayer->getDisplayedRasterElement(GRAY) == mpRaster)
            {
               grayBand = mpRasterLayer->getDisplayedBand(GRAY);
            }

            vector<string> isubcats;
            vector<double> centerWavelengths;
            const string centerWavelengthsPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
               CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
            if (pMetadata->getAttributeByPath(centerWavelengthsPath).getValue(centerWavelengths) == true)
            {
               for (vector<double>::const_iterator iter = centerWavelengths.begin();
                  iter != centerWavelengths.end();
                  ++iter)
               {
                  // Per MIL-STD-2500C, the ISUBCAT field is in nm and cannot be more than 999999.
                  const double isubcat = *iter * 1000.0;
                  if (isubcat > 999999)
                  {
                     isubcats.push_back(string());
                  }
                  else
                  {
                     isubcats.push_back(StringUtilities::toDisplayString(isubcat));
                  }
               }
            }
            else
            {
               const string isubcatPath[] = { Nitf::NITF_METADATA, Nitf::IMAGE_SUBHEADER,
                  Nitf::ImageSubheaderFieldNames::BAND_SIGNIFICANCES, END_METADATA_NAME };
               pMetadata->getAttributeByPath(isubcatPath).getValue(isubcats);
            }

            for (unsigned int band = 0; band < numBands; ++band)
            {
               if (exportBands[band].isOriginalNumberValid() == true)
               {
                  ossimNitfImageBandV2_1 imageBand;
                  const unsigned int originalNumber = exportBands[band].getOriginalNumber();
                  if (redBand.isOriginalNumberValid() == true && originalNumber == redBand.getOriginalNumber())
                  {
                     imageBand.setBandRepresentation(Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_RED);
                  }
                  else if (greenBand.isOriginalNumberValid() == true && originalNumber == greenBand.getOriginalNumber())
                  {
                     imageBand.setBandRepresentation(Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_GREEN);
                  }
                  else if (blueBand.isOriginalNumberValid() == true && originalNumber == blueBand.getOriginalNumber())
                  {
                     imageBand.setBandRepresentation(Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_BLUE);
                  }
                  else if (grayBand.isOriginalNumberValid() == true && originalNumber == grayBand.getOriginalNumber())
                  {
                     imageBand.setBandRepresentation(Nitf::ImageSubheaderFieldValues::BAND_REPRESENTATIONS_MONO);
                  }

                  if (isubcats.size() > originalNumber)
                  {
                     imageBand.setBandSignificance(isubcats[originalNumber]);
                  }

                  pImageHeader->setBandInfo(band, imageBand);
               }
            }
         }
      }

      status = exportClassification(pInParam, pDd->getClassification(), pFileHeader, pImageHeader, errorMessage);
      if (status)
      {
         if (pFileWriter->execute() == false)
         {
            errorMessage = "Error writing the NITF file";
            status = false;
         }
      }
   }

   pFileWriter->removeListener(this);

   if (status) 
   {
      mpProgress->updateProgress("Export complete", 100, NORMAL);
      pStep->finalize(Message::Success);
   }
   else
   {
      mpProgress->updateProgress(errorMessage, 100, ERRORS);
      pStep->addMessage(errorMessage, "app", "955E6F10-58C5-4e3c-BD36-17099DF933A6", true);
      pStep->finalize(Message::Failure);
   }

   return status;
}

Progress *Nitf::NitfExporter::getProgress() 
{
   return mpProgress;
}

RasterElement *Nitf::NitfExporter::getRasterElement()
{
   return mpRaster;
}

void Nitf::NitfExporter::processEvent(ossimEvent &event)
{
   if (event.canCastTo("ossimProcessProgressEvent"))
   {
      ossimProcessProgressEvent* pProgressEvent = PTR_CAST(ossimProcessProgressEvent, &event);
      VERIFYNRV(pProgressEvent != NULL);

      Progress* pProgress = getProgress();
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Exporting data", pProgressEvent->getPercentComplete(), NORMAL);
      }
   }
}

bool Nitf::NitfExporter::exportClassification(const PlugInArgList* pArgList, const Classification* pClassification,
   ossimNitfFileHeaderV2_1* pFileHeader, ossimNitfImageHeaderV2_1* pImageHeader, string& errorMessage)
{
   VERIFY(pArgList != NULL && pClassification != NULL && pFileHeader != NULL && pImageHeader != NULL);

   // Get all the initial values from pClassification
   string level = pClassification->getLevel();
   string system = pClassification->getSystem();
   string codewords = pClassification->getCodewords();
   string controlHandling = pClassification->getFileControl();
   string releasing = pClassification->getFileReleasing();
   string declassType = pClassification->getDeclassificationType();
   string declassDate;
   const DateTime* pDateTime = pClassification->getDeclassificationDate();
   if (pDateTime != NULL)
   {
      declassDate = pDateTime->getFormattedUtc("%Y%m%d");
   }

   string declassExempt = pClassification->getDeclassificationExemption();
   string downgrade = pClassification->getFileDowngrade();
   string downgradingDate;
   pDateTime = pClassification->getDowngradeDate();
   if (pDateTime != NULL)
   {
      downgradingDate = pDateTime->getFormattedUtc("%Y%m%d");
   }

   string classificationAuthorityType = pClassification->getAuthorityType();
   string classificationAuthority = pClassification->getAuthority();
   string classificationReason = pClassification->getClassificationReason();
   string securitySourceDate;
   pDateTime = pClassification->getSecuritySourceDate();
   if (pDateTime != NULL)
   {
      securitySourceDate = pDateTime->getFormattedUtc("%Y%m%d");
   }

   string securityControlNumber = pClassification->getSecurityControlNumber();
   string copyNumber = pClassification->getFileCopyNumber();
   string numberOfCopies = pClassification->getFileNumberOfCopies();
   string classificationText = pClassification->getDescription();

   // Check that the classification is valid
   string err;
   if (Nitf::isClassificationValidForExport(*pClassification, err) == false)
   {
      if (getExportOptionsWidget(pArgList) == NULL)
      {
         VERIFY(isBatch() == true);
         bool classificationMustBeValid;
         VERIFY(pArgList->getPlugInArgValue<bool>("Classification Must Be Valid", classificationMustBeValid) == true);
         if (classificationMustBeValid)
         {
            errorMessage = "Classification is not valid.";
            return false;
         }
      }
      else
      {
         // Get the user-modified values for each classification field
         level = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::LEVEL);
         system = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::SYSTEM);
         codewords = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::CODEWORDS);
         controlHandling = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::FILE_CONTROL);
         releasing = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::FILE_RELEASING);
         declassType = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::DECLASSIFICATION_TYPE);
         declassDate = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::DECLASSIFICATION_DATE);
         declassExempt = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::DECLASSIFICATION_EXEMPTION);
         downgrade = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::FILE_DOWNGRADE);
         downgradingDate = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::DOWNGRADE_DATE);
         classificationAuthorityType = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::AUTHORITY_TYPE);
         classificationAuthority = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::AUTHORITY);
         classificationReason = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::CLASSIFICATION_REASON);
         securitySourceDate = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::SOURCE_DATE);
         securityControlNumber = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::SECURITY_CONTROL_NUMBER);
         copyNumber = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::FILE_COPY_NUMBER);
         numberOfCopies = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::FILE_NUMBER_OF_COPIES);
         classificationText = mpOptionsWidget->getModifiedValue(OptionsNitfExporter::DESCRIPTION);
      }
   }

   // Only set the classification markings if the level is not unclassified.
   // If no markings are set, OSSIM sets all fields to their appropriate unclassified levels by default.
   // Note that all ImageHeader markings are copied from the file header manually by this method.
   // DES markings are copied from the file header by OSSIM.
   if (level != "U")
   {
      pFileHeader->setFileSecurityClassification(level);
      pFileHeader->setSecurityClassificationSys(system);
      pFileHeader->setCodeWords(codewords);
      pFileHeader->setControlAndHandling(controlHandling);
      pFileHeader->setReleasingInstructions(releasing);
      pFileHeader->setDeclassificationType(declassType);
      pFileHeader->setDeclassificationDate(declassDate);
      pFileHeader->setDeclassificationExemption(declassExempt);
      pFileHeader->setDowngrade(downgrade);
      pFileHeader->setDowngradingDate(downgradingDate);
      pFileHeader->setClassificationAuthorityType(classificationAuthorityType);
      pFileHeader->setClassificationAuthority(classificationAuthority);
      pFileHeader->setClassificationReason(classificationReason);
      pFileHeader->setSecuritySourceDate(securitySourceDate);
      pFileHeader->setSecurityControlNumber(securityControlNumber);
      pFileHeader->setCopyNumber(copyNumber);
      pFileHeader->setNumberOfCopies(numberOfCopies);
      pFileHeader->setClassificationText(classificationText);

      pImageHeader->setSecurityClassification(level);
      pImageHeader->setSecurityClassificationSystem(system);
      pImageHeader->setCodewords(codewords);
      pImageHeader->setControlAndHandling(controlHandling);
      pImageHeader->setReleasingInstructions(releasing);
      pImageHeader->setDeclassificationType(declassType);
      pImageHeader->setDeclassificationDate(declassDate);
      pImageHeader->setDeclassificationExempt(declassExempt);
      pImageHeader->setDowngrade(downgrade);
      pImageHeader->setDowngradeDate(downgradingDate);
      pImageHeader->setClassificationAuthorityType(classificationAuthorityType);
      pImageHeader->setClassificationAuthority(classificationAuthority);
      pImageHeader->setClassificationReason(classificationReason);
      pImageHeader->setSecuritySourceDate(securitySourceDate);
      pImageHeader->setSecurityControlNumber(securityControlNumber);
      pImageHeader->setClassificationText(classificationText);
   }

   return true;
}

ValidationResultType Nitf::NitfExporter::validate(const PlugInArgList* pArgList, string& errorMessage) const
{
   VERIFYRV(pArgList != NULL, VALIDATE_FAILURE);
   const RasterFileDescriptor* pDescriptor = pArgList->getPlugInArgValue<RasterFileDescriptor>(Exporter::ExportDescriptorArg());
   if (validateExportDescriptor(pDescriptor, errorMessage) == false)
   {
      return VALIDATE_FAILURE;
   }

   // Get the OptionsExportWidget. If this function returns non-NULL, then mpOptionsWidget is safe to use.
   if (const_cast<Nitf::NitfExporter*>(this)->getExportOptionsWidget(pArgList) == NULL)
   {
      // If unable to obtain the widget, then this should be running in batch mode
      VERIFYRV(isBatch() == true, VALIDATE_FAILURE);

      const RasterElement* pRasterElement = pArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
      if (pRasterElement == NULL)
      {
         errorMessage = "Unable to obtain the Raster Element.";
         return VALIDATE_FAILURE;
      }

      const DataDescriptor* pDataDescriptor = pRasterElement->getDataDescriptor();
      if (pDataDescriptor == NULL)
      {
         errorMessage = "Unable to obtain the Data Descriptor.";
         return VALIDATE_FAILURE;
      }

      // Query "Classification Must Be Valid" to determine whether or not the classification markings should be checked
      bool classificationMustBeValid;
      VERIFYRV(pArgList->getPlugInArgValue<bool>("Classification Must Be Valid",
         classificationMustBeValid) == true, VALIDATE_FAILURE);
      if (classificationMustBeValid == false)
      {
         return VALIDATE_SUCCESS;
      }

      const Classification* pClassification = pDataDescriptor->getClassification();
      if (pClassification == NULL)
      {
         errorMessage = "Unable to obtain the Classification.";
         return VALIDATE_FAILURE;
      }

      if (Nitf::isClassificationValidForExport(*pClassification, errorMessage) == false)
      {
         errorMessage = "Invalid classification markings.\n" + errorMessage;
         return VALIDATE_FAILURE;
      }
   }
   else if (mpOptionsWidget->isValid(errorMessage) == false)
   {
      errorMessage += "To accept these values, click \"Ok\".\n"
         "To modify these values, click \"Options\".\n"
         "To cancel the export, click \"Cancel\".";
      return VALIDATE_INFO;
   }

   return VALIDATE_SUCCESS;
}

QWidget* Nitf::NitfExporter::getExportOptionsWidget(const PlugInArgList* pInArgList)
{
   if (isBatch() == true)
   {
      return NULL;
   }

   if (mpOptionsWidget.get() == NULL)
   {
      const RasterElement* pRasterElement = pInArgList->getPlugInArgValue<RasterElement>(Exporter::ExportItemArg());
      VERIFYRV(pRasterElement != NULL, NULL);

      const DataDescriptor* pDataDescriptor = pRasterElement->getDataDescriptor();
      VERIFYRV(pDataDescriptor != NULL, NULL);

      const Classification* pClassification = pDataDescriptor->getClassification();
      VERIFYRV(pClassification != NULL, NULL);

      OptionsNitfExporter* pOptions = new OptionsNitfExporter(pClassification);
      mpOptionsWidget.reset(pOptions);
      VERIFYRV(mpOptionsWidget.get() != NULL, NULL);
   }

   return mpOptionsWidget.get();
}

bool Nitf::NitfExporter::validateExportDescriptor(const RasterFileDescriptor* pDescriptor, string& errorMessage) const
{
   VERIFY(pDescriptor != NULL);
   if (pDescriptor->getColumnCount() > 204800 || pDescriptor->getRowCount() > 204800)
   {
      errorMessage += "NITF export of more than 204,800 rows or columns is unsupported.\n";
      return false;
   }

   return true;
}
