/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDir>
#include <QtGui/QImage>

#include "assert.h"
#include "AppConfig.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "Exporter.h"
#include "FileDescriptor.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

using namespace std;

class PicturesTestCase : public TestCase
{
public:
   PicturesTestCase(std::string name) : TestCase(name)
   {
   }
   bool run()
   {
      bool success = true;

      // Get the loaded data set view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea(pView != NULL);

      // Export with the exporter
      string exporterName = getExporterName();
      string tempPath;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempPath = pTempPath->getFullPathAndName();
      }
      string exporterFilename = tempPath + SLASH + getExporterFilename();
      FactoryResource<FileDescriptor> pExporterFileDescriptor;
      pExporterFileDescriptor->setFilename(exporterFilename);

      ExporterResource exporter(exporterName, pView, pExporterFileDescriptor.get(), NULL);
      issea(exporter->execute());

      // Export with Qt
      string qtFilename;
      qtFilename = tempPath + SLASH + getQtFilename();
      string imageFormat = getImageFormat();

      SpatialDataViewImp* pViewImp = dynamic_cast<SpatialDataViewImp*>(pView);
      issea(pViewImp != NULL);

      QImage viewImage = pViewImp->getCurrentImage();
      issea(viewImage.isNull() == false);
      issea(viewImage.save(QString::fromStdString(qtFilename), imageFormat.c_str()));

      // Import the two images and compare
      QImage exporterImage(QString::fromStdString(exporterFilename), imageFormat.c_str());
      QImage qtImage(QString::fromStdString(qtFilename), imageFormat.c_str());
      // ensure the images are in the same format as some exporters may strip the alpha channel
      qtImage = qtImage.convertToFormat(exporterImage.format());
      issea(exporterImage.isNull() == false);
      issea(qtImage.isNull() == false);
      issea(exporterImage == qtImage);

      // Cleanup
      remove(exporterFilename.c_str());
      remove(qtFilename.c_str());

      return success;
   }

protected:
   virtual std::string getImageFormat() const = 0;
   virtual std::string getExporterName() const = 0;
   virtual std::string getExporterFilename() const = 0;
   virtual std::string getQtFilename() const = 0;
};

class BitmapExporterTestCase : public PicturesTestCase
{
public:
   BitmapExporterTestCase::BitmapExporterTestCase() : PicturesTestCase("Bitmap") {}

protected:
   std::string getImageFormat() const { return "BMP"; }
   std::string getExporterName() const { return "Bitmap View Exporter"; }
   std::string getExporterFilename() const { return "TestExporterBitmap.bmp"; }
   std::string getQtFilename() const { return "TestQtBitmap.bmp"; }
};

class JpegExporterTestCase : public PicturesTestCase
{
public:
   JpegExporterTestCase::JpegExporterTestCase() : PicturesTestCase("Jpeg") {}

protected:
   std::string getImageFormat() const { return "JPEG"; }
   std::string getExporterName() const { return "JPEG View Exporter"; }
   std::string getExporterFilename() const { return "TestExporterJpeg.jpg"; }
   std::string getQtFilename() const { return "TestQtJpeg.jpg"; }
};

class PngExporterTestCase : public PicturesTestCase
{
public:
   PngExporterTestCase::PngExporterTestCase() : PicturesTestCase("Png") {}

protected:
   std::string getImageFormat() const { return "PNG"; }
   std::string getExporterName() const { return "PNG View Exporter"; }
   std::string getExporterFilename() const { return "TestExporterPng.png"; }
   std::string getQtFilename() const { return "TestQtPng.png"; }
};

class Jpeg2000ExportTestCase : public TestCase
{
public:
   Jpeg2000ExportTestCase::Jpeg2000ExportTestCase() : TestCase("Jpeg2000Export") {}

