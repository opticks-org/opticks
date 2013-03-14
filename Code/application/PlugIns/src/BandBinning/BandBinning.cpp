/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "BadValues.h"
#include "BandBinning.h"
#include "BandBinningDlg.h"
#include "BandBinningUtilities.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProgressTracker.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "Wavelengths.h"

#include <string>

#include <QtCore/QString>
#include <QtGui/QMessageBox>

REGISTER_PLUGIN_BASIC(OpticksBandBinning, BandBinning);

namespace
{
   // Returns true if and only if results exist and were not deleted.
   bool outputExists(const std::string& name, const DataElement* pParent, bool destroyExisting)
   {
      DataElement* pExistingOutput =
         Service<ModelServices>()->getElement(name, TypeConverter::toString<RasterElement>(), pParent);
      if (pExistingOutput == NULL)
      {
         return false;
      }

      if (destroyExisting || QMessageBox::warning(Service<DesktopServices>()->getMainWidget(),
         APP_NAME, "Binning results already exist.\nTo proceed, the results must be deleted. "
         "Do you want to proceed?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
      {
         Service<ModelServices>()->destroyElement(pExistingOutput);
         return false;
      }

      return true;
   }

   // Returns true if and only if the user accepts the dialog (interactive mode only).
   bool getGroupedBandsFromDialog(const RasterDataDescriptor* pDescriptor,
      std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
   {
      // Interactive mode defaults each band to its own bin.
      // While not a very useful default in practice, it initializes the display to something reasonable.
      VERIFY(pDescriptor != NULL);
      if (groupedBands.empty())
      {
         const std::vector<DimensionDescriptor>& bands = pDescriptor->getBands();
         groupedBands.reserve(bands.size());
         for (std::vector<DimensionDescriptor>::const_iterator iter = bands.begin(); iter != bands.end(); ++iter)
         {
            VERIFY(iter->isOriginalNumberValid() == true);
            groupedBands.push_back(std::make_pair(*iter, *iter));
         }
      }

      BandBinningDlg dlg(groupedBands, pDescriptor, Service<DesktopServices>()->getMainWidget());
      if (dlg.exec() != QDialog::Accepted)
      {
         return false;
      }

      groupedBands = dlg.getGroupedBands();
      return true;
   }

   // Returns true if and only if a RasterElement was created either in-memory or on-disk.
   RasterElement* createOutputElement(const std::string& name, unsigned int rowCount, unsigned int columnCount,
      unsigned int bandCount, EncodingType dataType, InterleaveFormatType interleave, DataElement* pParent)
   {
      // Try in-memory first since it is faster to access.
      RasterElement* pOutputElement = RasterUtilities::createRasterElement(name,
         rowCount, columnCount, bandCount, dataType, interleave, true, pParent);
      if (pOutputElement != NULL)
      {
         return pOutputElement;
      }

      // In-memory creation failed, so try on-disk.
      return RasterUtilities::createRasterElement(name,
         rowCount, columnCount, bandCount, dataType, interleave, false, pParent);
   }

   // Returns true if and only if an output window and associated view were created.
   bool createOutputView(RasterElement* pOutputElement)
   {
      VERIFY(pOutputElement != NULL);
      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->createWindow(pOutputElement->getName(), SPATIAL_DATA_WINDOW));
      if (pWindow == NULL)
      {
         return false;
      }

      SpatialDataView* pView = pWindow->getSpatialDataView();
      if (pView == NULL ||
         pView->setPrimaryRasterElement(pOutputElement) == NULL ||
         pView->createLayer(RASTER, pOutputElement) == NULL)
      {
         return false;
      }

      return true;
   }

   // Returns true if and only if the output descriptor can be populated from the source descriptor for the given bins.
   bool populateOutputDataDescriptor(const RasterDataDescriptor* pDescriptor, RasterDataDescriptor* pOutputDescriptor,
      const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
   {
      // Manually copy applicable fields of the source descriptor.
      VERIFY(pDescriptor != NULL && pOutputDescriptor != NULL && groupedBands.empty() == false);
      pOutputDescriptor->setBadValues(pDescriptor->getBadValues());
      pOutputDescriptor->setXPixelSize(pDescriptor->getXPixelSize());
      pOutputDescriptor->setYPixelSize(pDescriptor->getYPixelSize());
      pOutputDescriptor->setUnits(pDescriptor->getUnits());
      pOutputDescriptor->setClassification(pDescriptor->getClassification());

      // Update wavelengths for the output descriptor based on those of the input descriptor and the bins.
      FactoryResource<Wavelengths> pWavelengths;
      if (pWavelengths->initializeFromDynamicObject(pDescriptor->getMetadata(), false) == false)
      {
         return false;
      }

      // If there are no wavelengths to copy, there is no more work to do.
      if (pWavelengths->isEmpty() == true)
      {
         return true;
      }

      // Create the output wavelengths with the same units as the input wavelengths.
      FactoryResource<Wavelengths> pOutputWavelengths;
      pOutputWavelengths->setUnits(pWavelengths->getUnits());

      // Do not copy start or end wavelengths since they cannot be accurately computed.
      // The center wavelength for each group is the mean of center wavelengths in the group.
      if (pWavelengths->hasCenterValues() == true)
      {
         std::vector<double> outputCenterWavelengths;
         outputCenterWavelengths.reserve(groupedBands.size());
         const std::vector<double>& centerWavelengths = pWavelengths->getCenterValues();

         for (std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >::const_iterator iter =
            groupedBands.begin();
            iter != groupedBands.end();
            ++iter)
         {
            double centerWavelength = centerWavelengths[iter->first.getActiveNumber()];
            for (unsigned int activeNumber = iter->first.getActiveNumber() + 1;  // Offset by one due to initialization.
               activeNumber <= iter->second.getActiveNumber();
               ++activeNumber)
            {
               centerWavelength += centerWavelengths[activeNumber];
            }

            outputCenterWavelengths.push_back(centerWavelength /
               (iter->second.getActiveNumber() - iter->first.getActiveNumber() + 1));
         }

         pOutputWavelengths->setCenterValues(outputCenterWavelengths, pOutputWavelengths->getUnits());
      }

      return pOutputWavelengths->applyToDynamicObject(pOutputDescriptor->getMetadata());
   }

   // Returns true if and only if all bad values across all bands to be processed are equivalent or do not exist.
   bool getUniformBadValues(const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands,
      const RasterElement* pElement, std::string& badValuesStr)
   {
      VERIFY(groupedBands.empty() == false);
      VERIFY(pElement != NULL);
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFY(pDescriptor != NULL);

      badValuesStr.clear();
      bool firstIteration = true;
      for (
         std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >::const_iterator iter = groupedBands.begin();
         iter != groupedBands.end();
         ++iter)
      {
         for (unsigned int activeNumber = iter->first.getActiveNumber();
            activeNumber <= iter->second.getActiveNumber();
            ++activeNumber)
         {
            DimensionDescriptor band = pDescriptor->getActiveBand(activeNumber);
            Statistics* pStatistics = pElement->getStatistics(band);
            VERIFY(pStatistics != NULL);
            const BadValues* pBandBadValues = pStatistics->getBadValues();
            const std::string bandBadValuesStr =
               pBandBadValues != NULL ? pBandBadValues->getBadValuesString() : std::string();
            if (firstIteration == true)
            {
               firstIteration = false;
               badValuesStr = bandBadValuesStr;
            }
            else if (badValuesStr != bandBadValuesStr)
            {
               badValuesStr.clear();
               return false;
            }
         }
      }

      return true;
   }

   // Performs band binning on any data where bad values are uniform across all bands.
   // Input can be any interleave; output is in BIP format.
   template<typename T>
   void createGroupedBandsBipUniformBadValues(const T* pJunk, ProgressTracker& progress, const bool& aborted,
      const RasterElement* pElement, RasterElement* pOutputElement,
      const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands,
      const BadValues* pBadValues)
   {
      // Timing tests have shown that this bool improves performance by as much as 25% when no bad values are present.
      const bool zeroBadValues = (pBadValues == NULL || pBadValues->empty());

      VERIFYNRV(pElement != NULL);
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFYNRV(pDescriptor != NULL);
      const unsigned int rowCount = pDescriptor->getRowCount();
      const unsigned int columnCount = pDescriptor->getColumnCount();

      VERIFYNRV(pOutputElement != NULL);
      RasterDataDescriptor* pOutputDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pOutputElement->getDataDescriptor());
      VERIFYNRV(pOutputDescriptor != NULL);
      VERIFYNRV(groupedBands.empty() == false);

      bool falseNegative = false;
      double percent = 0;
      const double step = 99.0 / (groupedBands.size() * rowCount);
      for (std::vector<std::pair<unsigned int, unsigned int> >::size_type i = 0; i < groupedBands.size(); ++i)
      {
         if (aborted == true)
         {
            break;
         }

         // Request all bands to be binned.
         FactoryResource<DataRequest> pRequest;
         pRequest->setBands(groupedBands[i].first, groupedBands[i].second);
         pRequest->setInterleaveFormat(BIP);
         DataAccessor srcAccessor = pElement->getDataAccessor(pRequest.release());
         const unsigned int bandCount =
            groupedBands[i].second.getActiveNumber() - groupedBands[i].first.getActiveNumber() + 1;

         // Output one band at a time.
         FactoryResource<DataRequest> pOutputRequest;
         pOutputRequest->setBands(pOutputDescriptor->getActiveBand(i), DimensionDescriptor());
         pOutputRequest->setInterleaveFormat(BIP);
         pOutputRequest->setWritable(true);
         DataAccessor dstAccessor = pOutputElement->getDataAccessor(pOutputRequest.release());
         Statistics* pStatistics = pOutputElement->getStatistics(pOutputDescriptor->getActiveBand(i));
         VERIFYNRV(pStatistics != NULL);
         pStatistics->setBadValues(pBadValues);

         // Iterate over each pixel, averaging each in turn.
         const std::string text = QString("Creating bin %1 of %2").arg(i + 1).arg(groupedBands.size()).toStdString();
         for (unsigned int row = 0; row < rowCount; ++row)
         {
            if (aborted == true)
            {
               break;
            }

            progress.report(text, percent, NORMAL);
            for (unsigned int column = 0; column < columnCount; ++column)
            {
               VERIFYNR(srcAccessor.isValid());
               VERIFYNR(dstAccessor.isValid());

               double average = 0;  // Use type double instead of T to combat overflow.
               unsigned int goodValueCount = 0;
               const T* pSrc = reinterpret_cast<T*>(srcAccessor->getColumn());
               for (unsigned int band = 0; band < bandCount; ++band)
               {
                  if (zeroBadValues ||
                     pBadValues->isBadValue(static_cast<double>(*pSrc)) == false)
                  {
                     average += static_cast<double>(*pSrc);
                     ++goodValueCount;
                  }

                  ++pSrc;
               }

               T* const pDst = reinterpret_cast<T*>(dstAccessor->getColumn());
               if (goodValueCount == 0)
               {
                  double badValue(0.0);    // default bad value
                  if (pBadValues != NULL)  // unlikely to be NULL since no good values were found, but check to be safe
                  {
                     badValue = pBadValues->getDefaultBadValue();
                  }
                  *pDst = static_cast<T>(badValue);
               }
               else
               {
                  *pDst = static_cast<T>(average / goodValueCount); // Truncates integer types.
                  if (zeroBadValues == false &&                     // pBadValues can't be NULL if zeroBadValues is false
                     pBadValues->isBadValue(static_cast<double>(*pDst)) == true)
                  {
                     // Corner case: the average of one or more good values happened to match a defined bad value.
                     // Flag a warning, keep the pixel set to the bad value, and continue processing.
                     falseNegative = true;
                  }
               }

               srcAccessor->nextColumn();
               dstAccessor->nextColumn();
            }

            srcAccessor->nextRow();
            dstAccessor->nextRow();
            percent += step;
         }
      }

      if (falseNegative == true)
      {
         progress.report("One or more bands contain values which, when averaged, were equivalent to a bad value. "
            "These good values were not modified and will be indistinguishable from any bad values. "
            "Remove bad values and run this algorithm again to correct this problem.", 99, WARNING);
      }
   }

   // Performs band binning on any data.
   // Input can be any interleave; output is in BIP format.
   template<typename T>
   void createGroupedBandsBipNonUniformBadValues(const T* pJunk, ProgressTracker& progress, const bool& aborted,
      const RasterElement* pElement, RasterElement* pOutputElement,
      const std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> >& groupedBands)
   {
      VERIFYNRV(pElement != NULL);
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFYNRV(pDescriptor != NULL);
      const unsigned int rowCount = pDescriptor->getRowCount();
      const unsigned int columnCount = pDescriptor->getColumnCount();

      VERIFYNRV(pOutputElement != NULL);
      RasterDataDescriptor* pOutputDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pOutputElement->getDataDescriptor());
      VERIFYNRV(pOutputDescriptor != NULL);
      VERIFYNRV(groupedBands.empty() == false);

      double percent = 0;
      const double step = 99.0 / (groupedBands.size() * rowCount);
      bool anyBadValuesSet = false;
      for (std::vector<std::pair<unsigned int, unsigned int> >::size_type i = 0; i < groupedBands.size(); ++i)
      {
         if (aborted == true)
         {
            break;
         }

         // Request all bands to be binned.
         FactoryResource<DataRequest> pRequest;
         pRequest->setBands(groupedBands[i].first, groupedBands[i].second);
         pRequest->setInterleaveFormat(BIP);
         DataAccessor srcAccessor = pElement->getDataAccessor(pRequest.release());
         const unsigned int bandCount =
            groupedBands[i].second.getActiveNumber() - groupedBands[i].first.getActiveNumber() + 1;

         // Output one band at a time.
         FactoryResource<DataRequest> pOutputRequest;
         pOutputRequest->setBands(pOutputDescriptor->getActiveBand(i), DimensionDescriptor());
         pOutputRequest->setInterleaveFormat(BIP);
         pOutputRequest->setWritable(true);
         DataAccessor dstAccessor = pOutputElement->getDataAccessor(pOutputRequest.release());

         // Iterate over each pixel, averaging each in turn.
         const std::string text = QString("Creating bin %1 of %2").arg(i + 1).arg(groupedBands.size()).toStdString();
         for (unsigned int row = 0; row < rowCount; ++row)
         {
            if (aborted == true)
            {
               break;
            }

            progress.report(text, percent, NORMAL);
            for (unsigned int column = 0; column < columnCount; ++column)
            {
               VERIFYNR(srcAccessor.isValid());
               VERIFYNR(dstAccessor.isValid());

               double average = 0.0;  // Use type double instead of T to combat overflow.
               unsigned int goodValueCount = 0;
               const T* pSrc = reinterpret_cast<T*>(srcAccessor->getColumn());
               T* const pDst = reinterpret_cast<T*>(dstAccessor->getColumn());
               for (unsigned int band = 0; band < bandCount; ++band)
               {
                  DimensionDescriptor activeBand =
                     pDescriptor->getActiveBand(groupedBands[i].first.getActiveNumber() + band);
                  Statistics* pStatistics = pElement->getStatistics(activeBand);
                  VERIFYNRV(pStatistics != NULL);
                  const BadValues* pBadValues = pStatistics->getBadValues();
                  if (pBadValues != NULL && pBadValues->isBadValue(static_cast<double>(*pSrc)) == true)
                  {
                     // Always writes the most recently encountered (arbitrary choice) bad value into *pDst.
                     *pDst = *pSrc;
                  }
                  else
                  {
                     average += static_cast<double>(*pSrc);
                     ++goodValueCount;
                  }

                  ++pSrc;
               }

               if (goodValueCount == 0)
               {
                  anyBadValuesSet = true;
                  Statistics* pStatistics = pOutputElement->getStatistics(pOutputDescriptor->getActiveBand(i));
                  VERIFYNRV(pStatistics != NULL);
                  BadValues* pBadValues = pStatistics->getBadValues();  // Statistics has BadValuesAdapter so always non-NULL
                  if (pBadValues->empty())
                  {
                     QString badValStr = QString("%1").arg(*pDst);
                     pBadValues->addBadValue(badValStr.toStdString());
                  }
                  else
                  {
                     *pDst = static_cast<T>(pBadValues->getDefaultBadValue());
                  }
               }
               else
               {
                  *pDst = static_cast<T>(average / goodValueCount); // Truncates integer types.
               }

               srcAccessor->nextColumn();
               dstAccessor->nextColumn();
            }

            srcAccessor->nextRow();
            dstAccessor->nextRow();
            percent += step;
         }
      }

      if (anyBadValuesSet == true)
      {
         progress.report("One or more input band groups contained only bad values for a given pixel location. "
            "This value was marked as a bad value and may result in false negatives for good values which, "
            "when averaged, were equivalent to the bad value. "
            "Any good values were not modified and will be indistinguishable from any bad values. "
            "Remove bad values and run this algorithm again to correct this problem.", 99, WARNING);
      }
   }
}

BandBinning::BandBinning()
{
   setName("Band Binning");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setShortDescription("Perform Band Binning and Averaging");
   setDescription("Perform Band Binning and Averaging");
   setMenuLocation("[General Algorithms]\\Band Binning");
   setDescriptorId("{68EF7B21-F97B-4775-9831-6854BCF1AADD}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
}

BandBinning::~BandBinning()
{}

bool BandBinning::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Progress>(Executable::ProgressArg(), Executable::ProgressArgDescription()));
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(),
      "Element on which band binning will be performed"));
   VERIFY(pArgList->addArg<Filename>("Filename",
      "A two-column, space-delimited text file describing how to bin the bands. "
      "Each row in the file describes one band bin. "
      "The first column represents the lower bound of the band bin (zero-based original numbers, inclusive range). "
      "The second column represents the upper bound of the band bin (zero-based original numbers, inclusive range). "
      "In Interactive mode, the file is optional and will be used to initialize the input dialog. "
      "In Batch mode, the file is required and will be used without user confirmation."));
   return true;
}

