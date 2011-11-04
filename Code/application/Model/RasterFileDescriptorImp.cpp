/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GcpListImp.h"
#include "ObjectResource.h"
#include "RasterFileDescriptorImp.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

RasterFileDescriptorImp::RasterFileDescriptorImp() :
   mHeaderBytes(0),
   mTrailerBytes(0),
   mPrelineBytes(0),
   mPostlineBytes(0),
   mPrebandBytes(0),
   mPostbandBytes(0),
   mBitsPerElement(0),
   mInterleave(BIP),
   mXPixelSize(1.0),
   mYPixelSize(1.0)
{
}

RasterFileDescriptorImp::~RasterFileDescriptorImp()
{
   clearBandFiles();
}

RasterFileDescriptorImp& RasterFileDescriptorImp::operator =(const RasterFileDescriptorImp& descriptor)
{
   if (this != &descriptor)
   {
      FileDescriptorImp::operator =(descriptor);

      mHeaderBytes = descriptor.mHeaderBytes;
      mTrailerBytes = descriptor.mTrailerBytes;
      mPrelineBytes = descriptor.mPrelineBytes;
      mPostlineBytes = descriptor.mPostlineBytes;
      mPrebandBytes = descriptor.mPrebandBytes;
      mPostbandBytes = descriptor.mPostbandBytes;
      mBitsPerElement = descriptor.mBitsPerElement;
      mInterleave = descriptor.mInterleave;

      setRows(descriptor.mRows);
      setColumns(descriptor.mColumns);
      setBands(descriptor.mBands);

      mXPixelSize = descriptor.mXPixelSize;
      mYPixelSize = descriptor.mYPixelSize;
      mUnits = descriptor.mUnits;
      mGcps = descriptor.mGcps;

      setBandFiles(descriptor.mBandFiles);
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

void RasterFileDescriptorImp::setHeaderBytes(unsigned int bytes)
{
   if (mHeaderBytes == bytes)
   {
      return;
   }
   mHeaderBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getHeaderBytes() const
{
   return mHeaderBytes;
}

void RasterFileDescriptorImp::setTrailerBytes(unsigned int bytes)
{
   if (mTrailerBytes == bytes)
   {
      return;
   }
   mTrailerBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getTrailerBytes() const
{
   return mTrailerBytes;
}

void RasterFileDescriptorImp::setPrelineBytes(unsigned int bytes)
{
   if (mPrelineBytes == bytes)
   {
      return;
   }
   mPrelineBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getPrelineBytes() const
{
   return mPrelineBytes;
}

void RasterFileDescriptorImp::setPostlineBytes(unsigned int bytes)
{
   if (mPostlineBytes == bytes)
   {
      return;
   }
   mPostlineBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getPostlineBytes() const
{
   return mPostlineBytes;
}

void RasterFileDescriptorImp::setPrebandBytes(unsigned int bytes)
{
   if (mPrebandBytes == bytes)
   {
      return;
   }
   mPrebandBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getPrebandBytes() const
{
   return mPrebandBytes;
}

void RasterFileDescriptorImp::setPostbandBytes(unsigned int bytes)
{
   if (mPostbandBytes == bytes)
   {
      return;
   }
   mPostbandBytes = bytes;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getPostbandBytes() const
{
   return mPostbandBytes;
}

void RasterFileDescriptorImp::setBitsPerElement(unsigned int numBits)
{
   if (mBitsPerElement == numBits)
   {
      return;
   }
   mBitsPerElement = numBits;
   notify(SIGNAL_NAME(Subject, Modified));
}

unsigned int RasterFileDescriptorImp::getBitsPerElement() const
{
   return mBitsPerElement;
}

void RasterFileDescriptorImp::setInterleaveFormat(InterleaveFormatType format)
{
   if (mInterleave == format)
   {
      return;
   }
   mInterleave = format;
   notify(SIGNAL_NAME(Subject, Modified));
}

InterleaveFormatType RasterFileDescriptorImp::getInterleaveFormat() const
{
   return mInterleave;
}

void RasterFileDescriptorImp::setRows(const vector<DimensionDescriptor>& rows)
{
   //ensure the provided values have correct on-disk numbers
   bool anyActiveNumberFound = false;
   unsigned int lastActiveNumber = 0;
   bool firstSkipFactorDetermined = false;
   unsigned int skipFactor = 1;
   for (vector<DimensionDescriptor>::size_type count = 0; count != rows.size(); ++count)
   {
      VERIFYNRV(rows[count].isOnDiskNumberValid());
      VERIFYNRV(rows[count].getOnDiskNumber() == count);
      VERIFYNRV(rows[count].isOriginalNumberValid());
      if (rows[count].isActiveNumberValid())
      {
         if (anyActiveNumberFound)
         {
            VERIFYNRV(rows[count].getActiveNumber() > lastActiveNumber);
            unsigned int curSkipFactor = rows[count].getActiveNumber() - lastActiveNumber;
            if (firstSkipFactorDetermined)
            {
               VERIFYNRV(curSkipFactor == skipFactor);
            }
            skipFactor = curSkipFactor;
            firstSkipFactorDetermined = true;
         }
         anyActiveNumberFound = true;
         lastActiveNumber = rows[count].getActiveNumber();
      }
   }
   mRows = rows;
   notify(SIGNAL_NAME(Subject, Modified));
}

const vector<DimensionDescriptor>& RasterFileDescriptorImp::getRows() const
{
   return mRows;
}

DimensionDescriptor RasterFileDescriptorImp::getOriginalRow(unsigned int originalNumber) const
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

DimensionDescriptor RasterFileDescriptorImp::getOnDiskRow(unsigned int onDiskNumber) const
{
   VERIFYRV(onDiskNumber < mRows.size(), DimensionDescriptor());
   return mRows[onDiskNumber];
}

DimensionDescriptor RasterFileDescriptorImp::getActiveRow(unsigned int activeNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mRows.begin(); iter != mRows.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isActiveNumberValid() == true)
      {
         if (descriptor.getActiveNumber() == activeNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

unsigned int RasterFileDescriptorImp::getRowCount() const
{
   return mRows.size();
}

void RasterFileDescriptorImp::setColumns(const vector<DimensionDescriptor>& columns)
{
   //ensure the provided values have correct on-disk numbers
   bool anyActiveNumberFound = false;
   unsigned int lastActiveNumber = 0;
   bool firstSkipFactorDetermined = false;
   unsigned int skipFactor = 1;
   for (vector<DimensionDescriptor>::size_type count = 0; count != columns.size(); ++count)
   {
      VERIFYNRV(columns[count].isOnDiskNumberValid());
      VERIFYNRV(columns[count].getOnDiskNumber() == count);
      VERIFYNRV(columns[count].isOriginalNumberValid());
      if (columns[count].isActiveNumberValid())
      {
         if (anyActiveNumberFound)
         {
            VERIFYNRV(columns[count].getActiveNumber() > lastActiveNumber);
            unsigned int curSkipFactor = columns[count].getActiveNumber() - lastActiveNumber;
            if (firstSkipFactorDetermined)
            {
               VERIFYNRV(curSkipFactor == skipFactor);
            }
            skipFactor = curSkipFactor;
            firstSkipFactorDetermined = true;
         }
         anyActiveNumberFound = true;
         lastActiveNumber = columns[count].getActiveNumber();
      }
   }
   mColumns = columns;
   notify(SIGNAL_NAME(Subject, Modified));
}

const vector<DimensionDescriptor>& RasterFileDescriptorImp::getColumns() const
{
   return mColumns;
}

DimensionDescriptor RasterFileDescriptorImp::getOriginalColumn(unsigned int originalNumber) const
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

DimensionDescriptor RasterFileDescriptorImp::getOnDiskColumn(unsigned int onDiskNumber) const
{
   VERIFYRV(onDiskNumber < mColumns.size(), DimensionDescriptor());
   return mColumns[onDiskNumber];
}

DimensionDescriptor RasterFileDescriptorImp::getActiveColumn(unsigned int activeNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mColumns.begin(); iter != mColumns.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isActiveNumberValid() == true)
      {
         if (descriptor.getActiveNumber() == activeNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

unsigned int RasterFileDescriptorImp::getColumnCount() const
{
   return mColumns.size();
}

void RasterFileDescriptorImp::setBands(const vector<DimensionDescriptor>& bands)
{
   //ensure the provided values have correct on-disk numbers
   bool anyActiveNumberFound = false;
   unsigned int lastActiveNumber = 0;
   for (vector<DimensionDescriptor>::size_type count = 0; count != bands.size(); ++count)
   {
      VERIFYNRV(bands[count].isOnDiskNumberValid());
      VERIFYNRV(bands[count].getOnDiskNumber() == count);
      VERIFYNRV(bands[count].isOriginalNumberValid());
      if (bands[count].isActiveNumberValid())
      {
         if (anyActiveNumberFound)
         {
            VERIFYNRV(bands[count].getActiveNumber() > lastActiveNumber);
         }
         anyActiveNumberFound = true;
         lastActiveNumber = bands[count].getActiveNumber();
      }
   }
   mBands = bands;
   notify(SIGNAL_NAME(Subject, Modified));
}

const vector<DimensionDescriptor>& RasterFileDescriptorImp::getBands() const
{
   return mBands;
}

DimensionDescriptor RasterFileDescriptorImp::getOriginalBand(unsigned int originalNumber) const
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

DimensionDescriptor RasterFileDescriptorImp::getOnDiskBand(unsigned int onDiskNumber) const
{
   VERIFYRV(onDiskNumber < mBands.size(), DimensionDescriptor());
   return mBands[onDiskNumber];
}

DimensionDescriptor RasterFileDescriptorImp::getActiveBand(unsigned int activeNumber) const
{
   vector<DimensionDescriptor>::const_iterator iter;
   for (iter = mBands.begin(); iter != mBands.end(); ++iter)
   {
      DimensionDescriptor descriptor = *iter;
      if (descriptor.isActiveNumberValid() == true)
      {
         if (descriptor.getActiveNumber() == activeNumber)
         {
            return descriptor;
         }
      }
   }

   return DimensionDescriptor();
}

unsigned int RasterFileDescriptorImp::getBandCount() const
{
   return mBands.size();
}

void RasterFileDescriptorImp::setXPixelSize(double pixelSize)
{
   if (pixelSize != mXPixelSize)
   {
      mXPixelSize = pixelSize;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double RasterFileDescriptorImp::getXPixelSize() const
{
   return mXPixelSize;
}

void RasterFileDescriptorImp::setYPixelSize(double pixelSize)
{
   if (pixelSize != mYPixelSize)
   {
      mYPixelSize = pixelSize;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

double RasterFileDescriptorImp::getYPixelSize() const
{
   return mYPixelSize;
}

void RasterFileDescriptorImp::setUnits(const UnitsImp* pUnits)
{
   if (pUnits != NULL)
   {
      mUnits = *pUnits;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

UnitsImp* RasterFileDescriptorImp::getUnits()
{
   return &mUnits;
}

const UnitsImp* RasterFileDescriptorImp::getUnits() const
{
   return &mUnits;
}

void RasterFileDescriptorImp::setGcps(const list<GcpPoint>& gcps)
{
   mGcps = gcps;
   notify(SIGNAL_NAME(Subject, Modified));
}

const list<GcpPoint>& RasterFileDescriptorImp::getGcps() const
{
   return mGcps;
}

void RasterFileDescriptorImp::setBandFiles(const vector<string>& bandFiles)
{
   clearBandFiles();
   for (vector<string>::const_iterator bandFileName = bandFiles.begin();
                                      bandFileName != bandFiles.end();
                                      ++bandFileName)
   {
      Filename* pBandFile = new FilenameImp(*bandFileName);
      if (pBandFile != NULL)
      {
         mBandFiles.push_back(pBandFile);
      }
   }
   notify(SIGNAL_NAME(Subject, Modified));
}

void RasterFileDescriptorImp::setBandFiles(const vector<const Filename*>& bandFiles)
{
   clearBandFiles();
   for (vector<const Filename*>::const_iterator bandFileName = bandFiles.begin();
                                               bandFileName != bandFiles.end();
                                               ++bandFileName)
   {
      VERIFYNRV(*bandFileName != NULL);
      Filename* pBandFile = new FilenameImp(**bandFileName);
      if (pBandFile != NULL)
      {
         mBandFiles.push_back(pBandFile);
      }
   }
   notify(SIGNAL_NAME(Subject, Modified));
}

const vector<const Filename*>& RasterFileDescriptorImp::getBandFiles() const
{
   return mBandFiles;
}

void RasterFileDescriptorImp::addToMessageLog(Message* pMessage) const
{
   FileDescriptorImp::addToMessageLog(pMessage);

   if (pMessage == NULL)
   {
      return;
   }

   // Rows
   pMessage->addProperty("File Rows", getRowCount());

   // Columns
   pMessage->addProperty("File Columns", getColumnCount());

   // Bands
   pMessage->addProperty("File Bands", getBandCount());

   // Bits per element
   pMessage->addProperty("Bits Per Element", mBitsPerElement);

   // Header bytes
   pMessage->addProperty("Header Bytes", mHeaderBytes);

   // Trailer bytes
   pMessage->addProperty("Trailer Bytes", mTrailerBytes);

   // Preline bytes
   pMessage->addProperty("Preline Bytes", mPrelineBytes);

   // Postline bytes
   pMessage->addProperty("Postline Bytes", mPostlineBytes);

   // TODO: GCPs

   // Pixel size
   pMessage->addProperty("File X Pixel Size", mXPixelSize);
   pMessage->addProperty("File Y Pixel Size", mYPixelSize);

   // Units
   pMessage->addProperty("File Units Name", mUnits.getUnitName());
   pMessage->addProperty("File Units Type", mUnits.getUnitType());
   pMessage->addProperty("File Units Scale", mUnits.getScaleFromStandard());
   pMessage->addProperty("File Units Min", mUnits.getRangeMin());
   pMessage->addProperty("File Units Max", mUnits.getRangeMax());

   // Interleave format
   pMessage->addProperty("File Interleave Format", mInterleave);

   // Band files
   pMessage->addProperty("Band Files", mBandFiles);

   // Preband bytes
   pMessage->addProperty("Preband Bytes", mPrebandBytes);

   // Postband bytes
   pMessage->addProperty("Postband Bytes", mPostbandBytes);

}

bool RasterFileDescriptorImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = FileDescriptorImp::toXml(pXml);
   if (bSuccess == true)
   {
      pXml->addAttr("type", "RasterFileDescriptor");
      pXml->addAttr("headerBytes", mHeaderBytes);
      pXml->addAttr("trailerBytes", mTrailerBytes);
      pXml->addAttr("prelineBytes", mPrelineBytes);
      pXml->addAttr("postlineBytes", mPostlineBytes);
      pXml->addAttr("prebandBytes", mPrebandBytes);
      pXml->addAttr("postbandBytes", mPostbandBytes);
      pXml->addAttr("bitsPerElement", mBitsPerElement);
      pXml->addAttr("interleaveFormat", mInterleave);

      // Rows
      pXml->pushAddPoint(pXml->addElement("rows"));
      XmlUtilities::serializeDimensionDescriptors("row", mRows, pXml);
      pXml->popAddPoint();

      // Columns
      pXml->pushAddPoint(pXml->addElement("columns"));
      XmlUtilities::serializeDimensionDescriptors("column", mColumns, pXml);
      pXml->popAddPoint();

      // Bands
      pXml->pushAddPoint(pXml->addElement("bands"));
      XmlUtilities::serializeDimensionDescriptors("band", mBands, pXml);
      pXml->popAddPoint();

      // Pixel size
      stringstream buf;
      buf << mXPixelSize << " " << mYPixelSize;
      pXml->addText(buf.str().c_str(), pXml->addElement("pixelSize"));

      // Units
      pXml->pushAddPoint(pXml->addElement("units"));
      bSuccess = mUnits.toXml(pXml);
      pXml->popAddPoint();

      // GCPs
      bSuccess = bSuccess && GcpListImp::gcpsToXml(mGcps.begin(), mGcps.end(), pXml);

      // Band files
      for (vector<const Filename*>::const_iterator bandFile = mBandFiles.begin();
                                                  bandFile != mBandFiles.end();
                                                  ++bandFile)
      {
         if (*bandFile != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("bandFile"));
            pXml->addText(StringUtilities::toXmlString(**bandFile));
            pXml->popAddPoint();
         }
      }
   }

   return bSuccess;
}

bool RasterFileDescriptorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   VERIFY(pDocument != NULL);

   bool success = FileDescriptorImp::fromXml(pDocument, version);
   if (!success)
   {
      return false;
   }

   mRows.clear();
   mColumns.clear();
   mBands.clear();
   mGcps.clear();
   clearBandFiles();

   DOMElement* pElement = static_cast<DOMElement*>(pDocument);

   XmlReader::StringStreamAssigner<unsigned int> parser;
   mHeaderBytes = parser(A(pElement->getAttribute(X("headerBytes"))));
   mTrailerBytes = parser(A(pElement->getAttribute(X("trailerBytes"))));
   mPrelineBytes = parser(A(pElement->getAttribute(X("prelineBytes"))));
   mPostlineBytes = parser(A(pElement->getAttribute(X("postlineBytes"))));
   mPrebandBytes = parser(A(pElement->getAttribute(X("prebandBytes"))));
   mPostbandBytes = parser(A(pElement->getAttribute(X("postbandBytes"))));
   mBitsPerElement = parser(A(pElement->getAttribute(X("bitsPerElement"))));
   bool error;
   mInterleave = StringUtilities::fromXmlString<InterleaveFormatType>(
      A(pElement->getAttribute(X("interleaveFormat"))), &error);
   success = !error;

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
      else if (XMLString::equals(pChild->getNodeName(), X("rows")))
      {
         XmlUtilities::deserializeDimensionDescriptors("row", mRows, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("columns")))
      {
         XmlUtilities::deserializeDimensionDescriptors("column", mColumns, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("bandFile")))
      {
         string bandFile = A(pChild->getTextContent());
         if (bandFile.empty() == false)
         {
            bool error;
            FactoryResource<Filename> pTempFilename(StringUtilities::fromXmlString<Filename*>(bandFile, &error));
            success = !error;
            if (success)
            {
               mBandFiles.push_back(pTempFilename.release());
            }
         }
      }
      else if (XMLString::equals(pChild->getNodeName(), X("bands")))
      {
         XmlUtilities::deserializeDimensionDescriptors("band", mBands, pChild);
      }
      else if (XMLString::equals(pChild->getNodeName(), X("units")))
      {
         success = mUnits.fromXml(pChild, version);
      }
   }
   success = success && GcpListImp::xmlToGcps(back_insert_iterator<list<GcpPoint> >(mGcps), pDocument, version);

   notify(SIGNAL_NAME(Subject, Modified));
   return success;
}

const string& RasterFileDescriptorImp::getObjectType() const
{
   static string sType("RasterFileDescriptorImp");
   return sType;
}

bool RasterFileDescriptorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOf(className);
}

void RasterFileDescriptorImp::getFileDescriptorTypes(vector<string>& classList)
{
   classList.push_back("RasterFileDescriptor");
   FileDescriptorImp::getFileDescriptorTypes(classList);
}

bool RasterFileDescriptorImp::isKindOfFileDescriptor(const string& className)
{
   if ((className == "RasterFileDescriptorImp") || (className == "RasterFileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOfFileDescriptor(className);
}

void RasterFileDescriptorImp::clearBandFiles()
{
   if (!mBandFiles.empty())
   {
      for (vector<const Filename*>::iterator bandFile = mBandFiles.begin(); bandFile != mBandFiles.end(); ++bandFile)
      {
         delete dynamic_cast<const FilenameImp*>(*bandFile);
      }
      mBandFiles.clear();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void XmlUtilities::serializeDimensionDescriptor(const DimensionDescriptor& desc, XMLWriter* pXml)
{
   // This method is meant for a single DimensionDescriptor and does not serialize "count" or "skipFactor" attributes.
   VERIFYNRV(pXml != NULL);
   if (desc.isOriginalNumberValid())
   {
      pXml->addAttr("originalNumber", desc.getOriginalNumber());
   }
   if (desc.isOnDiskNumberValid())
   {
      pXml->addAttr("onDiskNumber", desc.getOnDiskNumber());
   }
   if (desc.isActiveNumberValid())
   {
      pXml->addAttr("activeNumber", desc.getActiveNumber());
   }
}

void XmlUtilities::deserializeDimensionDescriptor(DimensionDescriptor& desc,
                                                  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode)
{
   // This method is meant for a single DimensionDescriptor and does not deserialize "count" or "skipFactor" attributes.
   VERIFYNRV(pNode != NULL);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement = static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pNode);
   XmlReader::StringStreamAssigner<unsigned int> valueParser;
   if (pElement->hasAttribute(X("originalNumber")))
   {
      desc.setOriginalNumber(valueParser(A(pElement->getAttribute(X("originalNumber")))));
   }
   if (pElement->hasAttribute(X("onDiskNumber")))
   {
      desc.setOnDiskNumber(valueParser(A(pElement->getAttribute(X("onDiskNumber")))));
   }
   if (pElement->hasAttribute(X("activeNumber")))
   {
      desc.setActiveNumber(valueParser(A(pElement->getAttribute(X("activeNumber")))));
   }
}

void XmlUtilities::serializeDimensionDescriptors(const string& elementName,
                                                 const vector<DimensionDescriptor>& desc, XMLWriter* pXml)
{
   VERIFYNRV(pXml != NULL);
   for (vector<DimensionDescriptor>::const_iterator iter = desc.begin(); iter != desc.end(); ++iter)
   {
      pXml->pushAddPoint(pXml->addElement(elementName));
      XmlUtilities::serializeDimensionDescriptor(*iter, pXml);

      // Determine how many contiguous descriptors with a constant skip factor exist.
      unsigned int count = 1;
      unsigned int skipFactor = 0;
      bool skipFactorValid = false;
      for (vector<DimensionDescriptor>::const_iterator iter2 = iter + 1; iter2 != desc.end(); ++iter, ++iter2)
      {
         // Ensure that precisely the same set of numbers is defined.
         if ((iter->isOriginalNumberValid() != iter2->isOriginalNumberValid()) ||
            (iter->isOnDiskNumberValid() != iter2->isOnDiskNumberValid()) ||
            (iter->isActiveNumberValid() != iter2->isActiveNumberValid()))
         {
            break;
         }

         if (iter->isOriginalNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getOriginalNumber() <= iter->getOriginalNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getOriginalNumber() - iter->getOriginalNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }
         if (iter->isOnDiskNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getOnDiskNumber() <= iter->getOnDiskNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getOnDiskNumber() - iter->getOnDiskNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }
         if (iter->isActiveNumberValid())
         {
            // Ensure that the numbers are ascending.
            if (iter2->getActiveNumber() <= iter->getActiveNumber())
            {
               break;
            }

            // Ensure that the difference of the numbers matches the current skip factor.
            unsigned int delta = iter2->getActiveNumber() - iter->getActiveNumber() - 1;
            if (skipFactorValid == false)
            {
               skipFactor = delta;
               skipFactorValid = true;
            }
            else if (skipFactor != delta)
            {
               break;
            }
         }

         ++count;
      }

      if (count > 1)
      {
         pXml->addAttr("count", count);
         if (skipFactor > 0)
         {
            pXml->addAttr("skipFactor", skipFactor);
         }
      }

      pXml->popAddPoint();
   }
}

void XmlUtilities::deserializeDimensionDescriptors(const string& elementName,
                                                   vector<DimensionDescriptor>& desc,
                                                   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pNode)
{
   VERIFYNRV(pNode != NULL);

   // Since this uses a vector, for performance, compute how many descriptors will be created and pre-allocate memory.
   unsigned int numDescriptors = 0;
   for (DOMNode* pChild = pNode->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X(elementName.c_str())))
      {
         unsigned int count = 1;
         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pChild);
         XmlReader::StringStreamAssigner<unsigned int> valueParser;
         if (pElement->hasAttribute(X("count")))
         {
            count = valueParser(A(pElement->getAttribute(X("count"))));
            if (count == 0)
            {
               count = 1;
            }
         }

         numDescriptors += count;
      }
   }
   desc.reserve(numDescriptors);

   for (DOMNode* pChild = pNode->getFirstChild(); pChild != NULL; pChild = pChild->getNextSibling())
   {
      if (XMLString::equals(pChild->getNodeName(), X(elementName.c_str())))
      {
         DimensionDescriptor descriptor;
         XmlUtilities::deserializeDimensionDescriptor(descriptor, pChild);

         unsigned int count = 1;
         unsigned int skipFactor = 0;

         XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pElement =
            static_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(pChild);
         XmlReader::StringStreamAssigner<unsigned int> valueParser;
         if (pElement->hasAttribute(X("count")))
         {
            count = valueParser(A(pElement->getAttribute(X("count"))));
            if (count == 0)
            {
               count = 1;
            }

            if (count > 1 && pElement->hasAttribute(X("skipFactor")))
            {
               skipFactor = valueParser(A(pElement->getAttribute(X("skipFactor"))));
            }
         }

         unsigned int originalNumber = descriptor.getOriginalNumber();
         unsigned int onDiskNumber = descriptor.getOnDiskNumber();
         unsigned int activeNumber = descriptor.getActiveNumber();
         for (unsigned int i = 0; i < count; ++i)
         {
            desc.push_back(descriptor);
            if (descriptor.isOriginalNumberValid())
            {
               originalNumber += skipFactor + 1;
               descriptor.setOriginalNumber(originalNumber);
            }
            if (descriptor.isOnDiskNumberValid())
            {
               onDiskNumber += skipFactor + 1;
               descriptor.setOnDiskNumber(onDiskNumber);
            }
            if (descriptor.isActiveNumberValid())
            {
               activeNumber += skipFactor + 1;
               descriptor.setActiveNumber(activeNumber);
            }
         }
      }
   }

   // Sanity check -- if this verify is ever triggered, investigate the algorithm above for correctness.
   VERIFYNRV(numDescriptors == desc.size());
}
