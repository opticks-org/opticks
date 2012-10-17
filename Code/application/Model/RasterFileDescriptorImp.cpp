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
#include "RasterFileDescriptor.h"
#include "RasterFileDescriptorImp.h"
#include "XmlUtilities.h"

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
   // Attach to the units object to notify when it changes
   VERIFYNR(mUnits.attach(SIGNAL_NAME(Subject, Modified), Slot(this, &RasterFileDescriptorImp::notifyUnitsModified)));
}

RasterFileDescriptorImp::~RasterFileDescriptorImp()
{
   clearBandFiles();
}

void RasterFileDescriptorImp::notifyUnitsModified(Subject &subject, const string &signal, const boost::any &data)
{
   if (&subject == &mUnits)
   {
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void RasterFileDescriptorImp::setHeaderBytes(unsigned int bytes)
{
   if (mHeaderBytes != bytes)
   {
      mHeaderBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, HeaderBytesChanged), boost::any(mHeaderBytes));
   }
}

unsigned int RasterFileDescriptorImp::getHeaderBytes() const
{
   return mHeaderBytes;
}

void RasterFileDescriptorImp::setTrailerBytes(unsigned int bytes)
{
   if (mTrailerBytes != bytes)
   {
      mTrailerBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, TrailerBytesChanged), boost::any(mTrailerBytes));
   }
}

unsigned int RasterFileDescriptorImp::getTrailerBytes() const
{
   return mTrailerBytes;
}

void RasterFileDescriptorImp::setPrelineBytes(unsigned int bytes)
{
   if (mPrelineBytes != bytes)
   {
      mPrelineBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, PrelineBytesChanged), boost::any(mPrelineBytes));
   }
}

unsigned int RasterFileDescriptorImp::getPrelineBytes() const
{
   return mPrelineBytes;
}

void RasterFileDescriptorImp::setPostlineBytes(unsigned int bytes)
{
   if (mPostlineBytes != bytes)
   {
      mPostlineBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, PostlineBytesChanged), boost::any(mPostlineBytes));
   }
}

unsigned int RasterFileDescriptorImp::getPostlineBytes() const
{
   return mPostlineBytes;
}

void RasterFileDescriptorImp::setPrebandBytes(unsigned int bytes)
{
   if (mPrebandBytes != bytes)
   {
      mPrebandBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, PrebandBytesChanged), boost::any(mPrebandBytes));
   }
}

unsigned int RasterFileDescriptorImp::getPrebandBytes() const
{
   return mPrebandBytes;
}

void RasterFileDescriptorImp::setPostbandBytes(unsigned int bytes)
{
   if (mPostbandBytes != bytes)
   {
      mPostbandBytes = bytes;
      notify(SIGNAL_NAME(RasterFileDescriptor, PostbandBytesChanged), boost::any(mPostbandBytes));
   }
}

unsigned int RasterFileDescriptorImp::getPostbandBytes() const
{
   return mPostbandBytes;
}

void RasterFileDescriptorImp::setBitsPerElement(unsigned int numBits)
{
   if (mBitsPerElement != numBits)
   {
      mBitsPerElement = numBits;
      notify(SIGNAL_NAME(RasterFileDescriptor, BitsPerElementChanged), boost::any(mBitsPerElement));
   }
}

unsigned int RasterFileDescriptorImp::getBitsPerElement() const
{
   return mBitsPerElement;
}

void RasterFileDescriptorImp::setInterleaveFormat(InterleaveFormatType format)
{
   if (mInterleave != format)
   {
      mInterleave = format;
      notify(SIGNAL_NAME(RasterFileDescriptor, InterleaveFormatChanged), boost::any(mInterleave));
   }
}

InterleaveFormatType RasterFileDescriptorImp::getInterleaveFormat() const
{
   return mInterleave;
}

void RasterFileDescriptorImp::setRows(const vector<DimensionDescriptor>& rows)
{
   if (rows == mRows)
   {
      return;
   }

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
   notify(SIGNAL_NAME(RasterFileDescriptor, RowsChanged), boost::any(mRows));
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
   if (onDiskNumber < mRows.size())
   {
      return mRows[onDiskNumber];
   }

   return DimensionDescriptor();
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
   if (columns == mColumns)
   {
      return;
   }

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
   notify(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged), boost::any(mColumns));
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
   if (onDiskNumber < mColumns.size())
   {
      return mColumns[onDiskNumber];
   }

   return DimensionDescriptor();
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
   if (bands == mBands)
   {
      return;
   }

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
   notify(SIGNAL_NAME(RasterFileDescriptor, BandsChanged), boost::any(mBands));
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
   if (onDiskNumber < mBands.size())
   {
      return mBands[onDiskNumber];
   }

   return DimensionDescriptor();
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
      notify(SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged));
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
      notify(SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged));
   }
}

double RasterFileDescriptorImp::getYPixelSize() const
{
   return mYPixelSize;
}

