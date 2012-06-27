/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "Georeference.h"
#include "ObjectResource.h"
#include "PlugIn.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterDataDescriptorImp.h"
#include "RasterFileDescriptorImp.h"
#include "RasterUtilities.h"
#include "StringUtilities.h"
#include "XmlUtilities.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

RasterDataDescriptorImp::RasterDataDescriptorImp(const string& name, const string& type, DataElement* pParent) :
   DataDescriptorImp(name, type, pParent),
   mDataType(INT1UBYTE),
   mValidDataTypes(StringUtilities::getAllEnumValues<EncodingType>()),
   mInterleave(BIP),
   mpBadValues(FactoryResource<BadValues>().release()),  // initialize to empty BadValues so user can set in import options
   mRowSkipFactor(0),
   mColumnSkipFactor(0),
   mXPixelSize(1.0),
   mYPixelSize(1.0),
   mDisplayMode(GRAYSCALE_MODE)
{
   // Attach to the member units and georeference descriptor objects to notify when they change
   VERIFYNR(mUnits.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterDataDescriptorImp::notifyModified)));
   VERIFYNR(mGeorefDescriptor.attach(SIGNAL_NAME(Subject, Modified),
      Slot(this, &RasterDataDescriptorImp::notifyModified)));
   mpBadValues.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterDataDescriptorImp::notifyBadValuesChanged));
}

RasterDataDescriptorImp::RasterDataDescriptorImp(const string& name, const string& type, const vector<string>& parent) :
   DataDescriptorImp(name, type, parent),
   mDataType(INT1UBYTE),
   mValidDataTypes(StringUtilities::getAllEnumValues<EncodingType>()),
   mInterleave(BIP),
   mpBadValues(FactoryResource<BadValues>().release()),  // initialize to empty BadValues so user can set in import options
   mRowSkipFactor(0),
   mColumnSkipFactor(0),
   mXPixelSize(1.0),
   mYPixelSize(1.0),
   mDisplayMode(GRAYSCALE_MODE)
{
   // Attach to the member units and georeference descriptor objects to notify when they change
   VERIFYNR(mUnits.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterDataDescriptorImp::notifyModified)));
   VERIFYNR(mGeorefDescriptor.attach(SIGNAL_NAME(Subject, Modified),
      Slot(this, &RasterDataDescriptorImp::notifyModified)));
   mpBadValues.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterDataDescriptorImp::notifyBadValuesChanged));
}

RasterDataDescriptorImp::~RasterDataDescriptorImp()
{}