   virtual bool run()
   {
      bool success = true;

      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      issearf(pTempPath != NULL);
      const std::string tempPath = pTempPath->getFullPathAndName() + SLASH + "Jpeg2000ExportTestCase" + SLASH;
      issearf(QDir().mkpath(QString::fromStdString(tempPath)));

      // Test subcubing on export by exporting the standard test raster element using the same options with this
      // exporter and the ENVI exporter. Then re-open the exported JPEG2000 file, and export it using the ENVI
      // exporter. The data files should be identical. The naming conventions used are:
      //    - Baseline: The file created by the ENVI exporter using a subset of pRaster.
      //    - JPEG2000: The file created by the JPEG2000 exporter using a subset of pRaster.
      //    - Test: The file created by the ENVI exporter after importing the JPEG2000 file.
      RasterElement* pElement = TestUtilities::getStandardRasterElement();
      issearf(pElement != NULL);
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDescriptor != NULL);

      // Define the parameters to use.
      // These are arbitrary choices which were mostly just picked to create non-trivial subsets.
      const std::string baselineFileName = tempPath + "baseline";
      const std::string jpeg2000FileExt = ".jp2";
      const std::string jpeg2000FileName = tempPath + "JPEG 2000";
      const std::string testFileName = tempPath + "test";

      const std::vector<DimensionDescriptor>& rows = pDescriptor->getRows();
      const DimensionDescriptor& startRow = rows[rows.size() / 4];
      const DimensionDescriptor& stopRow = rows[rows.size() / 2];
      const unsigned int rowSkipFactor = 2;

      const std::vector<DimensionDescriptor>& cols = pDescriptor->getColumns();
      const DimensionDescriptor& startCol = cols[cols.size() / 8];
      const DimensionDescriptor& stopCol = cols[cols.size() / 2];
      const unsigned int colSkipFactor = 3;

      const std::vector<DimensionDescriptor>& bands = pDescriptor->getBands();
      std::vector<DimensionDescriptor> subsetBands;
      for (size_t i = 0; i < bands.size(); ++i)
      {
         // Test arbitrary band combinations without a constant skip factor.
         if (i % 3 == 0 || i % 5 == 0)
         {
            subsetBands.push_back(bands[i]);
         }
      }

      // Create the Baseline and JPEG2000 files.
      std::vector<char> baselineData;
      {
         FactoryResource<RasterFileDescriptor> pBaselineFileDescriptor(
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, baselineFileName,
            startRow, stopRow, rowSkipFactor, startCol, stopCol, colSkipFactor, subsetBands));
         issearf(pBaselineFileDescriptor.get() != NULL);

         ExporterResource pEnviExporter("ENVI Exporter",
            pElement, pBaselineFileDescriptor.get(), NULL, false);
         issearf(pEnviExporter.get() != NULL);
         issearf(pEnviExporter->execute());

         // Read the data for comparison later.
         FileResource pBaselineFile(baselineFileName.c_str(), "rb");
         issearf(pBaselineFile.get() != NULL);
         fseek(pBaselineFile, 0, SEEK_END);
         baselineData.resize(ftell(pBaselineFile));
         fseek(pBaselineFile, 0, SEEK_SET);
         issearf(fread(&baselineData[0], 1, baselineData.size(), pBaselineFile) == baselineData.size());
      }

      {
         FactoryResource<RasterFileDescriptor> pJpeg2000FileDescriptor(
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, jpeg2000FileName + jpeg2000FileExt,
            startRow, stopRow, rowSkipFactor, startCol, stopCol, colSkipFactor, subsetBands));
         issearf(pJpeg2000FileDescriptor.get() != NULL);

         ExporterResource pJpeg2000Exporter("JPEG2000 Exporter",
            pElement, pJpeg2000FileDescriptor.get(), NULL, false);
         issearf(pJpeg2000Exporter.get() != NULL);
         issearf(pJpeg2000Exporter->execute());
      }

      // Import the JPEG2000 file, export it with the ENVI exporter, and diff the files.
      ImporterResource pJpeg2000Importer("JPEG2000 Importer", jpeg2000FileName + "." +
         StringUtilities::toXmlString(pDescriptor->getDataType()) + jpeg2000FileExt, NULL, false);
      issearf(pJpeg2000Importer.get() != NULL);
      issearf(pJpeg2000Importer->execute());
      std::vector<DataElement*> importedElements = pJpeg2000Importer->getImportedElements();
      issearf(importedElements.size() == 1);
      ModelResource<RasterElement> pJpeg2000Element(dynamic_cast<RasterElement*>(importedElements.front()));
      issearf(pJpeg2000Element.get() != NULL);

      FactoryResource<FileDescriptor> pTestFileDescriptor(
         RasterUtilities::generateFileDescriptorForExport(pJpeg2000Element->getDataDescriptor(), testFileName));
      issearf(pTestFileDescriptor.get() != NULL);

      ExporterResource pEnviExporter("ENVI Exporter",
         pJpeg2000Element.get(), pTestFileDescriptor.get(), NULL, false);
      issearf(pEnviExporter.get() != NULL);
      issearf(pEnviExporter->execute());

      FileResource pTestFile(testFileName.c_str(), "rb");
      issearf(pTestFile.get() != NULL);
      fseek(pTestFile, 0, SEEK_END);
      std::vector<char> testData(ftell(pTestFile));
      issearf(testData.size() == baselineData.size());
      fseek(pTestFile, 0, SEEK_SET);
      issearf(fread(&testData[0], 1, testData.size(), pTestFile) == testData.size());
      issearf(memcmp(&baselineData[0], &testData[0], testData.size()) == 0);

      return success;
   }
};