void RasterFileDescriptorImp::setUnits(const Units* pUnits)
{
   if ((pUnits != NULL) && (dynamic_cast<const UnitsAdapter*>(pUnits) != &mUnits))
   {
      mUnits.setUnits(pUnits);
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

Units* RasterFileDescriptorImp::getUnits()
{
   return &mUnits;
}

const Units* RasterFileDescriptorImp::getUnits() const
{
   return &mUnits;
}

void RasterFileDescriptorImp::setGcps(const list<GcpPoint>& gcps)
{
   if (gcps != mGcps)
   {
      mGcps = gcps;
      notify(SIGNAL_NAME(RasterFileDescriptor, GcpsChanged), boost::any(mGcps));
   }
}

const list<GcpPoint>& RasterFileDescriptorImp::getGcps() const
{
   return mGcps;
}

void RasterFileDescriptorImp::setBandFiles(const vector<string>& bandFiles)
{
   vector<string> currentBandFiles;
   for (vector<const Filename*>::const_iterator iter = mBandFiles.begin(); iter != mBandFiles.end(); ++iter)
   {
      const Filename* pBandFile = *iter;
      if (pBandFile != NULL)
      {
         currentBandFiles.push_back(pBandFile->getFullPathAndName());
      }
   }

   if (bandFiles == currentBandFiles)
   {
      return;
   }

   clearBandFiles();
   for (vector<string>::const_iterator iter = bandFiles.begin(); iter != bandFiles.end(); ++iter)
   {
      Filename* pBandFile = new FilenameImp(*iter);
      if (pBandFile != NULL)
      {
         mBandFiles.push_back(pBandFile);
      }
   }
   notify(SIGNAL_NAME(RasterFileDescriptor, BandFilesChanged), boost::any(mBandFiles));
}

void RasterFileDescriptorImp::setBandFiles(const vector<const Filename*>& bandFiles)
{
   vector<string> newBandFiles;
   for (vector<const Filename*>::const_iterator iter = bandFiles.begin(); iter != bandFiles.end(); ++iter)
   {
      const Filename* pBandFile = *iter;
      if (pBandFile != NULL)
      {
         newBandFiles.push_back(pBandFile->getFullPathAndName());
      }
   }

   vector<string> currentBandFiles;
   for (vector<const Filename*>::const_iterator iter = mBandFiles.begin(); iter != mBandFiles.end(); ++iter)
   {
      const Filename* pBandFile = *iter;
      if (pBandFile != NULL)
      {
         currentBandFiles.push_back(pBandFile->getFullPathAndName());
      }
   }

   if (newBandFiles == currentBandFiles)
   {
      return;
   }

   clearBandFiles();
   for (vector<const Filename*>::const_iterator iter = bandFiles.begin(); iter != bandFiles.end(); ++iter)
   {
      VERIFYNRV(*iter != NULL);
      Filename* pBandFile = new FilenameImp(**iter);
      if (pBandFile != NULL)
      {
         mBandFiles.push_back(pBandFile);
      }
   }
   notify(SIGNAL_NAME(RasterFileDescriptor, BandFilesChanged), boost::any(mBandFiles));
}

const vector<const Filename*>& RasterFileDescriptorImp::getBandFiles() const
{
   return mBandFiles;
}

bool RasterFileDescriptorImp::clone(const FileDescriptor* pFileDescriptor)
{
   const RasterFileDescriptorImp* pRasterFileDescriptor = dynamic_cast<const RasterFileDescriptorImp*>(pFileDescriptor);
   if ((pRasterFileDescriptor == NULL) || (FileDescriptorImp::clone(pFileDescriptor) == false))
   {
      return false;
   }

   if (pRasterFileDescriptor != this)
   {
      setHeaderBytes(pRasterFileDescriptor->getHeaderBytes());
      setTrailerBytes(pRasterFileDescriptor->getTrailerBytes());
      setPrelineBytes(pRasterFileDescriptor->getPrelineBytes());
      setPostlineBytes(pRasterFileDescriptor->getPostlineBytes());
      setPrebandBytes(pRasterFileDescriptor->getPrebandBytes());
      setPostbandBytes(pRasterFileDescriptor->getPostbandBytes());
      setBitsPerElement(pRasterFileDescriptor->getBitsPerElement());
      setInterleaveFormat(pRasterFileDescriptor->getInterleaveFormat());
      setRows(pRasterFileDescriptor->getRows());
      setColumns(pRasterFileDescriptor->getColumns());
      setBands(pRasterFileDescriptor->getBands());
      setXPixelSize(pRasterFileDescriptor->getXPixelSize());
      setYPixelSize(pRasterFileDescriptor->getYPixelSize());
      setUnits(pRasterFileDescriptor->getUnits());
      setGcps(pRasterFileDescriptor->getGcps());
      setBandFiles(pRasterFileDescriptor->getBandFiles());
   }

   return true;
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
   }
}
