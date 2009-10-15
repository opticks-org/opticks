/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "Filename.h"
#include "GeoPoint.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "MathUtil.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "ResultsExporter.h"
#include "ResultsOptionsWidget.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "ThresholdLayer.h"

#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksResults, ResultsExporter);

ResultsExporter::ResultsExporter() :
   mbInteractive(false),
   mbAbort(false),
   mpProgress(NULL),
   mpResults(NULL),
   mpFileDescriptor(NULL),
   mFirstThreshold(0.0),
   mSecondThreshold(0.0),
   mPassArea(MIDDLE),
   mGeocoordType(GEOCOORD_LATLON),
   mbMetadata(true),
   mbAppendFile(false),
   mpOptionsWidget(NULL),
   mpStep(NULL)
{
   setName("Results Exporter");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Results Exporter");
   setDescription("Exports a results raster element to a file with various options");
   setExtensions("Results Files (*.rls)");
   setSubtype(TypeConverter::toString<RasterElement>());
   setDescriptorId("{DE224774-8638-4aca-BAB8-4AE1F07E328E}");
   allowMultipleInstances(true);
}

ResultsExporter::~ResultsExporter()
{
   delete mpOptionsWidget;
}

QWidget* ResultsExporter::getExportOptionsWidget(const PlugInArgList *pInArgList)
{
   const DataDescriptor* pDescriptor = NULL;
   if (pInArgList != NULL)
   {
      RasterElement* pElement = pInArgList->getPlugInArgValue<RasterElement>(ExportItemArg());
      if (pElement != NULL)
      {
         pDescriptor = pElement->getDataDescriptor();
      }
   }
   if (mpOptionsWidget == NULL)
   {
      Service<DesktopServices> pDesktop;
      VERIFY(pDesktop.get() != NULL);

      mpOptionsWidget = new ResultsOptionsWidget(pDesktop->getMainWidget());
   }

   if (mpOptionsWidget != NULL)
   {
      const string& name = pDescriptor->getName();
      const string& type = pDescriptor->getType();
      DataElement* pParent = pDescriptor->getParent();

      RasterElement* pResults = dynamic_cast<RasterElement*>(mpModel->getElement(name, type, pParent));
      if (pResults != NULL)
      {
         PassArea passArea = MIDDLE;
         double dFirstThreshold = 0.0;
         double dSecondThreshold = 0.0;

         SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(mpDesktop->getCurrentWorkspaceWindow());
         if (pWindow != NULL)
         {
            SpatialDataView* pView = pWindow->getSpatialDataView();
            if (pView != NULL)
            {
               LayerList* pLayerList = pView->getLayerList();
               if (pLayerList != NULL)
               {
                  ThresholdLayer* pThresholdLayer =
                     static_cast<ThresholdLayer*>(pLayerList->getLayer(THRESHOLD, pResults));
                  if (pThresholdLayer != NULL)
                  {
                     passArea = pThresholdLayer->getPassArea();
                     dFirstThreshold = pThresholdLayer->getFirstThreshold();
                     dSecondThreshold = pThresholdLayer->getSecondThreshold();
                  }
                  else
                  {
                     Statistics* pStatistics = pResults->getStatistics();
                     if (pStatistics != NULL)
                     {
                        dFirstThreshold = pStatistics->getMin();
                        dSecondThreshold = pStatistics->getMax();
                     }
                  }
               }

               LatLonLayer* pLatLonLayer = static_cast<LatLonLayer*>(pView->getTopMostLayer(LAT_LONG));
               if (pLatLonLayer != NULL)
               {
                  GeocoordType geocoordType = pLatLonLayer->getGeocoordType();
                  mpOptionsWidget->setGeocoordType(geocoordType);
               }
            }
         }

         mpOptionsWidget->setPassArea(passArea);
         mpOptionsWidget->setFirstThreshold(dFirstThreshold);
         mpOptionsWidget->setSecondThreshold(dSecondThreshold);
      }
   }

   return mpOptionsWidget;
}

bool ResultsExporter::setBatch()
{
   mbInteractive = false;
   return true;
}

bool ResultsExporter::setInteractive()
{
   mbInteractive = true;
   return true;
}

bool ResultsExporter::hasAbort()
{
   return true;
}

