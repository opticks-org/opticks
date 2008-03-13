/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SessionItem.h"
#include "SessionItemSerializerImp.h"
#include "xmlwriter.h"

using namespace std;

SessionItemSerializerImp::SessionItemSerializerImp(string filename) :
   mBaseFilename(filename),
   mFilename(filename),
   mTotalBlocks(1),
   mBytesReserved(0),
   mBytesWritten(0)
{
}

SessionItemSerializerImp::~SessionItemSerializerImp()
{
}

std::vector<int64_t> SessionItemSerializerImp::getBlockSizes() const
{
   return mBlockSizes;
}

void SessionItemSerializerImp::reserve(int64_t size)
{
   if (mBytesReserved == 0)
   {
      mBytesReserved = size;
      mBlockSizes.push_back(size);
   }
}

bool SessionItemSerializerImp::serialize(const vector<unsigned char> &data)
{
   return serialize(&data.front(), data.size());
}

bool SessionItemSerializerImp::serialize(const void *pData, int64_t size)
{
   if (pData == NULL && size > 0)
   {
      return false;
   }

   if (mBytesReserved == 0)
   {
      reserve(size);
   }

   if (!mFile.validHandle())
   {
      if (!mFile.open(mFilename, O_WRONLY | O_CREAT | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE))
      {
         return false;
      }
   }

   if (size != 0)
   {
      if (size + mBytesWritten > mBytesReserved)
      {
         return false;
      }

      int64_t bytesWritten = mFile.write(pData, size);
      mBytesWritten += bytesWritten;
      if (bytesWritten != size)
      {
         return false;
      }
   }

   return true;
}

bool SessionItemSerializerImp::serialize(XMLWriter &writer)
{
   string text = writer.writeToString();
   return serialize(text.c_str(), text.length());
}

void SessionItemSerializerImp::endBlock()
{
   if (mFile.validHandle())
   {
      mFile.close();
   }
   mBytesReserved = mBytesWritten = 0;
   stringstream buf;
   buf << mBaseFilename << "." << mTotalBlocks++;
   mFilename = buf.str();
}