bool BandBinning::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<RasterElement>(Executable::DataElementArg(),
      "Output element for the result of the band binning operation."));
   return true;
}

bool BandBinning::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);

   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Execute " + getName(), "app", "{E1876086-0AA3-4e51-B8CE-D76D490F8294}");
   progress.report("Executing " + getName(), 0, NORMAL);

   RasterElement* pElement = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (pElement == NULL)
   {
      progress.report("No raster element provided.", 0, ERRORS);
      return false;
   }

   // Check for previous results and other incurable error conditions before displaying the dialog.
   const std::string outputName = pElement->getDisplayName(true) + "_binnedBands";
   if (outputExists(outputName, NULL, isBatch()) == true)
   {
      progress.report("Binning results already exist. "
         "Close the existing results or rename the input raster element and try again. ", 0, ERRORS);
      return false;
   }

   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
   VERIFY(pDescriptor != NULL);
   if (pDescriptor->getDataType() == INT4SCOMPLEX || pDescriptor->getDataType() == FLT8COMPLEX)
   {
      progress.report("Complex data is not supported.", 0, ERRORS);
      return false;
   }

   // Read bins from the input file if one was provided.
   std::vector<std::pair<DimensionDescriptor, DimensionDescriptor> > groupedBands =
      BandBinningUtilities::readFile(pInArgList->getPlugInArgValue<Filename>("Filename"), pDescriptor);

   // Display the dialog if the plug-in is in interactive mode.
   if (isBatch() == false && getGroupedBandsFromDialog(pDescriptor, groupedBands) == false)
   {
      progress.report("Cancelled", 0, ABORT, true);   // Set the "log" flag to get auto-close.
      return false;
   }

   // Make sure that the user specified at least one band group.
   if (groupedBands.empty())
   {
      progress.report("No band groups were specified.", 0, ERRORS);
      return false;
   }

   // Create the output RasterElement.
   ModelResource<RasterElement> pOutputElement(createOutputElement(outputName, pDescriptor->getRowCount(),
      pDescriptor->getColumnCount(), groupedBands.size(), pDescriptor->getDataType(), BIP, NULL));
   if (pOutputElement.get() == NULL)
   {
      progress.report("Unable to create output.", 0, ERRORS);
      return false;
   }

   // Populate the output data descriptor based on values in the input data descriptor.
   if (populateOutputDataDescriptor(pDescriptor,
      dynamic_cast<RasterDataDescriptor*>(pOutputElement->getDataDescriptor()), groupedBands) == false)
   {
      progress.report("Unable to populate output descriptor values. "
         "The output descriptor (including wavelengths) may be invalid.", 0, WARNING);
   }

   // TODO: optimize for BIL and BSQ
   if (pDescriptor->getInterleaveFormat() != BIP)
   {
      progress.report("Band binning is optimized for BIP data.", 0, WARNING);
   }

   // Querying for bad values during execution of the algorithm is computationally expensive.
   // If the bad values are uniform, then they do not need to be queried by the algorithm while it is running.
   // Scan the requested bands prior to running the algorithm to determine if the bad values are uniform.
   // If the bad values are uniform, run an optimized version of the algorithm which uses a const std::vector<int>
   // across all bands instead of querying each individual band for its bad values.
   std::string badValuesStr;
   if (getUniformBadValues(groupedBands, pElement, badValuesStr) == true)
   {
      FactoryResource<BadValues> pBadValues;
      pBadValues->setBadValues(badValuesStr);
      switchOnEncoding(pDescriptor->getDataType(), createGroupedBandsBipUniformBadValues, NULL,
         progress, mAborted, pElement, pOutputElement.get(), groupedBands, pBadValues.get());
   }
   else
   {
      progress.report("Non-uniform bad values are defined across bands of the input data. "
         "Remove bad values or set them to the same for each band for optimal performance.", 0, WARNING);
      switchOnEncoding(pDescriptor->getDataType(), createGroupedBandsBipNonUniformBadValues, NULL,
         progress, mAborted, pElement, pOutputElement.get(), groupedBands);
   }

   if (isAborted() == true)
   {
      progress.report("Cancelled", 0, ABORT, true);   // Set the "log" flag to get auto-close.
      return false;
   }

   // Create a window and view if the application is in interactive mode.
   // This is done after the algorithm to ensure that the reset stretch functionality works as intended.
   if (Service<ApplicationServices>()->isInteractive() == true && createOutputView(pOutputElement.get()) == false)
   {
      progress.report("Unable to create output view.", 0, WARNING);
   }

   // Update the output argument list, release the output resource, and return.
   if (pOutArgList != NULL)
   {
      VERIFY(pOutArgList->setPlugInArgValue<RasterElement>(Executable::DataElementArg(), pOutputElement.get()));
   }

   pOutputElement.release();
   progress.report(getName() + " complete.", 100, NORMAL);
   progress.upALevel();
   return true;
}
