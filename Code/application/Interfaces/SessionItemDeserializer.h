/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef SESSION_ITEM_DESERIALIZER_H
#define SESSION_ITEM_DESERIALIZER_H

#include "AppConfig.h"
#include "XercesIncludes.h"

#include <vector>

class XmlReader;

/**
 *  Used by SessionItems during session deserialization to restore their state 
 *  from the saved session.
 *
 *  @see        SessionItem, SessionItemSerializer
 */
class SessionItemDeserializer
{
public:
   /**
    *  Restores a portion of the data from a serialized SessionItem.
    *
    *  This method reads some data from the session and provides it back to the
    *  caller. The data will be retrieved from the current block. The data in a
    *  block in the session can be retrieved incrementally via multiple calls 
    *  to this method.
    *
    *  @param pData
    *               A pointer to a buffer into which the data is to be placed.
    *               The buffer must be big enough to hold the specified number
    *               of bytes of data.
    *  @param size
    *               The number of bytes to be placed into the buffer
    *
    *  @return True if the specified number of bytes were successfully 
    *                retrieved from the session or false otherwise.
    */
   virtual bool deserialize(void *pData, unsigned int size) = 0;

   /**
    *  Restores a portion of the data from a serialized SessionItem.
    *
    *  This method reads some data from the session and provides it back to the
    *  caller. The data will be retrieved from the current block. The data in a
    *  block in the session can be retrieved incrementally via multiple calls 
    *  to this method.
    *
    *  @param data
    *               A vector into which the data should be placed. The size of
    *               the vector determines the amount of data that will be
    *               retrieved from the session.
    *
    *  @return True if enough data to fill the vector were successfully 
    *                retrieved from the session or false otherwise.
    */
   virtual bool deserialize(std::vector<unsigned char> &data) = 0;

   /**
    *  Restores a portion of the data from a serialized SessionItem.
    *
    *  This method reads all of the data from a block in the session, parses it
    *  as XML and returns the root node.
    *
    *  @param reader
    *               The XmlReader to parse the session data with.
    *
    *  @return The XML root node if the parse was successful, or NULL otherwise.
    */
   virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *deserialize(XmlReader &reader) = 0;

   /**
    *  Restores a portion of the data from a serialized SessionItem.
    *
    *  This is a convenience function that reads all of the data from a block 
    *  in the session, parses it as XML and returns the root node if the root 
    *  node has the specified name.
    *
    *  @param reader
    *               The XmlReader to parse the session data with.
    *
    *  @param pRootElementName
    *               The name of the root element.
    *
    *  @return The XML root node if the parse was successful, or \c NULL otherwise.
    */
   virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *deserialize(XmlReader &reader, const char *pRootElementName) = 0;

   /**
    *  Accessor for the current block index.
    *
    *  @return The block number of the current block.
    */
   virtual int getCurrentBlock() const = 0;

   /**
    *  Moves to the next block of the serialized session.
    *
    *  This method moves the deserializer to the next block of serialized data
    *  in the session. This method should be used when deserializing data from
    *  a SessionItem that stored its data in multiple blocks via 
    *  SessionItemSerializer::endBlock().
    *
    *  @see SessionItemSerializer
    */
   virtual void nextBlock() = 0;

   /**
    *  Returns the sizes of the blocks in the serialized item.
    *
    *  This method returns the sizes of the blocks that were saved when the
    *  SessionItem was serialized.
    *
    *  @return A vector containing the size of each block.
    */
   virtual std::vector<int64_t> getBlockSizes() const = 0;

protected:
   /**
    *  Destroys the SessionItemDeserializer object.
    *
    *  The SessionItemDeserializer object is automatically deleted by
    *  SessionManager.  Plug-ins do not need to destroy it.
    */
   virtual ~SessionItemDeserializer() {}
};

#endif
