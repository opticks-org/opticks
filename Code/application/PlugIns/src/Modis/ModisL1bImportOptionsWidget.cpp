/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Hdf4Utilities.h"
#include "LabeledSection.h"
#include "ModisL1bImportOptionsWidget.h"
#include "RasterConversionTypeComboBox.h"
#include "StringUtilities.h"
#include "Units.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>

#include <mfhdf.h>

#include <string>
#include <vector>

ModisL1bImportOptionsWidget::ModisL1bImportOptionsWidget(RasterDataDescriptor* pDescriptor) :
   LabeledSectionGroup(NULL),
   mpDescriptor(pDescriptor)
{
   // Raster values section
   QWidget* pRasterWidget = new QWidget(this);

   QLabel* pRasterConversionLabel = new QLabel("Raster Value Conversion:", pRasterWidget);
   mpRasterConversionCombo = new RasterConversionTypeComboBox(this);

   QHBoxLayout* pRasterLayout = new QHBoxLayout(pRasterWidget);
   pRasterLayout->setMargin(0);
   pRasterLayout->setSpacing(5);
   pRasterLayout->addWidget(pRasterConversionLabel);
   pRasterLayout->addWidget(mpRasterConversionCombo);
   pRasterLayout->addStretch();

   LabeledSection* pRasterValuesSection = new LabeledSection(pRasterWidget, "Raster Pixel Values", this);

   // Layout
   QWidget* pWidget = widget();
   if (pWidget != NULL)
   {
      QLayout* pLayout = pWidget->layout();
      if (pLayout != NULL)
      {
         pLayout->setMargin(10);
      }
   }

   // Initialization
   addSection(pRasterValuesSection);
   addStretch(10);
   setSizeHint(350, 150);

   ModisUtilities::RasterConversionType rasterConversion;
   if (mpDescriptor.get() != NULL)
   {
      // If the raster conversion value exists in the metadata, the data is being imported from the MRU file list
      const DynamicObject* pMetadata = mpDescriptor->getMetadata();
      rasterConversion = ModisUtilities::getRasterConversion(pMetadata);
   }

   mpRasterConversionCombo->setRasterConversion(rasterConversion);

   // Connections
   VERIFYNR(connect(mpRasterConversionCombo, SIGNAL(rasterConversionChanged(ModisUtilities::RasterConversionType)),
      this, SLOT(updateDataDescriptor(ModisUtilities::RasterConversionType))));
}

ModisL1bImportOptionsWidget::~ModisL1bImportOptionsWidget()
{}

void ModisL1bImportOptionsWidget::setRasterConversion(ModisUtilities::RasterConversionType rasterConversion)
{
   mpRasterConversionCombo->setRasterConversion(rasterConversion);
}

ModisUtilities::RasterConversionType ModisL1bImportOptionsWidget::getRasterConversion() const
{
   return mpRasterConversionCombo->getRasterConversion();
}

EncodingType ModisL1bImportOptionsWidget::getOriginalDataType() const
{
   if (mpDescriptor.get() == NULL)
   {
      return EncodingType();
   }

   const FileDescriptor* pFileDescriptor = mpDescriptor->getFileDescriptor();
   if (pFileDescriptor == NULL)
   {
      return EncodingType();
   }

   const Filename& filenameObj = pFileDescriptor->getFilename();
   std::string filename = filenameObj.getFullPathAndName();

   HdfUtilities::Hdf4FileResource pHdfFile(filename.c_str());
   if ((pHdfFile.get() == NULL) || (*pHdfFile == FAIL))
   {
      return EncodingType();
   }

   const DynamicObject* pMetadata = mpDescriptor->getMetadata();
   VERIFYRV(pMetadata != NULL, EncodingType());

   EncodingType dataType;

   std::vector<ModisUtilities::HdfDatasetInfo> datasets = ModisUtilities::getBandDatasets(pMetadata);
   for (std::vector<ModisUtilities::HdfDatasetInfo>::iterator iter = datasets.begin(); iter != datasets.end(); ++iter)
   {
      HdfUtilities::Hdf4DatasetResource pHdfDataset(*pHdfFile, iter->first);
      if ((pHdfDataset.get() != NULL) && (*pHdfDataset != FAIL))
      {
         int32 hdfDataType = 0;
         if (SDgetinfo(*pHdfDataset, NULL, NULL, NULL, &hdfDataType, NULL) == SUCCEED)
         {
            EncodingType currentDataType = HdfUtilities::hdf4TypeToEncodingType(hdfDataType);
            if (currentDataType != dataType)
            {
               if (dataType.isValid() == true)
               {
                  return EncodingType();
               }

               dataType = currentDataType;
            }
         }
      }
   }

   return dataType;
}

void ModisL1bImportOptionsWidget::updateDataDescriptor(ModisUtilities::RasterConversionType rasterConversion)
{
   if (mpDescriptor.get() == NULL)
   {
      return;
   }

   // Update the imported data type and unit type based on the selected conversion
   Units* pUnits = mpDescriptor->getUnits();
   VERIFYNRV(pUnits != NULL);

   EncodingType dataType = FLT4BYTES;
   UnitType unitType;

   switch (rasterConversion)
   {
   case ModisUtilities::NO_CONVERSION:
      // If the user is applying a scale factor, the data type needs to remain FLT4BYTES
      if (pUnits->getScaleFromStandard() == 1.0)
      {
         dataType = getOriginalDataType();
      }

      unitType = DIGITAL_NO;
      break;

   case ModisUtilities::CONVERT_TO_RADIANCE:
      unitType = RADIANCE;
      break;

   case ModisUtilities::CONVERT_TO_REFLECTANCE:
      unitType = REFLECTANCE;
      break;

   default:
      break;
   }

   VERIFYNRV(dataType.isValid() == true);
   VERIFYNRV(unitType.isValid() == true);

   mpDescriptor->setDataType(dataType);
   mpDescriptor->setValidDataTypes(std::vector<EncodingType>(1, dataType));
   pUnits->setUnitType(unitType);

   // Add an attribute to the metadata so that the conversion can be performed when importing from the MRU file list
   DynamicObject* pMetadata = mpDescriptor->getMetadata();
   if (pMetadata != NULL)
   {
      std::string conversionText = StringUtilities::toXmlString(rasterConversion);
      pMetadata->setAttribute(RASTER_CONVERSION, conversionText);
   }
}