bool ResultsExporter::getInputSpecification(PlugInArgList*& pArgList)
{
   // Set up list
   pArgList = mpPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ProgressArg());
   pArg->setType("Progress");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportItemArg());
   pArg->setType("RasterElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = mpPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName(ExportDescriptorArg());
   pArg->setType("RasterFileDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   if (!mbInteractive)
   {
      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Pass Area");
      pArg->setType("PassArea");
      pArg->setDefaultValue(&mPassArea);
      pArgList->addArg(*pArg);

      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("First Threshold");
      pArg->setType("double");
      pArg->setDefaultValue(&mFirstThreshold);
      pArgList->addArg(*pArg);

      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Second Threshold");
      pArg->setType("double");
      pArg->setDefaultValue(&mSecondThreshold);
      pArgList->addArg(*pArg);

      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Geocoordinate Type");
      pArg->setType("GeocoordType");
      pArg->setDefaultValue(&mGeocoordType);
      pArgList->addArg(*pArg);

      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Metadata");
      pArg->setType("bool");
      pArg->setDefaultValue(&mbMetadata);
      pArgList->addArg(*pArg);

      pArg = mpPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Append To File");
      pArg->setType("bool");
      pArg->setDefaultValue(&mbAppendFile);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool ResultsExporter::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ResultsExporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute results exporter", "app", "ABF9EDE4-4672-4361-86BB-3258ADFB0793");
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      return false;
   }

   // Check for complex data
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpResults->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      mMessage = "Could not get the data descriptor from the results matrix!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   EncodingType eDataType = pDescriptor->getDataType();
   if ((eDataType == INT4SCOMPLEX) || (eDataType == FLT8COMPLEX))
   {
      mMessage = "Cannot save complex data in text format!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (pDescriptor->getBandCount() > 1)
   {
      mMessage = "Can only export single band data.";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   pDescriptor->addToMessageLog(pStep.get());

   const string& filename = mpFileDescriptor->getFilename();

   pStep->addProperty("filename", filename);
   pStep->addProperty("firstThreshold", mFirstThreshold);
   pStep->addProperty("secondThreshold", mSecondThreshold);
   pStep->addProperty("passArea", mPassArea);
   pStep->addProperty("geocoordType", mGeocoordType);
   pStep->addProperty("metadata", mbMetadata);
   pStep->addProperty("appendFile", mbAppendFile);

   DataElement* pParent = mpResults->getParent();
   if (pParent != NULL)
   {
      pStep->addProperty("sourceDataSet", pParent->getName());
   }

   ofstream fileOutput;
   fileOutput.open(filename.c_str(), mbAppendFile ? ios_base::app : ios_base::trunc);

   if (!fileOutput.is_open())
   {
      mMessage = "Could not open the output file for writing!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (!writeOutput(fileOutput))
   {
      if (mbAbort)
      {
         pStep->finalize(Message::Abort);
      }
      fileOutput.close();
      remove(filename.c_str());
   }

   mMessage = "Results matrix export complete!";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 100, NORMAL);
   }

   pStep->finalize(Message::Success);
   return true;
}

bool ResultsExporter::writeOutput(ostream &stream)
{
   mMessage = "Exporting results matrix...";
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(mMessage, 0, NORMAL);
   }

   StepResource pStep(mMessage, "app", "D890E37C-B960-4527-8AAC-D62F2DE7A541");

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpResults->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      mMessage = "Could not get the results data descriptor!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure);
      return false;
   }

   VERIFY(mpResults != NULL);
   string name = mpResults->getName();

   VERIFY(mpFileDescriptor != NULL);
   const vector<DimensionDescriptor>& rows = mpFileDescriptor->getRows();
   const vector<DimensionDescriptor>& columns = mpFileDescriptor->getColumns();
   unsigned int numRows = pDescriptor->getRowCount();
   unsigned int numColumns = pDescriptor->getColumnCount();
   EncodingType eDataType = pDescriptor->getDataType();
   const vector<int>& badValues = pDescriptor->getBadValues();

   if (mbMetadata)
   {
      stream << APP_NAME << " Results Raster\n";
      stream << "Version = 4\n";
      stream << "Results Name = " << name << "\n";

      DataElement* pParent = mpResults->getParent();
      if (pParent != NULL)
      {
         stream << "Data Set Name = " << pParent->getName() << "\n";
      }

      stream << "Rows = " << numRows << "\n";
      stream << "Columns = " << numColumns << "\n";

      string dataType = StringUtilities::toDisplayString(eDataType);
      stream << "Data Type = " << dataType << "\n";

      Statistics* pStatistics = mpResults->getStatistics();
      if (pStatistics != NULL)
      {
         stream << "Min = " << pStatistics->getMin() << "\n";
         stream << "Max = " << pStatistics->getMax() << "\n";
         stream << "Average = " << pStatistics->getAverage() << "\n";
         stream << "Standard Deviation = " << pStatistics->getStandardDeviation() << "\n\n";
      }
   }

   RasterElement* pGeo = getGeoreferencedRaster();

   DataAccessor da = mpResults->getDataAccessor();
   if (!da.isValid())
   {
      mMessage = "Could not access the data in the results raster!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      pStep->finalize(Message::Failure);
      return false;
   }

   unsigned int activeRowNumber = 0;
   for (unsigned int r = 0; r < rows.size(); ++r)
   {
      if (mbAbort)
      {
         mMessage = "Results exporter aborted!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ABORT);
         }

         pStep->finalize(Message::Abort);
         return false;
      }

      DimensionDescriptor rowDim = rows[r];
      // Skip to the next row
      for (; activeRowNumber < rowDim.getActiveNumber(); ++activeRowNumber)
      {
         da->nextRow();
      }

      unsigned int activeColumnNumber = 0;
      for (unsigned int c = 0; c < columns.size(); ++c)
      {
         DimensionDescriptor columnDim = columns[c];
         // Skip to the next column
         for (; activeColumnNumber < columnDim.getActiveNumber(); ++activeColumnNumber)
         {
            da->nextColumn();
         }

         VERIFY(da.isValid());

         double dValue = mpModel->getDataValue(eDataType, da->getColumn(), COMPLEX_MAGNITUDE, 0);
         if (isValueExported(dValue, badValues))
         {
            string location = getLocationString(r, c, pGeo);
            char buffer[1024];
            sprintf(buffer, "%lf\n", dValue);
            stream << name << "    " << location << "    " << buffer;
         }
      }

      // Update the progress
      int iProgress = (r * 100) / rows.size();
      if (iProgress == 100)
      {
         iProgress = 99;
      }

      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, iProgress, NORMAL);
      }
   }

   stream << "\n";
   return true;
}

