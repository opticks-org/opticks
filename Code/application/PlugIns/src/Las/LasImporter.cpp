/*
 * The information in this file is
 * Copyright(c) 2014 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DataVariant.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "ImportDescriptor.h"
#include "LasImporter.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudFileDescriptor.h"
#include "PointCloudView.h"
#include "PointCloudWindow.h"
#include "ProgressTracker.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"
#include "UtilityServices.h"

#include <liblas/liblas.hpp>
#include <liblas/iterator.hpp>

#include <boost/atomic.hpp>
#include <math.h>
#include <QtCore/QString>
#include <QtGui/QComboBox>

REGISTER_PLUGIN_BASIC(Las, LasImporter);

LasImporter::LasImporter() : mCustomOptions(), mPolishEntered(false)
{
   setName("LAS Importer");
   setDescription("LAS LIDAR point cloud importer");
   setDescriptorId("{43DD7AED-0B29-4106-B524-B55ED86FAEB8}");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setExtensions("LAS Files (*.las *.laz)");
   setAbortSupported(true);

   addDependencyCopyright("liblas", Service<UtilityServices>()->getTextFromFile(":/licenses/liblas"));
}

LasImporter::~LasImporter()
{
}

std::vector<ImportDescriptor*> LasImporter::getImportDescriptors(const std::string& filename)
{
   std::vector<ImportDescriptor*> descriptors;

   try
   {
      std::ifstream ifs;
      if (!liblas::Open(ifs, filename.c_str()))
      {
         return descriptors;
      }
      liblas::ReaderFactory f;
      liblas::Reader reader = f.CreateWithStream(ifs);      
      const liblas::Header& header = reader.GetHeader();

      mOffsetX = header.GetOffsetX();
      mOffsetY = header.GetOffsetY();
      mOffsetZ = header.GetOffsetZ();
      double xScale = header.GetScaleX();
      double yScale = header.GetScaleY();
      double zScale = header.GetScaleZ();
      mMinX = (header.GetMinX() - mOffsetX) / xScale;
      mMinY = (header.GetMinY() - mOffsetY) / yScale;
      mMinZ = (header.GetMinZ() - mOffsetZ) / zScale;
      mMaxX = (header.GetMaxX() - mOffsetX) / xScale;
      mMaxY = (header.GetMaxY() - mOffsetY) / yScale;
      mMaxZ = (header.GetMaxZ() - mOffsetZ) / zScale;

      ImportDescriptorResource descZ(filename, TypeConverter::toString<PointCloudElement>());
      VERIFYRV(descZ.get(), descriptors);

      liblas::Header::RecordsByReturnArray pointsByReturn = header.GetPointRecordsByReturnCount();      

      PointCloudDataDescriptor* pDesc = 
         generatePointCloudDataDescriptor(filename, NULL, BSQ, INT4SBYTES, INT2UBYTES, INT1UBYTE, IN_MEMORY);

      uint32_t numPoints = header.GetPointRecordsCount();

      mOffsetX = header.GetOffsetX();
      mOffsetY = header.GetOffsetY();
      mOffsetZ = header.GetOffsetZ();

      pDesc->setPointCount(numPoints);
      pDesc->setFileDescriptor(pDesc->getFileDescriptor());
      pDesc->setArrangement(POINT_ARRAY);
      pDesc->setPointCount(numPoints);
      pDesc->setHasClassificationData(true);
      pDesc->setHasIntensityData(true);
      pDesc->setXMax(mMaxX);
      pDesc->setXMin(mMinX);
      pDesc->setYMax(mMaxY);
      pDesc->setYMin(mMinY);
      pDesc->setZMax(mMaxZ);
      pDesc->setZMin(mMinZ);
      pDesc->setXOffset(mOffsetX);
      pDesc->setYOffset(mOffsetY);
      pDesc->setZOffset(mOffsetZ);
      pDesc->setXScale(xScale);
      pDesc->setYScale(yScale);
      pDesc->setZScale(zScale);
      pDesc->setProcessingLocation(IN_MEMORY);

      VERIFYRV(pDesc, descriptors);
      descZ->setDataDescriptor(pDesc);

      VERIFYRV(RasterUtilities::generateAndSetFileDescriptor(pDesc, filename, "", LITTLE_ENDIAN_ORDER), descriptors);

      DynamicObject* pMetadataZ = pDesc->getMetadata();
      VERIFYRV(pMetadataZ, descriptors);
      pMetadataZ->setAttributeByPath("LAS/Version", QString("%1.%2").arg(header.GetVersionMajor()).arg(header.GetVersionMinor()).toStdString());
      pMetadataZ->setAttributeByPath("LAS/Creation DOY", header.GetCreationDOY());
      pMetadataZ->setAttributeByPath("LAS/Creation Year", header.GetCreationYear());
      pMetadataZ->setAttributeByPath("LAS/Point Format", header.GetDataFormatId() == liblas::ePointFormat0 ? 0 : 1);
      pMetadataZ->setAttributeByPath("LAS/Point Count", header.GetPointRecordsCount());
      pMetadataZ->setAttributeByPath("LAS/Points Per Return", pointsByReturn);
      pMetadataZ->setAttributeByPath("LAS/Scale/X", header.GetScaleX());
      pMetadataZ->setAttributeByPath("LAS/Scale/Y", header.GetScaleY());
      pMetadataZ->setAttributeByPath("LAS/Scale/Z", header.GetScaleZ());
      pMetadataZ->setAttributeByPath("LAS/Offset/X", header.GetOffsetX());
      pMetadataZ->setAttributeByPath("LAS/Offset/Y", header.GetOffsetY());
      pMetadataZ->setAttributeByPath("LAS/Offset/Z", header.GetOffsetZ());

      if ( !pMetadataZ->getAttributeByPath("LAS/Thinning Options/Algorithm").isValid() )
      {
          pMetadataZ->setAttributeByPath("LAS/Thinning Options/Algorithm", static_cast<int>(THIN_NONE));
          pMetadataZ->setAttributeByPath("LAS/Thinning Options/Max Points", 100000);
          pMetadataZ->setAttributeByPath("LAS/Thinning Options/Grid Size", 1.0);
      }

      PointCloudFileDescriptor* pFileDesc = static_cast<PointCloudFileDescriptor*>(pDesc->getFileDescriptor());

      descriptors.push_back(descZ.release());
   }
   catch(std::exception&)
   {
   }
   return descriptors;
}

unsigned char LasImporter::getFileAffinity(const std::string& filename)
{
   try
   {
      std::ifstream ifs;
      if (!liblas::Open(ifs, filename.c_str()))
      {
         return CAN_NOT_LOAD;
      }
      liblas::ReaderFactory f;
      liblas::Reader reader = f.CreateWithStream(ifs);      
      const liblas::Header& header = reader.GetHeader();
      return CAN_LOAD;
   }
   catch(std::exception&)
   {
      return CAN_NOT_LOAD;
   }
}

bool LasImporter::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(ProgressArg(), NULL, Executable::ProgressArgDescription());
   pInArgList->addArg<PointCloudElement>(ImportElementArg(), NULL, "Data will be loaded into this element");
   return true;
}

bool LasImporter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   if (!isBatch())
   {
      pOutArgList->addArg<PointCloudView>("View", NULL, "The new point cloud view that was created.");
   }
   return true;
}

void LasImporter::polishDataDescriptor(DataDescriptor *pDescriptor)
{
   // if no metadata has been pushed into the options widget, then the user hasn't called the widget
   // so don't update the metadata with bad values
   if ( !( mCustomOptions.getInputGridSize()->text() == "" && mCustomOptions.getInputMaxPoints()->text() == "" ) )
   {
      DynamicObject* pMetadata = pDescriptor->getMetadata();
      // ensure we are reentrant by locking the sets or we'll get in an infinite loop
      bool echk = false; // check value...must be stack local for reentrance check to work
      if (mPolishEntered.compare_exchange_strong(echk, true, boost::memory_order_seq_cst))
      {
         pMetadata->setAttributeByPath( "LAS/Thinning Options/Algorithm", 
            mCustomOptions.getDropDown()->currentIndex() );
         pMetadata->setAttributeByPath( "LAS/Thinning Options/Max Points", 
            mCustomOptions.getInputMaxPoints()->text().toInt() );
         pMetadata->setAttributeByPath( "LAS/Thinning Options/Grid Size", 
            mCustomOptions.getInputGridSize()->text().toDouble() );
         mPolishEntered.store(false);
      }
   }
}

bool LasImporter::execute( PlugInArgList* pInArgList, PlugInArgList* pOutArgList )
{
   if ( pInArgList == NULL || pOutArgList == NULL )
   {
      return false;
   }
   ProgressTracker progress( pInArgList->getPlugInArgValue<Progress>(ProgressArg() ), "Loading LAS data",
      "LIDAR", "{b781a8a7-94fb-4172-b3ca-8666f45d0bb3}" );
   progress.report( "Loading LAS data...", 1, NORMAL );
   PointCloudElement* pData = pInArgList->getPlugInArgValue<PointCloudElement>(ImportElementArg());
   if ( pData == NULL )
   {
      progress.report( "No data element specified.", 0, ERRORS, true );
      return false;
   }
   PointCloudDataDescriptor* pDesc = dynamic_cast<PointCloudDataDescriptor*>( pData->getDataDescriptor() );
   VERIFY( pDesc );
   if ( pDesc->getProcessingLocation() == ON_DISK_READ_ONLY )
   {
      progress.report( "On-disk read only not supported.", 0, ERRORS, true );
      return false;
   }
   if (!pData->createDefaultPager())
   {
      progress.report( "Unable to allocate space for point cloud", 0, ERRORS, true );
      return false;
   }
   std::ifstream ifs;
   if (!liblas::Open(ifs, pDesc->getFileDescriptor()->getFilename().getFullPathAndName().c_str()))
   {
      return false;
   }
   liblas::ReaderFactory f;
   liblas::Reader reader = f.CreateWithStream(ifs);      
   const liblas::Header& header = reader.GetHeader();

   int thinningOption(0);
   pDesc->getMetadata()->getAttributeByPath( "LAS/Thinning Options/Algorithm" ).getValue( thinningOption );
   switch ( thinningOption ) {
       case THIN_NONE:
           if (maxPointsThinning(header.GetPointRecordsCount(), pDesc, 
                     reader, header, pData, progress, &mAborted) < 0)
           {
              return false;
           }
           break;
       case THIN_MAX_POINTS:
           int maxPoints;
           pDesc->getMetadata()->getAttributeByPath("LAS/Thinning Options/Max Points").getValue(maxPoints);
           int totPoints;
           if ((totPoints = maxPointsThinning( maxPoints, pDesc, reader, header, pData, progress, &mAborted)) < 0)
           {
              return false;
           }
           pDesc->setPointCount(totPoints);
           break;
       default:
           break;
   }      
   

   // Create the view
   if (!isBatch())
   {
      PointCloudView* pView = NULL;
      PointCloudWindow* pWindow = static_cast<PointCloudWindow*>(
            Service<DesktopServices>()->createWindow( pData->getName(), POINT_CLOUD_WINDOW ) );
      progress.report( "Creating view...", 99, NORMAL );
      pView = ( pWindow == NULL ) ? NULL : pWindow->getPointCloudView();
      if ( pView == NULL )
      {
         progress.report("Error creating view...", 0, ERRORS, true);
         return false;
      }
      // Set the spatial data in the view
      pView->setPrimaryPointCloud(pData);

      if (!pOutArgList->setPlugInArgValue("View", pView))
      {
         progress.report("Unable to set View return argument.", 99, WARNING, true);
      }
   }

   progress.report( "Finished loading LAS data..", 100, NORMAL );
   progress.upALevel();
   return true;
}

bool LasImporter::isProcessingLocationSupported(ProcessingLocation location) const 
{
    return !(location == ON_DISK_READ_ONLY);
}

QWidget* LasImporter::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
    int thinningOption;
    double gridSize;
    int maxPoints;

    pDescriptor->getMetadata()->getAttributeByPath("LAS/Thinning Options/Algorithm").getValue(thinningOption);
    pDescriptor->getMetadata()->getAttributeByPath("LAS/Thinning Options/Max Points").getValue(maxPoints);
    pDescriptor->getMetadata()->getAttributeByPath("LAS/Thinning Options/Grid Size").getValue(gridSize);

    mCustomOptions.getDropDown()->setCurrentIndex(thinningOption);
    mCustomOptions.getInputGridSize()->setText(QString::number(gridSize));
    mCustomOptions.getInputMaxPoints()->setText(QString::number(maxPoints));
    return mCustomOptions.getWidget();
}

bool LasImporter::validate(const DataDescriptor* pDescriptor,
      const std::vector<const DataDescriptor*>& importedDescriptors, std::string& errorMessage) const
{
   const PointCloudDataDescriptor* pDesc = dynamic_cast<const PointCloudDataDescriptor*>(pDescriptor);
   if (pDesc == NULL || !ImporterShell::validate(pDescriptor, importedDescriptors, errorMessage))
   {
      return false;
   }
   if (pDesc->getProcessingLocation() == IN_MEMORY)
   {
      uint64_t size = pDesc->getPointSizeInBytes() * pDesc->getPointCount();
      uint64_t maxMemoryAvail = Service<UtilityServices>()->getMaxMemoryBlockSize();
      if (maxMemoryAvail < size)
      {
         errorMessage = "The data set cannot be loaded into memory. Use a different "
                        "processing location or specify a subset.";
         return false;
      }
   }
   if (pDesc->getXScale() == 0. || pDesc->getYScale() == 0. || pDesc->getZScale() == 0.)
   {
      errorMessage = "Invalid scale factor (0.0).";
      return false;
   }

   return true;
}

QWidget* LasImporter::getPreview(const DataDescriptor* pDescriptor, Progress* pProgress)
{
   return NULL;
}

int LasImporter::maxPointsThinning(const unsigned int maxPoints,
                                   const PointCloudDataDescriptor* pDesc,
                                   liblas::Reader& reader,
                                   liblas::Header const& header,
                                   PointCloudElement* pElement,
                                   ProgressTracker& progress,
                                   bool* pAborted)
{
   bool intensity = pDesc->getFileDescriptor()->getDatasetLocation() == "intensity";
   VERIFY(pDesc->getSpatialDataType() == INT4SBYTES && pDesc->getIntensityDataType() == INT2UBYTES && pDesc->getClassificationDataType() == INT1UBYTE);
   unsigned int nthPoint;
   if (maxPoints >= header.GetPointRecordsCount())
   {
      nthPoint = 1;
   }
   else
   {
      nthPoint = static_cast<unsigned int>(std::ceil(static_cast<double>(header.GetPointRecordsCount()) / maxPoints));
   }

   unsigned int cur = 0;
   unsigned int total = header.GetPointRecordsCount();
   int oldPercent = 0;
   int curPercent = 0;
   double minX = header.GetMinX();
   double minY = header.GetMinY();
   int totPoints = 0;
   FactoryResource<PointCloudDataRequest> pReq;
   pReq->setWritable(true);
   PointCloudAccessor accessor(pElement->getPointCloudAccessor(pReq.release()));
   for (liblas::reader_iterator<liblas::Point> it(reader); cur < total; ++it)
   {
      curPercent = ++cur * 100 / total;
      if (curPercent - oldPercent >= 1 && curPercent < 99)
      {
         if (pAborted != NULL && *pAborted)
         {
            progress.report("Import canceled", 0, ABORT);
            return -1;
         }
         progress.report("Loading LAS data...", curPercent, NORMAL);
      }
      else if (curPercent - oldPercent >=1)
      {
         progress.report("Loading LAS data...", 99, NORMAL);
      }
      oldPercent = curPercent;
      if (cur % nthPoint == 0)
      {
         *reinterpret_cast<int32_t*>(accessor->getRawX()) = (*it).GetRawX();
         *reinterpret_cast<int32_t*>(accessor->getRawY()) = (*it).GetRawY();
         *reinterpret_cast<int32_t*>(accessor->getRawZ()) = (*it).GetRawZ();
         *reinterpret_cast<uint16_t*>(accessor->getRawIntensity()) = (*it).GetIntensity();
         *reinterpret_cast<unsigned char*>(accessor->getRawClassification()) = (*it).GetClassification().GetClass();
         accessor->setPointValid(true);
         accessor->nextPoint();
         totPoints++;
      }
   }
   return totPoints;
}

PointCloudDataDescriptor* LasImporter::generatePointCloudDataDescriptor(const std::string& name, DataElement* pParent,
                                                                        InterleaveFormatType interleave,
                                                                        EncodingType encoding,
                                                                        EncodingType intensityEncoding,
                                                                        EncodingType classEncoding,
                                                                        ProcessingLocation location)
{
   // right now, these are the only supported types for LAS since this is what liblas returns
   // offset/scale adjusted values are doubles but this is handled by the PointCloudAccessor
   if (encoding != INT4SBYTES || intensityEncoding != INT2UBYTES || classEncoding != INT1UBYTE)
   {
      return NULL;
   }
   Service<ModelServices> pModel;
   PointCloudDataDescriptor* pDd = dynamic_cast<PointCloudDataDescriptor*>(
      pModel->createDataDescriptor(name, "PointCloudElement", pParent));
   if (pDd == NULL)
   {
      return NULL;
   }

   pDd->setSpatialDataType(encoding);
   pDd->setIntensityDataType(intensityEncoding);
   pDd->setClassificationDataType(classEncoding);
   pDd->setProcessingLocation(location);

   return pDd;
}