void RasterDataDescriptorImp::notifyModified(Subject &subject, const string &signal, const boost::any &data)
{
   if ((&subject == &mUnits) || (&subject == &mGeorefDescriptor))
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void RasterDataDescriptorImp::notifyBadValuesChanged(Subject& subject, const std::string& signal,
   const boost::any& value)
{
   notify(SIGNAL_NAME(RasterDataDescriptor, BadValuesChanged), boost::any(mpBadValues.get()));
}

void RasterDataDescriptorImp::setDataType(EncodingType dataType)
{
   if (dataType != mDataType)
   {
      mDataType = dataType;
      notify(SIGNAL_NAME(RasterDataDescriptor, DataTypeChanged), boost::any(mDataType));
   }
}

EncodingType RasterDataDescriptorImp::getDataType() const
{
   return mDataType;
}

void RasterDataDescriptorImp::setValidDataTypes(const vector<EncodingType>& validDataTypes)
{
   if (validDataTypes != mValidDataTypes)
   {
      mValidDataTypes = validDataTypes;
      notify(SIGNAL_NAME(RasterDataDescriptor, ValidDataTypesChanged), boost::any(mValidDataTypes));
   }
}

const vector<EncodingType>& RasterDataDescriptorImp::getValidDataTypes() const
{
   return mValidDataTypes;
}

void RasterDataDescriptorImp::setInterleaveFormat(InterleaveFormatType format)
{
   if (format != mInterleave)
   {
      mInterleave = format;
      notify(SIGNAL_NAME(RasterDataDescriptor, InterleaveFormatChanged), boost::any(mInterleave));
   }
}

InterleaveFormatType RasterDataDescriptorImp::getInterleaveFormat() const
{
   return mInterleave;
}

unsigned int RasterDataDescriptorImp::getBytesPerElement() const
{
   return RasterUtilities::bytesInEncoding(mDataType);
}

void RasterDataDescriptorImp::setBadValues(const BadValues* pBadValues)
{
   if (mpBadValues.get() == NULL)
   {
      if (pBadValues != NULL)
      {
         mpBadValues.reset(FactoryResource<BadValues>().release());
         mpBadValues->setBadValues(pBadValues);
      }
      else
      {
         return;
      }
   }
   else if (pBadValues == NULL)  // indicates that the bands are not all using the same bad values criteria
   {
      Service<ObjectFactory>()->destroyObject(mpBadValues.get(), TypeConverter::toString<BadValues>());

      // the BadValues object in mpBadValues is now NULL
      notify(SIGNAL_NAME(RasterDataDescriptor, BadValuesChanged), boost::any(mpBadValues.get()));
   }
   else
   {
      if (mpBadValues->compare(pBadValues) == false)
      {
         mpBadValues->setBadValues(pBadValues);
      }
   }
}

void RasterDataDescriptorImp::setBadValues(const std::vector<int>& badValues)
{
   if (mpBadValues.get() == NULL)
   {
      mpBadValues.reset(FactoryResource<BadValues>().release());
   }
   FactoryResource<BadValues> pTempValues;
   pTempValues->addBadValues(badValues);
   mpBadValues->setBadValues(pTempValues.get());
}

const BadValues* RasterDataDescriptorImp::getBadValues() const
{
   return mpBadValues.get();
}

BadValues* RasterDataDescriptorImp::getBadValues()
{
   return mpBadValues.get();
}

void RasterDataDescriptorImp::setRows(const vector<DimensionDescriptor>& rows)
{
   if (rows != mRows)
   {
      //ensure the provided values have correct active numbers
      bool anyActiveNumberSet = false;
      bool anyOnDiskNumberSet = false;
      unsigned int skipFactor = 1;
      for (vector<DimensionDescriptor>::size_type count = 0; count != rows.size(); ++count)
      {
         if (rows[count].isActiveNumberValid())
         {
            anyActiveNumberSet = true;
            VERIFYNRV(rows[count].getActiveNumber() == count);
         }
         else
         {
            VERIFYNRV(!anyActiveNumberSet);
         }
         VERIFYNRV(rows[count].isOriginalNumberValid());
         if (rows[count].isOnDiskNumberValid())
         {
            anyOnDiskNumberSet = true;
            if (count > 0)
            {
               //determine skip factor on second iteration
               VERIFYNRV(rows[count].getOnDiskNumber() > rows[count-1].getOnDiskNumber());
               unsigned int curSkipFactor = rows[count].getOnDiskNumber() - rows[count-1].getOnDiskNumber();
               if (count > 1)
               {
                  //on any iteration after second, verify skip factor remains the same
                  VERIFYNRV(curSkipFactor == skipFactor);
               }
               skipFactor = curSkipFactor;
            }
         }
         else
         {
            VERIFYNRV(!anyOnDiskNumberSet);
         }
      }
      mRows = rows;
      mRowSkipFactor = skipFactor - 1;
      notify(SIGNAL_NAME(RasterDataDescriptor, RowsChanged), boost::any(mRows));
   }
}

const vector<DimensionDescriptor>& RasterDataDescriptorImp::getRows() const
{
   return mRows;
}

unsigned int RasterDataDescriptorImp::getRowSkipFactor() const
{
   return mRowSkipFactor;
}

DimensionDescriptor RasterDataDescriptorImp::getOriginalRow(unsigned int originalNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mRows.begin(); iter != mRows.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOriginalNumberValid() == true)
      {
         if (descriptor.getOriginalNumber() == originalNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getOnDiskRow(unsigned int onDiskNumber) const
{
   const RasterFileDescriptorImp* pFd = static_cast<const RasterFileDescriptorImp*>(getFileDescriptor());
   if (pFd != NULL)
   {
      DimensionDescriptor descriptor = pFd->getOnDiskRow(onDiskNumber);
      if (descriptor.isActiveNumberValid() == true)
      {
         return getActiveRow(descriptor.getActiveNumber());
      }
   }
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mRows.begin(); iter != mRows.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOnDiskNumberValid() == true)
      {
         if (descriptor.getOnDiskNumber() == onDiskNumber)
         {
            return descriptor;
         }
      }
   }
   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getActiveRow(unsigned int activeNumber) const
{
   VERIFYRV(activeNumber < mRows.size(), DimensionDescriptor());
   return mRows[activeNumber];
}

unsigned int RasterDataDescriptorImp::getRowCount() const
{
   return mRows.size();
}

void RasterDataDescriptorImp::setColumns(const vector<DimensionDescriptor>& columns)
{
   if (columns != mColumns)
   {
      //ensure the provided values have correct active numbers
      bool anyActiveNumberSet = false;
      bool anyOnDiskNumberSet = false;
      unsigned int skipFactor = 1;
      for (vector<DimensionDescriptor>::size_type count = 0; count != columns.size(); ++count)
      {
         if (columns[count].isActiveNumberValid())
         {
            anyActiveNumberSet = true;
            VERIFYNRV(columns[count].getActiveNumber() == count);
         }
         else
         {
            VERIFYNRV(!anyActiveNumberSet);
         }
         VERIFYNRV(columns[count].isOriginalNumberValid());
         if (columns[count].isOnDiskNumberValid())
         {
            anyOnDiskNumberSet = true;
            if (count > 0)
            {
               //determine skip factor on second iteration
               VERIFYNRV(columns[count].getOnDiskNumber() > columns[count-1].getOnDiskNumber());
               unsigned int curSkipFactor = columns[count].getOnDiskNumber() - columns[count - 1].getOnDiskNumber();
               if (count > 1)
               {
                  //on any iteration after second, verify skip factor remains the same
                  VERIFYNRV(curSkipFactor == skipFactor);
               }
               skipFactor = curSkipFactor;
            }
         }
         else
         {
            VERIFYNRV(!anyOnDiskNumberSet);
         }
      }
      mColumns = columns;
      mColumnSkipFactor = skipFactor - 1;
      notify(SIGNAL_NAME(RasterDataDescriptor, ColumnsChanged), boost::any(mColumns));
   }
}

const vector<DimensionDescriptor>& RasterDataDescriptorImp::getColumns() const
{
   return mColumns;
}

unsigned int RasterDataDescriptorImp::getColumnSkipFactor() const
{
   return mColumnSkipFactor;
}

DimensionDescriptor RasterDataDescriptorImp::getOriginalColumn(unsigned int originalNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mColumns.begin(); iter != mColumns.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOriginalNumberValid() == true)
      {
         if (descriptor.getOriginalNumber() == originalNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getOnDiskColumn(unsigned int onDiskNumber) const
{
   const RasterFileDescriptorImp* pFd = static_cast<const RasterFileDescriptorImp*>(getFileDescriptor());
   if (pFd != NULL)
   {
      DimensionDescriptor descriptor = pFd->getOnDiskColumn(onDiskNumber);
      if (descriptor.isActiveNumberValid() == true)
      {
         return getActiveColumn(descriptor.getActiveNumber());
      }
   }
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mColumns.begin(); iter != mColumns.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOnDiskNumberValid() == true)
      {
         if (descriptor.getOnDiskNumber() == onDiskNumber)
         {
            return descriptor;
         }
      }
   }
   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getActiveColumn(unsigned int activeNumber) const
{
   VERIFYRV(activeNumber < mColumns.size(), DimensionDescriptor());
   return mColumns[activeNumber];
}

unsigned int RasterDataDescriptorImp::getColumnCount() const
{
   return mColumns.size();
}

void RasterDataDescriptorImp::setBands(const vector<DimensionDescriptor>& bands)
{
   if (bands != mBands)
   {
      //ensure the provided values have correct active numbers
      bool anyActiveNumberSet = false;
      bool anyOnDiskNumberSet = false;
      for (vector<DimensionDescriptor>::size_type count = 0; count != bands.size(); ++count)
      {
         if (bands[count].isActiveNumberValid())
         {
            anyActiveNumberSet = true;
            VERIFYNRV(bands[count].getActiveNumber() == count);
         }
         else
         {
            VERIFYNRV(!anyActiveNumberSet);
         }
         VERIFYNRV(bands[count].isOriginalNumberValid());
         if (bands[count].isOnDiskNumberValid())
         {
            anyOnDiskNumberSet = true;
            if (count > 0)
            {
               VERIFYNRV(bands[count].getOnDiskNumber() > bands[count - 1].getOnDiskNumber());
            }
         }
         else
         {
            VERIFYNRV(!anyOnDiskNumberSet);
         }
      }
      mBands = bands;
      notify(SIGNAL_NAME(RasterDataDescriptor, BandsChanged), boost::any(mBands));
   }
}

const vector<DimensionDescriptor>& RasterDataDescriptorImp::getBands() const
{
   return mBands;
}

DimensionDescriptor RasterDataDescriptorImp::getOriginalBand(unsigned int originalNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mBands.begin(); iter != mBands.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOriginalNumberValid() == true)
      {
         if (descriptor.getOriginalNumber() == originalNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getOnDiskBand(unsigned int onDiskNumber) const
{
   const RasterFileDescriptorImp* pFd = static_cast<const RasterFileDescriptorImp*>(getFileDescriptor());
   if (pFd != NULL)
   {
      DimensionDescriptor descriptor = pFd->getOnDiskBand(onDiskNumber);
      if (descriptor.isActiveNumberValid() == true)
      {
         return getActiveBand(descriptor.getActiveNumber());
      }
   }
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mBands.begin(); iter != mBands.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isOnDiskNumberValid() == true)
      {
         if (descriptor.getOnDiskNumber() == onDiskNumber)
         {
            return descriptor;
         }
      }
   }
   return DimensionDescriptor();
}

DimensionDescriptor RasterDataDescriptorImp::getActiveBand(unsigned int activeNumber) const
{
   VERIFYRV(activeNumber < mBands.size(), DimensionDescriptor());
   return mBands[activeNumber];
}

unsigned int RasterDataDescriptorImp::getBandCount() const
{
   return mBands.size();
}

void RasterDataDescriptorImp::setXPixelSize(double pixelSize)
{
   if (pixelSize <= 0.0)
   {
      return;
   }

   if (pixelSize != mXPixelSize)
   {
      mXPixelSize = pixelSize;
      notify(SIGNAL_NAME(RasterDataDescriptor, PixelSizeChanged));
   }
}

double RasterDataDescriptorImp::getXPixelSize() const
{
   return mXPixelSize;
}

void RasterDataDescriptorImp::setYPixelSize(double pixelSize)
{
   if (pixelSize <= 0.0)
   {
      return;
   }

   if (pixelSize != mYPixelSize)
   {
      mYPixelSize = pixelSize;
      notify(SIGNAL_NAME(RasterDataDescriptor, PixelSizeChanged));
   }
}

double RasterDataDescriptorImp::getYPixelSize() const
{
   return mYPixelSize;
}

void RasterDataDescriptorImp::setUnits(const Units* pUnits)
{
   if ((pUnits != NULL) && (dynamic_cast<const UnitsAdapter*>(pUnits) != &mUnits))
   {
      mUnits.setUnits(pUnits);   // Any changes to the units values will automatically notify
                                 // Subject::Modified from within notifyUnitsModified()
   }
}

Units* RasterDataDescriptorImp::getUnits()
{
   return &mUnits;
}

const Units* RasterDataDescriptorImp::getUnits() const
{
   return &mUnits;
}

void RasterDataDescriptorImp::setGeoreferenceDescriptor(const GeoreferenceDescriptor* pGeorefDescriptor)
{
   if ((pGeorefDescriptor != NULL) && (mGeorefDescriptor.compare(pGeorefDescriptor) == false))
   {
      mGeorefDescriptor.clone(pGeorefDescriptor);  // Any changes to the descriptor values will automatically
                                                   // notify Subject::Modified from within notifyModified()
   }
}

GeoreferenceDescriptor* RasterDataDescriptorImp::getGeoreferenceDescriptor()
{
   return &mGeorefDescriptor;
}

const GeoreferenceDescriptor* RasterDataDescriptorImp::getGeoreferenceDescriptor() const
{
   return &mGeorefDescriptor;
}

void RasterDataDescriptorImp::setDefaultGeoreferencePlugIn()
{
   string plugInName;
   unsigned char plugInAffinity = Georeference::CAN_NOT_GEOREFERENCE;

   const vector<string>& plugIns = mGeorefDescriptor.getValidGeoreferencePlugIns();
   for (vector<string>::const_iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
   {
      string currentPlugInName = *iter;
      if (currentPlugInName.empty() == false)
      {
         PlugInResource pCurrentPlugIn(currentPlugInName);

         Georeference* pGeoreference = dynamic_cast<Georeference*>(pCurrentPlugIn.get());
         if (pGeoreference != NULL)
         {
            unsigned char currentPlugInAffinity =
               pGeoreference->getGeoreferenceAffinity(dynamic_cast<RasterDataDescriptor*>(this));
            if (currentPlugInAffinity > plugInAffinity)
            {
               plugInName = currentPlugInName;
               plugInAffinity = currentPlugInAffinity;
            }
         }
      }
   }

   mGeorefDescriptor.setGeoreferencePlugInName(plugInName);
}

void RasterDataDescriptorImp::setDisplayBand(RasterChannelType eColor, DimensionDescriptor band)
{
   switch (eColor)
   {
      case GRAY:
         if (band != mGrayBand)
         {
            mGrayBand = band;
            notify(SIGNAL_NAME(RasterDataDescriptor, DisplayBandChanged));
         }
         break;

      case RED:
         if (band != mRedBand)
         {
            mRedBand = band;
            notify(SIGNAL_NAME(RasterDataDescriptor, DisplayBandChanged));
         }
         break;

      case GREEN:
         if (band != mGreenBand)
         {
            mGreenBand = band;
            notify(SIGNAL_NAME(RasterDataDescriptor, DisplayBandChanged));
         }
         break;

      case BLUE:
         if (band != mBlueBand)
         {
            mBlueBand = band;
            notify(SIGNAL_NAME(RasterDataDescriptor, DisplayBandChanged));
         }
         break;

      default:
         break;
   }
}

DimensionDescriptor RasterDataDescriptorImp::getDisplayBand(RasterChannelType eColor) const
{
   DimensionDescriptor band;
   switch (eColor)
   {
      case GRAY:
         band = mGrayBand;
         break;
      case RED:
         band = mRedBand;
         break;
      case GREEN:
         band = mGreenBand;
         break;
      case BLUE:
         band = mBlueBand;
         break;
      default:
         break;
   }

   return band;
}

void RasterDataDescriptorImp::setDisplayMode(DisplayMode displayMode)
{
   if (displayMode != mDisplayMode)
   {
      mDisplayMode = displayMode;
      notify(SIGNAL_NAME(RasterDataDescriptor, DisplayModeChanged), boost::any(mDisplayMode));
   }
}

DisplayMode RasterDataDescriptorImp::getDisplayMode() const
{
   return mDisplayMode;
}

DataDescriptor* RasterDataDescriptorImp::copy(const string& name, DataElement* pParent) const
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(DataDescriptorImp::copy(name, pParent));
   if (pDescriptor != NULL)
   {
      pDescriptor->setDataType(mDataType);
      pDescriptor->setValidDataTypes(mValidDataTypes);
      pDescriptor->setInterleaveFormat(mInterleave);
      pDescriptor->setBadValues(mpBadValues.get());
      pDescriptor->setRows(mRows);
      pDescriptor->setColumns(mColumns);
      pDescriptor->setBands(mBands);
      pDescriptor->setXPixelSize(mXPixelSize);
      pDescriptor->setYPixelSize(mYPixelSize);
      pDescriptor->setUnits(&mUnits);
      pDescriptor->setGeoreferenceDescriptor(&mGeorefDescriptor);
      pDescriptor->setDisplayBand(GRAY, mGrayBand);
      pDescriptor->setDisplayBand(RED, mRedBand);
      pDescriptor->setDisplayBand(GREEN, mGreenBand);
      pDescriptor->setDisplayBand(BLUE, mBlueBand);
      pDescriptor->setDisplayMode(mDisplayMode);
   }

   return pDescriptor;
}

DataDescriptor* RasterDataDescriptorImp::copy(const string& name, const vector<string>& parent) const
{
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(DataDescriptorImp::copy(name, parent));
   if (pDescriptor != NULL)
   {
      pDescriptor->setDataType(mDataType);
      pDescriptor->setValidDataTypes(mValidDataTypes);
      pDescriptor->setInterleaveFormat(mInterleave);
      pDescriptor->setBadValues(mpBadValues.get());
      pDescriptor->setRows(mRows);
      pDescriptor->setColumns(mColumns);
      pDescriptor->setBands(mBands);
      pDescriptor->setXPixelSize(mXPixelSize);
      pDescriptor->setYPixelSize(mYPixelSize);
      pDescriptor->setUnits(&mUnits);
      pDescriptor->setGeoreferenceDescriptor(&mGeorefDescriptor);
      pDescriptor->setDisplayBand(GRAY, mGrayBand);
      pDescriptor->setDisplayBand(RED, mRedBand);
      pDescriptor->setDisplayBand(GREEN, mGreenBand);
      pDescriptor->setDisplayBand(BLUE, mBlueBand);
      pDescriptor->setDisplayMode(mDisplayMode);
   }

   return pDescriptor;
}

bool RasterDataDescriptorImp::clone(const DataDescriptor* pDescriptor)
{
   const RasterDataDescriptorImp* pRasterDescriptor = dynamic_cast<const RasterDataDescriptorImp*>(pDescriptor);
   if ((pRasterDescriptor == NULL) || (DataDescriptorImp::clone(pDescriptor) == false))
   {
      return false;
   }

   if (pRasterDescriptor != this)
   {
      setDataType(pRasterDescriptor->getDataType());
      setValidDataTypes(pRasterDescriptor->getValidDataTypes());
      setInterleaveFormat(pRasterDescriptor->getInterleaveFormat());
      setBadValues(pRasterDescriptor->getBadValues());
      setRows(pRasterDescriptor->getRows());
      setColumns(pRasterDescriptor->getColumns());
      setBands(pRasterDescriptor->getBands());
      setXPixelSize(pRasterDescriptor->getXPixelSize());
      setYPixelSize(pRasterDescriptor->getYPixelSize());
      setUnits(pRasterDescriptor->getUnits());
      setGeoreferenceDescriptor(pRasterDescriptor->getGeoreferenceDescriptor());
      setDisplayBand(GRAY, pRasterDescriptor->getDisplayBand(GRAY));
      setDisplayBand(RED, pRasterDescriptor->getDisplayBand(RED));
      setDisplayBand(GREEN, pRasterDescriptor->getDisplayBand(GREEN));
      setDisplayBand(BLUE, pRasterDescriptor->getDisplayBand(BLUE));
      setDisplayMode(pRasterDescriptor->getDisplayMode());
   }

   return true;
}

void RasterDataDescriptorImp::addToMessageLog(Message* pMessage) const
{
   DataDescriptorImp::addToMessageLog(pMessage);

   if (pMessage == NULL)
   {
      return;
   }

   // Rows
   pMessage->addProperty("Rows", getRowCount());

   // Columns
   pMessage->addProperty("Columns", getColumnCount());

   // Bands
   pMessage->addProperty("Bands", getBandCount());

   // Data type
   pMessage->addProperty("Data Type", mDataType);

   // Interleave format
   pMessage->addProperty("Interleave Format", mInterleave);

   // Bad values
   if (mpBadValues.get() != NULL)
   {
      pMessage->addProperty("Bad Values", mpBadValues->getBadValuesString());
   }

   // Pixel size
   pMessage->addProperty("X Pixel Size", mXPixelSize);
   pMessage->addProperty("Y Pixel Size", mYPixelSize);

   // Units
   pMessage->addProperty("Units Name", mUnits.getUnitName());
   pMessage->addProperty("Units Type", mUnits.getUnitType());
   pMessage->addProperty("Units Scale", mUnits.getScaleFromStandard());
   pMessage->addProperty("Units Min", mUnits.getRangeMin());
   pMessage->addProperty("Units Max", mUnits.getRangeMax());

   // Georeference descriptor
   pMessage->addProperty("Georeference on Import", mGeorefDescriptor.getGeoreferenceOnImport());
   pMessage->addProperty("Georeference Plug-In", mGeorefDescriptor.getGeoreferencePlugInName());
   pMessage->addProperty("Valid Georeference Plug-Ins", mGeorefDescriptor.getValidGeoreferencePlugIns());
   pMessage->addProperty("Create Layer", mGeorefDescriptor.getCreateLayer());
   pMessage->addProperty("Layer Name", mGeorefDescriptor.getLayerName());
   pMessage->addProperty("Display Layer", mGeorefDescriptor.getDisplayLayer());
   pMessage->addProperty("Coordinate Type", mGeorefDescriptor.getGeocoordType());
   pMessage->addProperty("Latitude/Longitude Format", mGeorefDescriptor.getLatLonFormat());

   // Gray band
   if (mGrayBand.isValid())
   {
      pMessage->addProperty("Gray Band", mGrayBand.getOriginalNumber() + 1);
   }

   // Red band
   if (mRedBand.isValid())
   {
      pMessage->addProperty("Red Band", mRedBand.getOriginalNumber() + 1);
   }

   // Green band
   if (mGreenBand.isValid())
   {
      pMessage->addProperty("Green Band", mGreenBand.getOriginalNumber() + 1);
   }

   // Blue band
   if (mBlueBand.isValid())
   {
      pMessage->addProperty("Blue Band", mBlueBand.getOriginalNumber() + 1);
   }

   // Display mode
   pMessage->addProperty("Display Mode",
      StringUtilities::toXmlString(mDisplayMode));
}

bool RasterDataDescriptorImp::toXml(XMLWriter* pXml) const
{
   if ((pXml == NULL) || (DataDescriptorImp::toXml(pXml) == false))
   {
      return false;
   }

   // Data type
   pXml->addAttr("dataType", mDataType);

   // Valid data types
   pXml->pushAddPoint(pXml->addElement("ValidDataTypes"));
   for (vector<EncodingType>::const_iterator iter = mValidDataTypes.begin(); iter != mValidDataTypes.end(); ++iter)
   {
      pXml->addText(StringUtilities::toXmlString(*iter), pXml->addElement("value"));
   }
   pXml->popAddPoint();

   // Bad values
   if (mpBadValues.get() != NULL)
   {
      pXml->pushAddPoint(pXml->addElement("BadValues"));
      if (!mpBadValues->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }

   // Rows
   pXml->pushAddPoint(pXml->addElement("rows"));
   XmlUtilities::serializeDimensionDescriptors("row", mRows, pXml);
   pXml->popAddPoint();

   // Columns
   pXml->pushAddPoint(pXml->addElement("columns"));
   XmlUtilities::serializeDimensionDescriptors("column", mColumns, pXml);
   pXml->popAddPoint();

   // Pixel size
   stringstream buf;
   buf << mXPixelSize << " " << mYPixelSize;
   pXml->addText(buf.str().c_str(), pXml->addElement("pixelSize"));

   // Units
   pXml->pushAddPoint(pXml->addElement("units"));
   if (mUnits.toXml(pXml) == false)
   {
      return false;
   }
   pXml->popAddPoint();

   // Georeference descriptor
   pXml->pushAddPoint(pXml->addElement("georeference"));
   if (mGeorefDescriptor.toXml(pXml) == false)
   {
      return false;
   }
   pXml->popAddPoint();

   // Interleave
   pXml->addAttr("interleaveFormat", mInterleave);

   // Bands
   pXml->pushAddPoint(pXml->addElement("bands"));
   XmlUtilities::serializeDimensionDescriptors("band", mBands, pXml);
   pXml->popAddPoint();

   // Gray Band
   if (mGrayBand.isValid() == true)
   {
      pXml->pushAddPoint(pXml->addElement("grayBand"));
      XmlUtilities::serializeDimensionDescriptor(mGrayBand, pXml);
      pXml->popAddPoint();
   }

   // Red Band
   if (mRedBand.isValid() == true)
   {
      pXml->pushAddPoint(pXml->addElement("redBand"));
      XmlUtilities::serializeDimensionDescriptor(mRedBand, pXml);
      pXml->popAddPoint();
   }

   // Green Band
   if (mGreenBand.isValid() == true)
   {
      pXml->pushAddPoint(pXml->addElement("greenBand"));
      XmlUtilities::serializeDimensionDescriptor(mGreenBand, pXml);
      pXml->popAddPoint();
   }

   // Blue Band
   if (mBlueBand.isValid() == true)
   {
      pXml->pushAddPoint(pXml->addElement("blueBand"));
      XmlUtilities::serializeDimensionDescriptor(mBlueBand, pXml);
      pXml->popAddPoint();
   }

   // Display Mode
   pXml->addAttr("displayMode", mDisplayMode);

   return true;
}

bool RasterDataDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   if (DataDescriptorImp::fromXml(pDocument, version) == false)
   {
      return false;
   }

   setBadValues(NULL);
   mValidDataTypes.clear();
   mRows.clear();
   mColumns.clear();
   mBands.clear();
   mGrayBand = DimensionDescriptor();
   mRedBand = DimensionDescriptor();
   mGreenBand = DimensionDescriptor();
   mBlueBand = DimensionDescriptor();

   UnitsAdapter units;
   mUnits.setUnits(&units);

   GeoreferenceDescriptorAdapter georefDescriptor;
   mGeorefDescriptor.clone(&georefDescriptor);

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);

   mDataType = StringUtilities::fromXmlString<EncodingType>(A(pElement->getAttribute(X("dataType"))));
   mInterleave = StringUtilities::fromXmlString<InterleaveFormatType>(A(pElement->getAttribute(X("interleaveFormat"))));
   mDisplayMode = StringUtilities::fromXmlString<DisplayMode>(A(pElement->getAttribute(X("displayMode"))));

   for (DOMNode* pChild = pDocument->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X("pixelSize")))
      {
         DOMNode* pGchld(pChild->getFirstChild());
         LocationType pixelSize(1.0, 1.0);
         XmlReader::StrToLocation(pGchld->getNodeValue(), pixelSize);
         mXPixelSize = pixelSize.mX;
         mYPixelSize = pixelSize.mY;
      }
      else if (XMLString::equals(pChild->getNodeName(), X("ValidDataTypes")))
      {
         for (DOMNode* pGrandchild = pChild->getFirstChild(); pGrandchild != NULL;
            pGrandchild = pGrandchild->getNextSibling())
         {
            if (XMLString::equals(pGrandchild->getNodeName(), X("value")))
            {
               DOMNode* pValue = pGrandchild->getFirstChild();
               if (pValue != NULL)
               {
                  EncodingType value = StringUtilities::fromXmlString<EncodingType>(A(pValue->getNodeValue()));
                  mValidDataTypes.push_back(value);
               }
            }
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("BadValues")))
      {
         if (mpBadValues.get() == NULL)
         {
            mpBadValues.reset(FactoryResource<BadValues>().release());
         }
         if (mpBadValues->fromXml(pChild, version) == false)
         {
            return false;
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("rows")))
      {
         XmlUtilities::deserializeDimensionDescriptors("row", mRows, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("columns")))
      {
         XmlUtilities::deserializeDimensionDescriptors("column", mColumns, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("bands")))
      {
         XmlUtilities::deserializeDimensionDescriptors("band", mBands, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("units")))
      {
         if (mUnits.fromXml(pChild, version) == false)
         {
            return false;
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("georeference")))
      {
         if (mGeorefDescriptor.fromXml(pChild, version) == false)
         {
            return false;
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("grayBand")))
      {
         XmlUtilities::deserializeDimensionDescriptor(mGrayBand, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("redBand")))
      {
         XmlUtilities::deserializeDimensionDescriptor(mRedBand, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("greenBand")))
      {
         XmlUtilities::deserializeDimensionDescriptor(mGreenBand, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("blueBand")))
      {
         XmlUtilities::deserializeDimensionDescriptor(mBlueBand, pChild);
      }
   }

   // Allow any data type if no valid data types were explicitly set
   if (mValidDataTypes.empty() == true)
   {
      mValidDataTypes = StringUtilities::getAllEnumValues<EncodingType>();
   }

   return true;
}

const string& RasterDataDescriptorImp::getObjectType() const
{
   static string sType("RasterDataDescriptorImp");
   return sType;
}

bool RasterDataDescriptorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOf(className);
}

void RasterDataDescriptorImp::getDataDescriptorTypes(vector<string>& classList)
{
   classList.push_back("RasterDataDescriptor");
   DataDescriptorImp::getDataDescriptorTypes(classList);
}

bool RasterDataDescriptorImp::isKindOfDataDescriptor(const string& className)
{
   if ((className == "RasterDataDescriptorImp") || (className == "RasterDataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOfDataDescriptor(className);
}
