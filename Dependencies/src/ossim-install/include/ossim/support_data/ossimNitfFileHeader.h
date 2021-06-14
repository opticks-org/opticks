//*******************************************************************
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#ifndef ossimNitfFileHeader_HEADER
#define ossimNitfFileHeader_HEADER 1

#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIosFwd.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/support_data/ossimNitfTagInformation.h>
#include <ossim/support_data/ossimNitfDesInformation.h>

#include <vector>

class ossimNitfImageHeader;
class ossimNitfSymbolHeader;
class ossimNitfTextHeader;
class ossimNitfLabelHeader;
class ossimNitfRegisteredTag;
class ossimNitfDataExtensionSegment;
// These structures are just data holders and will
// not be used directly by outside users.  This is for internal use
//
class OSSIM_DLL ossimNitfImageOffsetInformation : public ossimObject
{
public:
   virtual std::ostream& print(std::ostream& out) const;

   ossimNitfImageOffsetInformation(ossim_uint64 headerOffset,
                                   ossim_uint64 dataOffset)
      :theImageHeaderOffset(headerOffset),
       theImageDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theImageHeaderOffset;
   ossim_uint64 theImageDataOffset;
};


class OSSIMDLLEXPORT ossimNitfSymbolOffsetInformation : public ossimObject
{
public:
   ossimNitfSymbolOffsetInformation(ossim_uint64 headerOffset,
                                    ossim_uint64 dataOffset)
      :theSymbolHeaderOffset(headerOffset),
       theSymbolDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theSymbolHeaderOffset;
   ossim_uint64 theSymbolDataOffset;
};

class OSSIMDLLEXPORT ossimNitfGraphicOffsetInformation : public ossimObject
{
public:
   ossimNitfGraphicOffsetInformation(ossim_uint64 headerOffset,
                                    ossim_uint64 dataOffset)
      :theGraphicHeaderOffset(headerOffset),
       theGraphicDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theGraphicHeaderOffset;
   ossim_uint64 theGraphicDataOffset;
};

class OSSIMDLLEXPORT ossimNitfLabelOffsetInformation : public ossimObject
{
public:
   ossimNitfLabelOffsetInformation(ossim_uint64 headerOffset,
                                   ossim_uint64 dataOffset)
      :theLabelHeaderOffset(headerOffset),
       theLabelDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theLabelHeaderOffset;
   ossim_uint64 theLabelDataOffset;
};

class OSSIMDLLEXPORT ossimNitfTextOffsetInformation : public ossimObject
{
public:
   ossimNitfTextOffsetInformation(ossim_uint64 headerOffset,
                                  ossim_uint64 dataOffset)
      :theTextHeaderOffset(headerOffset),
       theTextDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theTextHeaderOffset;
   ossim_uint64 theTextDataOffset;
   
};

class OSSIMDLLEXPORT ossimNitfDataExtSegOffsetInformation : public ossimObject
{
public:
   ossimNitfDataExtSegOffsetInformation(ossim_uint64 headerOffset,
                                        ossim_uint64 dataOffset)
      :theDataExtSegHeaderOffset(headerOffset),
       theDataExtSegDataOffset(dataOffset)
      {
      }
   
   ossim_uint64 theDataExtSegHeaderOffset;
   ossim_uint64 theDataExtSegDataOffset;   
};