bool ResultsExporter::abort()
{
   mbAbort = true;
   return true;
}

bool ResultsExporter::extractInputArgs(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      mMessage = "The input arg list is invalid!";
      return false;
   }

   PlugInArg* pArg = NULL;

   // Progress
   if (pArgList->getArg(ProgressArg(), pArg) && (pArg != NULL))
   {
      mpProgress = pArg->getPlugInArgValue<Progress>();
   }

   // Results matrix
   if (pArgList->getArg(ExportItemArg(), pArg) && (pArg != NULL))
   {
      mpResults = pArg->getPlugInArgValue<RasterElement>();
   }
   else
   {
      mMessage = "Could not read the results input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (mpResults == NULL)
   {
      mMessage = "The results input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   // File descriptor
   if (pArgList->getArg(ExportDescriptorArg(), pArg) && (pArg != NULL))
   {
      mpFileDescriptor = pArg->getPlugInArgValue<RasterFileDescriptor>();
   }
   else
   {
      mMessage = "Could not read the file descriptor input value!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (mpFileDescriptor == NULL)
   {
      mMessage = "The file descriptor input value is invalid!";
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(mMessage, 0, ERRORS);
      }

      mpStep->finalize(Message::Failure, mMessage);
      return false;
   }

   if (mbInteractive)
   {
      // Get the options from the options widget
      if (mpOptionsWidget == NULL)
      {
         // Create the dialog, which sets the default values to those in the results matrix
         PlugInArgList* pInArgList = NULL;
         if (getInputSpecification(pInArgList) && (pInArgList != NULL))
         {
            pInArgList->setPlugInArgValue(ExportItemArg(), mpResults);
         }
         getExportOptionsWidget(pInArgList);
      }

      if (mpOptionsWidget != NULL)
      {
         mFirstThreshold = mpOptionsWidget->getFirstThreshold();
         mSecondThreshold = mpOptionsWidget->getSecondThreshold();
         mPassArea = mpOptionsWidget->getPassArea();
         mGeocoordType = mpOptionsWidget->getGeocoordType();
         mbMetadata = mpOptionsWidget->isMetadataEnabled();
         mbAppendFile = mpOptionsWidget->appendToFile();
      }
   }
   else
   {
      // Pass area
      if (!pArgList->getArg("Pass Area", pArg) || (pArg == NULL))
      {
         mMessage = "Could not read the pass area input value!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      PassArea* pPassArea = pArg->getPlugInArgValue<PassArea>();
      if (pPassArea != NULL)
      {
         mPassArea = *pPassArea;
      }

      // First threshold
      if (!pArgList->getArg("First Threshold", pArg) || (pArg == NULL))
      {
         mMessage = "Could not read the first threshold input value!";
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress(mMessage, 0, ERRORS);
         }

         mpStep->finalize(Message::Failure, mMessage);
         return false;
      }

      double* pFirstThreshold = pArg->getPlugInArgValue<double>();
      if (pFirstThreshold != NULL)
      {
         mFirstThreshold = *pFirstThreshold;
      }

      // Second threshold
      if ((mPassArea == MIDDLE) || (mPassArea == OUTSIDE))
      {
         if (!pArgList->getArg("Second Threshold", pArg) || (pArg == NULL))
         {
            mMessage = "Could not read the second threshold input value!";
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress(mMessage, 0, ERRORS);
            }

            mpStep->finalize(Message::Failure, mMessage);
            return false;
         }

         double* pSecondThreshold = pArg->getPlugInArgValue<double>();
         if (pSecondThreshold != NULL)
         {
            mSecondThreshold = *pSecondThreshold;
         }
      }

      // Geocoord type
      if (pArgList->getArg("Geocoordinate Type", pArg) && (pArg != NULL))
      {
         GeocoordType* pGeocoordType = pArg->getPlugInArgValue<GeocoordType>();
         if (pGeocoordType != NULL)
         {
            mGeocoordType = *pGeocoordType;
         }
      }

      // Metadata
      if (pArgList->getArg("Metadata", pArg) && (pArg != NULL))
      {
         bool* pMetadata = pArg->getPlugInArgValue<bool>();
         if (pMetadata != NULL)
         {
            mbMetadata = *pMetadata;
         }
      }

      // Append
      if (pArgList->getArg("Append To File", pArg) && (pArg != NULL))
      {
         bool* pAppend = pArg->getPlugInArgValue<bool>();
         if (pAppend != NULL)
         {
            mbAppendFile = *pAppend;
         }
      }
   }

   return true;
}

