#ifndef ossimRpfColorGrayscaleOffsetRecord_HEADER
#define ossimRpfColorGrayscaleOffsetRecord_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIosFwd.h>

class ossimRpfColorGrayscaleOffsetRecord
{
public:
   friend std::ostream& operator <<(
      std::ostream& out, const ossimRpfColorGrayscaleOffsetRecord& data);
   
   ossimRpfColorGrayscaleOffsetRecord();

   ossimErrorCode parseStream(ossim::istream& in,
                              ossimByteOrder byteOrder);
   
   void print(std::ostream& out)const;

   void clearFields();
   ossim_uint16 getColorGrayscaleTableId()const
      {
         return theColorGrayscaleTableId;
      }
   ossim_uint32 getNumberOfColorGrayscaleRecords()const
      {
         return theNumberOfColorGrayscaleRecords;
      }
   ossim_uint32 getColorGrayscaleTableOffset()const
      {
         return theColorGrayscaleTableOffset;
      }
private:
   ossim_uint16  theColorGrayscaleTableId;
   ossim_uint32  theNumberOfColorGrayscaleRecords;
   unsigned char theColorGrayscaleElementLength;
   ossim_uint16  theHistogramRecordLength;
   ossim_uint32  theColorGrayscaleTableOffset;
   ossim_uint32  theHistogramTableOffset;
};

#endif