class OSSIM_DLL ossimNitfFileHeader : public ossimObject,
                                      public ossimPropertyInterface
{
public:
   friend class ossimNitfFile;

   ossimNitfFileHeader();
   virtual ~ossimNitfFileHeader();

   virtual void parseStream(ossim::istream& in)= 0;
   virtual void writeStream(ossim::ostream& out)=0;
   
   virtual bool isEncrypted()const=0;
   virtual ossim_int32 getNumberOfImages()const=0;
   virtual ossim_int32 getNumberOfLabels()const=0;
   virtual ossim_int32 getNumberOfSymbols()const=0;
   virtual ossim_int32 getNumberOfGraphics()const=0;
   virtual ossim_int32 getNumberOfTextSegments()const=0;
   virtual ossim_int32 getNumberOfDataExtSegments()const=0;
   virtual ossim_int32 getNumberOfReservedExtSegments()const=0;
   virtual ossim_int32 getHeaderSize()const=0;
   
   void addTag(const ossimNitfTagInformation& tag, bool unique=true);
   virtual void removeTag(const ossimString& tagName);
   virtual bool  getTagInformation(ossimNitfTagInformation& tag,
                                   int idx)const;

   virtual std::vector<ossimNitfTagInformation> getAllTags() { return theTagList; }
   virtual void setAllTags(std::vector<ossimNitfTagInformation> tagList) { theTagList = tagList; }

   virtual int getNumberOfTags()const;

   virtual bool getTag(ossimNitfTagInformation& tagInfo,
                       const ossimString& tagName)const;
   ossimRefPtr<ossimNitfRegisteredTag> getTagData(const ossimString &tagName);
   const ossimRefPtr<ossimNitfRegisteredTag> getTagData(
       const ossimString &tagName) const;
   virtual bool getDesInformation(ossimNitfDesInformation &desInfo,
                                  const ossimString &desId,
                                  bool exactMatch = false);
   virtual ossim_int64 getFileSize()const=0;
   virtual const char* getVersion()const=0;
   virtual const char* getDateTime()const=0;
   virtual ossimDrect getImageRect()const=0;
   virtual ossimString getSecurityClassification()const=0;
   
   /**
   * isValid will test if the fields are valid and will return true or false.
   */
   virtual bool isValid()const=0;
   
   bool hasImages()const;
   bool hasSymbols()const;
   bool hasGraphics()const;
   bool hasText()const;
   bool hasLabels()const;
   bool hasDataExtSegments()const;

   virtual ossimNitfImageHeader* getNewImageHeader(ossim_uint32 imageNumber,
                                                   ossim::istream& in)const=0;
   virtual ossimNitfSymbolHeader* getNewSymbolHeader(ossim_uint32 symbolNumber,
                                                    ossim::istream& in)const=0;
   virtual ossimNitfLabelHeader* getNewLabelHeader(ossim_uint32 labelNumber,
                                                   ossim::istream& in)const=0;
   virtual ossimNitfTextHeader* getNewTextHeader(ossim_uint32 textNumber,
                                                ossim::istream& in)const=0;
   virtual ossimNitfDataExtensionSegment* getNewDataExtensionSegment(
      ossim_int32 dataExtNumber, ossim::istream& in)const=0;
   
   virtual ossimNitfImageHeader*    allocateImageHeader()const=0;
   virtual ossimNitfSymbolHeader*   allocateSymbolHeader()const=0;
   virtual ossimNitfLabelHeader*    allocateLabelHeader()const=0;
   virtual ossimNitfTextHeader*     allocateTextHeader()const=0;
   virtual ossimNitfDataExtensionSegment *allocateDataExtSegment()const=0;
   virtual const std::vector<ossimNitfDesInformation>& getDesInfoList()const;

    virtual ossim_uint32 getTotalTagLength() const;

   /** @brief Sets file length (FL) field. */
   virtual void setFileLength(ossim_uint64 fileLength) = 0;

   /** @brief Sets header length (HL) field. */
   virtual void setHeaderLength(ossim_uint64 headerLength) = 0;

   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix=std::string()) const;
   
   /**
    * @brief print tags method that outputs a key/value type format adding
    * prefix to keys.
    */
   virtual std::ostream& printTags(
      std::ostream& out,
      const std::string& prefix=std::string()) const;
   
   virtual bool saveState(ossimKeywordlist& kwl, const ossimString& prefix="")const;
   
protected:
   std::vector<ossimNitfTagInformation> theTagList;
   std::vector<ossimNitfDesInformation> theDesList;
   
TYPE_DATA
};

#endif
