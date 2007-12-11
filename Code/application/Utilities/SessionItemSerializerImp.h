/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef SESSION_ITEM_SERIALIZER_IMP_H
#define SESSION_ITEM_SERIALIZER_IMP_H

#include "AppConfig.h"
#include "FileResource.h"
#include "SessionItemSerializer.h"

#include <stdio.h>
#include <string>
#include <vector>

class SessionItemSerializerImp : public SessionItemSerializer
{
public:
   SessionItemSerializerImp(std::string filename);
   virtual ~SessionItemSerializerImp();
   void reserve(int64_t size);
   bool serialize(const std::vector<unsigned char> &data);
   bool serialize(const void *pData, int64_t size);
   bool serialize(XMLWriter &writer);
   std::vector<int64_t> getBlockSizes() const;
   void endBlock();
   unsigned int getBlockCount() const { return mTotalBlocks; }
private:
   std::string mBaseFilename;
   std::string mFilename;
   unsigned int mTotalBlocks;
   LargeFileResource mFile;
   int64_t mBytesReserved;
   int64_t mBytesWritten;
   std::vector<int64_t> mBlockSizes;
};

#endif
