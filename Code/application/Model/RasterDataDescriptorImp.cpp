/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "RasterDataDescriptor.h"
#include "RasterDataDescriptorImp.h"
#include "RasterFileDescriptorImp.h"
#include "RasterUtilities.h"
#include "StringUtilities.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

RasterDataDescriptorImp::RasterDataDescriptorImp(const string& name, const string& type,
   DataElement* pParent) :
   DataDescriptorImp(name, type, pParent),
   mDataType(INT1UBYTE),
   mValidDataTypes(StringUtilities::getAllEnumValues<EncodingType>()),
   mInterleave(BIP),
   mRowSkipFactor(0),
   mColumnSkipFactor(0),
   mXPixelSize(1.0),
   mYPixelSize(1.0),
   mDisplayMode(GRAYSCALE_MODE)
{
}

RasterDataDescriptorImp::RasterDataDescriptorImp(const string& name, const string& type, const vector<string>& parent) :
   DataDescriptorImp(name, type, parent),
   mDataType(INT1UBYTE),
   mValidDataTypes(StringUtilities::getAllEnumValues<EncodingType>()),
   mInterleave(BIP),
   mRowSkipFactor(0),
   mColumnSkipFactor(0),
   mXPixelSize(1.0),
   mYPixelSize(1.0),
   mDisplayMode(GRAYSCALE_MODE)
{
}

RasterDataDescriptorImp::~RasterDataDescriptorImp()
{
}

