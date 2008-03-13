/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Hdf5CustomReader.h"
#include "Hdf5CustomWriter.h"
#include "Hdf5Resource.h"

class DateTime;
class TempDateTimeTransferStruct;

class DateTimeReaderWriter : public Hdf5CustomReader, public Hdf5CustomWriter
{
public:
   DateTimeReaderWriter();
   DateTimeReaderWriter(hid_t dataType);
   ~DateTimeReaderWriter();

   unsigned int getSupportedDimensionality() const;
   Hdf5TypeResource getReadMemoryType() const;
   bool setDataToWrite(void* pObject);
   Hdf5TypeResource getWriteMemoryType() const;
   Hdf5TypeResource getWriteFileType() const;
   Hdf5DataSpaceResource createDataSpace() const;
   bool setReadDataSpace(const std::vector<hsize_t>& dataSpace);
   void* getReadBuffer() const;
   const void* getWriteBuffer() const;
   void* getValue() const;
   bool isValid() const;

private:
   mutable DateTime* mpValue; //not owned by class
   mutable TempDateTimeTransferStruct* mpWriteBuffer;
   mutable TempDateTimeTransferStruct* mpReadBuffer;
   hid_t mDataType;
};
