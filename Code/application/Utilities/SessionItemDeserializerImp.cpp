/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SessionItemDeserializerImp.h"
#include "xmlreader.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

SessionItemDeserializerImp::SessionItemDeserializerImp(const string &filename, const std::vector<int64_t> &blockSizes) : 
   mBaseFilename(filename),
   mCurrentBlock(0),
   mBlockSizes(blockSizes)
{
}

SessionItemDeserializerImp::~SessionItemDeserializerImp()
{
}

bool SessionItemDeserializerImp::deserialize(void *pData, unsigned int size)
{
   if (!ensureFileIsOpen())
   {
      return false;
   }
   else
   {
      int64_t bytesRead = mFile.read(pData, size);
      return bytesRead == static_cast<int64_t>(size);
   }
}

bool SessionItemDeserializerImp::deserialize(vector<unsigned char> &data)
{
   if (data.size() == 0)
   {
      return true;
   }
   else
   {
      return deserialize(&data.front(), data.size());
   }
}

DOMDocument *SessionItemDeserializerImp::deserialize(XmlReader &reader)
{
   ensureFileIsClosed();
   return reader.parse(filenameForCurrentBlock());
}

DOMElement *SessionItemDeserializerImp::deserialize(
   XmlReader &reader, const char *pRootElementName)
{
   DOMDocument *pDocument = deserialize(reader);
   if (pDocument)
   {
      DOMElement* pRootElement = pDocument->getDocumentElement();
      if (pRootElement != NULL)
      {
         if(XMLString::equals(pRootElement->getNodeName(), X(pRootElementName)))
         {
            return pRootElement;
         }
      }
   }
   return NULL;
}

void SessionItemDeserializerImp::nextBlock()
{
   ++mCurrentBlock;
   ensureFileIsClosed();
}

void SessionItemDeserializerImp::ensureFileIsClosed()
{
   if (mFile.validHandle())
   {
      mFile.close();
   }
}

bool SessionItemDeserializerImp::ensureFileIsOpen()
{
   if (!mFile.validHandle() && !mFile.open(filenameForCurrentBlock(), O_BINARY, S_IREAD))
   {
      return false;
   }
   return true;
}

string SessionItemDeserializerImp::filenameForCurrentBlock() const
{
   if (mCurrentBlock == 0)
   {
      return mBaseFilename;
   }
   else
   {
      stringstream buf;
      buf << mBaseFilename << "." << mCurrentBlock;
      return buf.str();
   }
}

std::vector<int64_t> SessionItemDeserializerImp::getBlockSizes() const
{
   return mBlockSizes;
}