bool ResultsExporter::isValueExported(double dValue, const vector<int>& badValues) const
{
   bool bExported = false;

   if (find(badValues.begin(), badValues.end(), roundDouble(dValue)) != badValues.end())
   {
      return false;
   }

   switch (mPassArea)
   {
      case LOWER:
         if (dValue < mFirstThreshold)
         {
            bExported = true;
         }
         break;

      case UPPER:
         if (dValue > mFirstThreshold)
         {
            bExported = true;
         }
         break;

      case MIDDLE:
         if ((dValue > mFirstThreshold) && (dValue < mSecondThreshold))
         {
            bExported = true;
         }
         break;

      case OUTSIDE:
         if ((dValue < mFirstThreshold) || (dValue > mSecondThreshold))
         {
            bExported = true;
         }
         break;

      default:
         break;
   }

   return bExported;
}

RasterElement* ResultsExporter::getGeoreferencedRaster() const
{
   if (mpResults == NULL)
   {
      return NULL;
   }

   // First check if this RasterElement is georeferenced
   if (mpResults->isGeoreferenced())
   {
      return mpResults;
   }
   // Next, assume this is a "Results matrix" and check the parent
   RasterElement* pParent = dynamic_cast<RasterElement*>(mpResults->getParent());
   if (pParent != NULL && pParent->isGeoreferenced())
   {
      return pParent;
   }
   // Finally, give up...there's no geodata
   return NULL;
}