class Jpeg2000ImportTestCase : public TestCase
{
public:
   Jpeg2000ImportTestCase::Jpeg2000ImportTestCase() : TestCase("Jpeg2000Import") {}

   virtual bool run()
   {
      // Tests that loading files > 16 bits written out with the OPTICKS-1435 implementation works as intended.
      bool success = true;
      const std::string baselineFileName = TestUtilities::getTestDataPath() + SLASH + "allValues-32bits";

      FileResource pBaselineFile(baselineFileName.c_str(), "rb");
      issearf(pBaselineFile.get() != NULL);
      fseek(pBaselineFile, 0, SEEK_END);
      std::vector<char> baselineData(ftell(pBaselineFile));
      fseek(pBaselineFile, 0, SEEK_SET);
      issearf(fread(&baselineData[0], 1, baselineData.size(), pBaselineFile) == baselineData.size());

      ImporterResource pJp2Importer("JPEG2000 Importer", baselineFileName + ".INT4UBYTES.jp2", NULL, false);
      issearf(pJp2Importer.get() != NULL);
      issearf(pJp2Importer->execute());
      std::vector<DataElement*> importedElements = pJp2Importer->getImportedElements();
      issearf(importedElements.size() == 1);
      RasterElement* pJp2Element = dynamic_cast<RasterElement*>(importedElements.front());
      issearf(pJp2Element != NULL);
      issearf(memcmp(&baselineData[0], pJp2Element->getRawData(), baselineData.size()) == 0);

      ImporterResource pJ2kImporter("JPEG2000 Importer", baselineFileName + ".INT4UBYTES.j2k", NULL, false);
      issearf(pJ2kImporter.get() != NULL);
      issearf(pJ2kImporter->execute());
      importedElements = pJ2kImporter->getImportedElements();
      issearf(importedElements.size() == 1);
      RasterElement* pJ2kElement = dynamic_cast<RasterElement*>(importedElements.front());
      issearf(pJ2kElement != NULL);
      issearf(memcmp(&baselineData[0], pJ2kElement->getRawData(), baselineData.size()) == 0);

      return success;
   }
};

class Jpeg2000TestCase : public TestCase
{
public:
   Jpeg2000TestCase::Jpeg2000TestCase() : TestCase("Jpeg2000") {}