void RasterDataDescriptorImp::setDataType(EncodingType dataType)
{
   if (dataType != mDataType)
   {
      mDataType = dataType;
      notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(Subject, Modified));
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

void RasterDataDescriptorImp::setBadValues(const vector<int>& badValues)
{
   if (badValues != mBadValues)
   {
      mBadValues = badValues;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

const vector<int>& RasterDataDescriptorImp::getBadValues() const
{
   return mBadValues;
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
               int curSkipFactor = rows[count].getOnDiskNumber() - rows[count-1].getOnDiskNumber();
               VERIFYNRV(curSkipFactor >= 1);
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
      notify(SIGNAL_NAME(Subject, Modified));
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
               int curSkipFactor = columns[count].getOnDiskNumber() - columns[count - 1].getOnDiskNumber();
               VERIFYNRV(curSkipFactor >= 1);
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
      notify(SIGNAL_NAME(Subject, Modified));
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
               int numberDiff = bands[count].getOnDiskNumber() - bands[count - 1].getOnDiskNumber();
               VERIFYNRV(numberDiff >= 1);
            }
         }
         else
         {
            VERIFYNRV(!anyOnDiskNumberSet);
         }
      }
      mBands = bands;
      notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double RasterDataDescriptorImp::getYPixelSize() const
{
   return mYPixelSize;
}

void RasterDataDescriptorImp::setUnits(const UnitsImp* pUnits)
{
   if ((pUnits != NULL) && (pUnits != &mUnits))
   {
      mUnits = *pUnits;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

UnitsImp* RasterDataDescriptorImp::getUnits()
{
   return &mUnits;
}

const UnitsImp* RasterDataDescriptorImp::getUnits() const
{
   return &mUnits;
}

void RasterDataDescriptorImp::setDisplayBand(RasterChannelType eColor, DimensionDescriptor band)
{
   switch (eColor)
   {
      case GRAY:
         if (band != mGrayBand)
         {
            mGrayBand = band;
            notify(SIGNAL_NAME(Subject, Modified));
         }
         break;

      case RED:
         if (band != mRedBand)
         {
            mRedBand = band;
            notify(SIGNAL_NAME(Subject, Modified));
         }
         break;

      case GREEN:
         if (band != mGreenBand)
         {
            mGreenBand = band;
            notify(SIGNAL_NAME(Subject, Modified));
         }
         break;

      case BLUE:
         if (band != mBlueBand)
         {
            mBlueBand = band;
            notify(SIGNAL_NAME(Subject, Modified));
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
      notify(SIGNAL_NAME(Subject, Modified));
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
      pDescriptor->setBadValues(mBadValues);
      pDescriptor->setRows(mRows);
      pDescriptor->setColumns(mColumns);
      pDescriptor->setBands(mBands);
      pDescriptor->setXPixelSize(mXPixelSize);
      pDescriptor->setYPixelSize(mYPixelSize);
      pDescriptor->setUnits(&mUnits);
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
      pDescriptor->setBadValues(mBadValues);
      pDescriptor->setRows(mRows);
      pDescriptor->setColumns(mColumns);
      pDescriptor->setBands(mBands);
      pDescriptor->setXPixelSize(mXPixelSize);
      pDescriptor->setYPixelSize(mYPixelSize);
      pDescriptor->setUnits(&mUnits);
      pDescriptor->setDisplayBand(GRAY, mGrayBand);
      pDescriptor->setDisplayBand(RED, mRedBand);
      pDescriptor->setDisplayBand(GREEN, mGreenBand);
      pDescriptor->setDisplayBand(BLUE, mBlueBand);
      pDescriptor->setDisplayMode(mDisplayMode);
   }

   return pDescriptor;
}

void RasterDataDescriptorImp::addToMessageLog(Message* pMessage) const
{
   DataDescriptorImp::addToMessageLog(pMessage);

   if (pMessage != NULL)
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
   pMessage->addProperty("Bad Values", mBadValues);

   // Pixel size
   pMessage->addProperty("X Pixel Size", mXPixelSize);
   pMessage->addProperty("Y Pixel Size", mYPixelSize);

   // Units
   pMessage->addProperty("Units Name", mUnits.getUnitName());
   pMessage->addProperty("Units Type", mUnits.getUnitType());
   pMessage->addProperty("Units Scale", mUnits.getScaleFromStandard());
   pMessage->addProperty("Units Min", mUnits.getRangeMin());
   pMessage->addProperty("Units Max", mUnits.getRangeMax());

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
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = DataDescriptorImp::toXml(pXml);
   if (bSuccess == true)
   {
      pXml->addAttr("dataType", mDataType);

      // Valid data types
      pXml->pushAddPoint(pXml->addElement("ValidDataTypes"));
      for (vector<EncodingType>::const_iterator iter = mValidDataTypes.begin(); iter != mValidDataTypes.end(); ++iter)
      {
         pXml->addText(StringUtilities::toXmlString(*iter), pXml->addElement("value"));
      }
      pXml->popAddPoint();

      // Bad values
      pXml->pushAddPoint(pXml->addElement("BadValues"));
      vector<int>::const_iterator valuesIter;
      for (valuesIter = mBadValues.begin(); valuesIter != mBadValues.end(); ++valuesIter)
      {
         stringstream buf;
         buf << *valuesIter;
         pXml->addText(buf.str().c_str(), pXml->addElement("value"));
      }
      pXml->popAddPoint();

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
      bSuccess = mUnits.toXml(pXml);
      pXml->popAddPoint();
      // Interleave
      pXml->addAttr("interleaveFormat", mInterleave);

      // Bands
      pXml->pushAddPoint(pXml->addElement("bands"));
      XmlUtilities::serializeDimensionDescriptors("band", mBands, pXml);
      pXml->popAddPoint();

      // Gray Band
      if ((bSuccess == true) && (mGrayBand.isValid()))
      {
         pXml->pushAddPoint(pXml->addElement("grayBand"));
         XmlUtilities::serializeDimensionDescriptor(mGrayBand, pXml);
         pXml->popAddPoint();
      }

      // Red Band
      if ((bSuccess == true) && (mRedBand.isValid()))
      {
         pXml->pushAddPoint(pXml->addElement("redBand"));
         XmlUtilities::serializeDimensionDescriptor(mRedBand, pXml);
         pXml->popAddPoint();
      }

      // Green Band
      if ((bSuccess == true) && (mGreenBand.isValid()))
      {
         pXml->pushAddPoint(pXml->addElement("greenBand"));
         XmlUtilities::serializeDimensionDescriptor(mGreenBand, pXml);
         pXml->popAddPoint();
      }

      // Blue Band
      if ((bSuccess == true) && (mBlueBand.isValid()))
      {
         pXml->pushAddPoint(pXml->addElement("blueBand"));
         XmlUtilities::serializeDimensionDescriptor(mBlueBand, pXml);
         pXml->popAddPoint();
      }

      // Display Mode
      if (bSuccess == true)
      {
         pXml->addAttr("displayMode", mDisplayMode);
      }
   }

   return bSuccess;
}

bool RasterDataDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   bool success = DataDescriptorImp::fromXml(pDocument, version);
   if (!success)
   {
      return false;
   }

   mBadValues.clear();
   mValidDataTypes.clear();
   mRows.clear();
   mColumns.clear();
   mBands.clear();
   mGrayBand = DimensionDescriptor();
   mRedBand = DimensionDescriptor();
   mGreenBand = DimensionDescriptor();
   mBlueBand = DimensionDescriptor();

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);

   mDataType = StringUtilities::fromXmlString<EncodingType>(
      A(pElement->getAttribute(X("dataType"))));

   mInterleave = StringUtilities::fromXmlString<InterleaveFormatType>(
      A(pElement->getAttribute(X("interleaveFormat"))));

   mDisplayMode = StringUtilities::fromXmlString<DisplayMode>(
      A(pElement->getAttribute(X("displayMode"))));

   for (DOMNode* pChild = pDocument->getFirstChild(); success && pChild != NULL; pChild = pChild->getNextSibling())
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
         for (DOMNode* pGrandchild = pChild->getFirstChild(); pGrandchild != NULL;
            pGrandchild = pGrandchild->getNextSibling())
         {
            if (XMLString::equals(pGrandchild->getNodeName(), X("value")))
            {
               DOMNode* pValue = pGrandchild->getFirstChild();
               if (pValue != NULL)
               {
                  int value = StringUtilities::fromXmlString<int>(A(pValue->getNodeValue()));
                  mBadValues.push_back(value);
               }
            }
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
         success = mUnits.fromXml(pChild, version);
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

   return success;
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