string ResultsExporter::getLocationString(unsigned int uiRow, unsigned int uiColumn, const RasterElement* pGeo) const
{
   char buffer[1024];
   sprintf(buffer, "Pixel: (%u, %u)", uiColumn + 1, uiRow + 1);

   string location = buffer;
   if (pGeo != NULL && pGeo->isGeoreferenced())
   {
      LocationType pixelCoord;
      pixelCoord.mX = uiColumn;
      pixelCoord.mY = uiRow;

      LocationType coords = pGeo->convertPixelToGeocoord(pixelCoord);

      LatLonPoint latLonPoint(coords);
      if (mGeocoordType == GEOCOORD_UTM)
      {
         UtmPoint utmPoint(latLonPoint);
         string utmText = utmPoint.getText();
         location = "UTM: (" + utmText + ")";
      }
      else if (mGeocoordType == GEOCOORD_MGRS)
      {
         MgrsPoint mgrsPoint(latLonPoint);
         string mgrsText = mgrsPoint.getText();
         location = "MGRS: (" + mgrsText + ")";
      }
      else if (mGeocoordType == GEOCOORD_LATLON)
      {
         string latLonText = latLonPoint.getText();
         location = "Geo: (" + latLonText + ")";
      }
   }

   return location;
}

bool ResultsExporter::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return runAllTests(pProgress, failure);
}

bool ResultsExporter::runAllTests(Progress *pProgress, ostream& failure)
{
   bool success = true;

   mpProgress = pProgress;
   mpResults = RasterUtilities::createRasterElement("ResultsExporterTestMatrix", 2, 2, FLT4BYTES, true, NULL);
   if (mpResults == NULL)
   {
      failure << "Unable to create a Raster Element.";
      success = false;
   }
   else
   {
      ModelResource<RasterElement> pResultsResource(mpResults);
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(mpResults->getDataDescriptor());
      if (pDescriptor == NULL)
      {
         failure << "Unable to get the Raster Data Descriptor.";
         success = false;
      }
      else
      {
         // Bad values
         std::vector<int> badValues;
         badValues.push_back(2);
         badValues.push_back(4);
         pDescriptor->setBadValues(badValues);

         // Create the element
         FactoryResource<DataRequest> pRequest;
         pRequest->setWritable(true);
         DataAccessor da = mpResults->getDataAccessor(pRequest.release());
         if (da.isValid() == false)
         {
            failure << "Data Accessor is not valid.";
            success = false;
         }
         else
         {
            float* pData = reinterpret_cast<float*>(da->getRow());
            if (pData == NULL)
            {
               failure << "Data is not accessible.";
               success = false;
            }
            else
            {
               pData[0] = 1.0;
               pData[1] = 2.0;
               pData[2] = 3.0;
               pData[3] = 4.0;

               mFirstThreshold = 1.5;
               mSecondThreshold = 4.5;
               mPassArea = MIDDLE;
               mGeocoordType = GEOCOORD_LATLON;
               mbMetadata = false;
               mbAppendFile = false;

               const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
               if (pTempPath == NULL)
               {
                  failure << "Unable to get the temporary path from ConfigurationSettings.";
                  success = false;
               }
               else
               {
                  const string filename = pTempPath->getFullPathAndName() + "/ResultsExporterTest.rls";
                  mpFileDescriptor = dynamic_cast<RasterFileDescriptor*>
                     (RasterUtilities::generateFileDescriptorForExport(pDescriptor, filename));
                  if (mpFileDescriptor == NULL)
                  {
                     failure << "Unable to generate a Raster File Descriptor for export.";
                     success = false;
                  }
                  else
                  {
                     FactoryResource<RasterFileDescriptor> pFileDescriptor(mpFileDescriptor);
                     stringstream stream;
                     writeOutput(stream);

                     if (stream.str() != "ResultsExporterTestMatrix    Pixel: (1, 2)    3.000000\n\n")
                     {
                        failure << "Invalid output stream.";
                        success = false;
                     }
                  }
               }
            }
         }
      }
   }

   mpResults = NULL;
   mpProgress = NULL;
   mpFileDescriptor = NULL;
   return success;
}