   virtual bool run()
   {
      bool success = true;

      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      issearf(pTempPath != NULL);
      const std::string tempPath = pTempPath->getFullPathAndName() + SLASH + "Jpeg2000TestCase" + SLASH;
      issearf(QDir().mkpath(QString::fromStdString(tempPath)));

      // This is the list of files to test.
      // The entries here are relative to TestUtilities::getTestDataPath().
      std::vector<std::string> files;
      files.push_back("0.tif");
      files.push_back("42.tif");
      files.push_back("0xDA.tif");
      files.push_back("0xDABC.tif");
      files.push_back("0x1FFFFFF.tif");
      files.push_back("0x3FFFFFF.tif");
      files.push_back("walking1-16bits");
      files.push_back("walking1-32bits");
      files.push_back("allValues-32bits");
      files.push_back("3x4x5-incrementing");
      files.push_back("landsat6band.tif");

      // These are the data types to test.
      // Interleave conversion on import will be performed and export tested for each of these data types.
      std::vector<EncodingType> types;
      types.push_back(INT1UBYTE);
      types.push_back(INT1SBYTE);
      types.push_back(INT2UBYTES);
      types.push_back(INT2SBYTES);
      types.push_back(INT4UBYTES);
      types.push_back(INT4SBYTES);
      types.push_back(FLT4BYTES);

      // These are the Jpeg2000 extensions to test.
      std::vector<std::string> exts;
      exts.push_back(".jp2");
      exts.push_back(".j2k");

      // For each file and interleave defined:
      //   1. Load the first dataset from the file using the GDAL Importer (specifying BIP as the interleave).
      //      The GDAL Importer is used for two reasons:
      //        a. It can import all of the file formats we need here.
      //        b. It can perform data type conversion on import, which we need here.
      //   2. Export the dataset using the ENVI Exporter.
      //   For each Jpeg2000 extension:
      //      3. Export the dataset using the Jpeg2000 Exporter.
      //      4. Import the dataset from (3) using the Jpeg2000 Importer.
      //      5. Export the dataset from (4) using the ENVI Exporter.
      //      6. Compare the data files (and not the header files!) exported in (2) and (5); they should be identical.
      for (std::vector<std::string>::const_iterator fileIter = files.begin(); fileIter != files.end(); ++fileIter)
      {
         printf("    File: %s\n", fileIter->c_str());
         for (std::vector<EncodingType>::const_iterator typeIter = types.begin(); typeIter != types.end(); ++typeIter)
         {
            // 1. Load the first dataset from the file using the GDAL Importer (specifying BIP as the interleave).
            printf("        Interleave: %s\n", StringUtilities::toDisplayString(*typeIter).c_str());
            ImporterResource pGdalImporter("Generic GDAL Importer",
               TestUtilities::getTestDataPath() + *fileIter, NULL, false);
            issearf(pGdalImporter.get() != NULL);
            std::vector<ImportDescriptor*> importDescriptors = pGdalImporter->getImportDescriptors();
            issearf(importDescriptors.size() >= 1);
            for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin() + 1;
               iter != importDescriptors.end();
               ++iter)
            {
               (*iter)->setImported(false);
            }

            // Use a ModelResource so that the element is deleted after each iteration.
            ModelResource<RasterElement> pGdalElement(reinterpret_cast<RasterElement*>(NULL));
            RasterDataDescriptor* pDescriptor = NULL;
            {
               ImportDescriptor* pImportDescriptor = importDescriptors.front();
               issearf(pImportDescriptor != NULL);
               RasterDataDescriptor* pImportDataDescriptor =
                  dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
               issearf(pImportDataDescriptor != NULL);
               pImportDataDescriptor->setDataType(*typeIter);
               pImportDataDescriptor->setInterleaveFormat(BIP);
               issearf(pGdalImporter->execute());
               std::vector<DataElement*> importedElements = pGdalImporter->getImportedElements();
               issearf(importedElements.size() == 1);
               pGdalElement = ModelResource<RasterElement>(dynamic_cast<RasterElement*>(importedElements.front()));
               issearf(pGdalElement.get() != NULL);
               pDescriptor = dynamic_cast<RasterDataDescriptor*>(pGdalElement->getDataDescriptor());
               issearf(pDescriptor != NULL);
            }

            //   2. Export the dataset using the ENVI Exporter.
            std::vector<char> baselineData;
            {
               const std::string baselineFileName = tempPath + *fileIter + "-baseline";
               FactoryResource<RasterFileDescriptor> pBaselineFileDescriptor(dynamic_cast<RasterFileDescriptor*>(
                  RasterUtilities::generateFileDescriptorForExport(pDescriptor, baselineFileName)));
               issearf(pBaselineFileDescriptor.get() != NULL);
               ExporterResource pEnviExporter("ENVI Exporter",
                  pGdalElement.get(), pBaselineFileDescriptor.get(), NULL, false);
               issearf(pEnviExporter.get() != NULL);
               issearf(pEnviExporter->execute());

               // Read the data for comparison later.
               FileResource pBaselineFile(baselineFileName.c_str(), "rb");
               issearf(pBaselineFile.get() != NULL);
               fseek(pBaselineFile, 0, SEEK_END);
               baselineData.resize(ftell(pBaselineFile));
               fseek(pBaselineFile, 0, SEEK_SET);
               issearf(fread(&baselineData[0], 1, baselineData.size(), pBaselineFile) == baselineData.size());
            }

            //   For each Jpeg2000 extension:
            for (std::vector<std::string>::const_iterator extIter = exts.begin(); extIter != exts.end(); ++extIter)
            {
               //   3. Export the dataset using the Jpeg2000 Exporter.
               // Append type information so the importer can set the data type correctly.
               std::string ext = "." + StringUtilities::toXmlString(*typeIter) + *extIter;

               printf("            Extension: %s\n", ext.c_str());
               const std::string jpeg2000FileName = tempPath + *fileIter + ext;
               FactoryResource<RasterFileDescriptor> pJpeg2000FileDescriptor(dynamic_cast<RasterFileDescriptor*>(
                  RasterUtilities::generateFileDescriptorForExport(pDescriptor, jpeg2000FileName)));
               ExporterResource pJpeg2000Exporter("JPEG2000 Exporter",
                  pGdalElement.get(), pJpeg2000FileDescriptor.get(), NULL, false);
               issearf(pJpeg2000Exporter.get() != NULL);
               issearf(pJpeg2000Exporter->execute());

               //   4. Import the dataset from (3) using the Jpeg2000 Importer.
               ImporterResource pJpeg2000Importer("JPEG2000 Importer", jpeg2000FileName, NULL, false);
               issearf(pJpeg2000Importer.get() != NULL);
               issearf(pJpeg2000Importer->execute());
               std::vector<DataElement*> importedElements = pJpeg2000Importer->getImportedElements();
               issearf(importedElements.size() == 1);

               // Use a ModelResource here for the same reason as mentioned previously.
               ModelResource<RasterElement> pJpeg2000Element(dynamic_cast<RasterElement*>(importedElements.front()));
               issearf(pJpeg2000Element.get() != NULL);

               //   5. Export the dataset from (4) using the ENVI Exporter.
               const std::string testFileName = tempPath + *fileIter + "-test";
               FactoryResource<RasterFileDescriptor> pTestFileDescriptor(dynamic_cast<RasterFileDescriptor*>(
                  RasterUtilities::generateFileDescriptorForExport(pDescriptor, testFileName)));
               issearf(pTestFileDescriptor.get() != NULL);
               ExporterResource pEnviExporter("ENVI Exporter",
                  pJpeg2000Element.get(), pTestFileDescriptor.get(), NULL, false);
               issearf(pEnviExporter.get() != NULL);
               issearf(pEnviExporter->execute());

               //   6. Compare the data files (and not the header files!) exported in (2) and (5); they should be identical.
               FileResource pTestFile(testFileName.c_str(), "rb");
               issearf(pTestFile.get() != NULL);
               fseek(pTestFile, 0, SEEK_END);
               std::vector<char> testData(ftell(pTestFile));
               issearf(testData.size() == baselineData.size());
               fseek(pTestFile, 0, SEEK_SET);
               issearf(fread(&testData[0], 1, testData.size(), pTestFile) == testData.size());
               issearf(memcmp(&baselineData[0], &testData[0], testData.size()) == 0);
            }
         }
      }

      return success;
   }
};

class PicturesTestSuite : public TestSuiteNewSession
{
public:
   PicturesTestSuite() : TestSuiteNewSession("Pictures")
   {
      addTestCase(new BitmapExporterTestCase);
      addTestCase(new JpegExporterTestCase);
      addTestCase(new PngExporterTestCase);
      addTestCase(new Jpeg2000ExportTestCase);
      addTestCase(new Jpeg2000ImportTestCase);
      addTestCase(new Jpeg2000TestCase);
   }
};

REGISTER_SUITE( PicturesTestSuite )
