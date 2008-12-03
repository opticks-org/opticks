/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SESSIONITEMDESERIALIZERIMP_H
#define SESSIONITEMDESERIALIZERIMP_H

#include "AppConfig.h"
#include "FileResource.h"
#include "SessionItemDeserializer.h"

#include <string>
#include <vector>

class SessionItemDeserializerImp : public SessionItemDeserializer, public SessionItemDeserializerExt1
{
public:
   SessionItemDeserializerImp(const std::string &filename, const std::vector<int64_t> &blockSizes);
   ~SessionItemDeserializerImp();
   bool deserialize(void *pData, unsigned int size);
   bool deserialize(std::vector<unsigned char> &data);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *deserialize(XmlReader &reader);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *deserialize(XmlReader &reader, const char *pRootElementName);
   void nextBlock();
   std::vector<int64_t> getBlockSizes() const;
   int getCurrentBlock() const;

private:
   void ensureFileIsClosed();
   bool ensureFileIsOpen();
   std::string filenameForCurrentBlock() const;

   std::string mBaseFilename;
   LargeFileResource mFile;
   int mCurrentBlock;
   std::vector<int64_t> mBlockSizes;
};

#endif
