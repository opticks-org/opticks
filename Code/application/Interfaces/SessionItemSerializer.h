/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#ifndef SESSION_ITEM_SERIALIZER_H
#define SESSION_ITEM_SERIALIZER_H

class XMLWriter;

#include "AppConfig.h"
#include <vector>

/**
 *  Used by SessionItems during session serialization to save their state in
 *  the session. The data stored via the SessionItemSerializer will be restored
 *  to the SessionItem on loading a session via the SessionItemDeserializer.
 *  If a SessionItem needs to be restored, but does not need to save any state
 *  information, it should call serialize(NULL,0) to ensure that it will be 
 *  recreated on session load. If a SessionItem does not do at least this on
 *  serialization, it will not be recreated on session restore. Conversely, if
 *  a SessionItem does not need to be recreated on session load, it should not
 *  call any of the serialize methods on the SessionItemSerializer.
 *
 *  @see        SessionItem
 *  @see        SessionItemDeserializer
 */
class SessionItemSerializer
{
public:
   /**
    *  Reserves a specified number of bytes in the session.
    *
    *  This method is called by a SessionItem during session serialization
    *  to specify the total number of bytes that the SessionItem will be saving
    *  to the session. If serialize() will only be called once, this method
    *  need not be called since the size can be inferred from the data - this
    *  method need only be called if serialize() will be called more than once.
    *  If this method is called more than once, only the first call will be 
    *  applied. When using multiple blocks, reserve() should be used to reserve
    *  the space needed for each block, not for then entire SessionItem.
    *  
    *  @param size
    *            The total number of bytes that will be serialized to this 
    *            object.
    *
    *  @see endBlock()
    */
   virtual void reserve(int64_t size) = 0;

   /**
    *  Saves the specified data in the session file.
    *
    *  This method is called by a SessionItem during session serialization
    *  to save the state of the SessionItem. The data provided to this method
    *  will be retrieved upon deserialization to recreate the SessionItem. If
    *  this method is to be called more than once, the reserve() method must be
    *  called to specify the total amount of space to reserve.
    *  
    *  @param data
    *            The data to be saved in the session. It can be in any format.
    *
    *  @return True if the data is successfully saved, or false otherwise
    */
   virtual bool serialize(const std::vector<unsigned char> &data) = 0;

   /**
    *  Saves the specified data in the session file.
    *
    *  This method is called by a SessionItem during session serialization
    *  to save the state of the SessionItem. The data provided to this method
    *  will be retrieved upon deserialization to recreate the SessionItem.
    *
    *  If the size is 0, this method will return true and the current block will be
    *  created. This can be used to force the creation of a size 0 block.
    *
    *  @param pData
    *            A pointer to the data to be written. The method will return
    *            false if this pointer is NULL and \b size is not 0.
    *
    *  @param size
    *           The size of the data in bytes
    *
    *  @return True if the data is successfully saved, or false otherwise
    */
   virtual bool serialize(const void *pData, int64_t size) = 0;

   /**
    *  Saves the specified data in the session file.
    *
    *  This method is called by a SessionItem during session serialization
    *  to save the state of the SessionItem. The data provided to this method
    *  will be retrieved upon deserialization to recreate the SessionItem. This
    *  method may not be called more than once. This is a convenience method
    *  for saving session information in XML format.
    *  
    *  @param writer
    *            An XMLWriter in which the state of the SessionItem has been
    *            written.
    *
    *  @return True if the data is successfully saved, or false otherwise
    */
   virtual bool serialize(XMLWriter &writer) = 0;

   /**
    *  Creates a new session item block.
    *
    *  Typically, SessionItem data can be saved as a single block of data either with
    *  a single call to serialize() or a call to reserve() followed by multiple calls
    *  to serialize(). It may be desirable in certain cases to block the data for a single
    *  SessionItem. The first block may contain an XML header, for example, and further
    *  blocks contain raw binary data. In this case, the XML is more easily separable
    *  from the binary data and can be more easily passed to an XML parser without the
    *  raw binary data interfering or slowing down the parse.
    *
    *  A SessionItem wishing to use this feature should call serialize() once, or reserve()
    *  followed by serialize() to write the first block. When finished with the block, call
    *  endBlock() to close the block and prepare the SessionItemSerializer for the next block.
    *  Call serialize() or reserve() followed by serialize() for the second block. Repeat this
    *  for additional blocks of data. The final block will implicitly call endBlock(). 
    *
    *  @note Different blocks will not necessarily be stored adjacent to each other. Different
    *        session serialization implementations may locate blocks in different files, or in
    *        different segments of the same file. SessionItem serialization should treat different
    *        blocks as discreet items. You can not, for example, write out a data cube with each
    *        band stored in a different block and expect that the entire data cube is contiguous
    *        on the disk.
    */
   virtual void endBlock() = 0;
};

#endif
