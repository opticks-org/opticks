/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include "Filename.h"
#include "TypeAwareObject.h"
#include "Serializable.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>

class Message;

/**
 *  Describes how a data element is stored in a file on disk.
 *
 *  A file descriptor contains data that describes how a data element is stored
 *  on-disk, including the filename.  There are two primary uses for a file
 *  descriptor:
 *  - On import, a file descriptor is used to indicate how the imported data is
 *    stored in the file on disk.  It is typically set into a data descriptor
 *    by an importer.
 *  - On export, a file descriptor is used to indicate how the exported data
 *    should be saved to disk.  This file descriptor is separate from a
 *    potentially imported file descriptor and does not have a parent data
 *    descriptor.  It can be created by any object that will perform the
 *    export.
 *
 *  If a DataElement is created by an algorithm and was not imported from a
 *  file, the element's data descriptor will have a \b NULL file descriptor.
 *
 *  @see        DataElement, DataDescriptor,
 *              DataDescriptor::setFileDescriptor()
 *
 * This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setFilename(), setDatasetLocation(), 
 *    setEndian(), and clone().
 *  - Everything else documented in Subject.
 */
class FileDescriptor : public Subject, public Serializable
{
public:
   /**
    *  Emitted when the filename changes with
    *  boost::any<\link Filename\endlink*> containing a pointer to the new
    *  filename.
    *
    *  @see     setFilename()
    */
   SIGNAL_METHOD(FileDescriptor, FilenameChanged)

   /**
    *  Emitted when the data set location changes with boost::any<std::string>
    *  containing the new data set location.
    *
    *  @see     setDatasetLocation()
    */
   SIGNAL_METHOD(FileDescriptor, DatasetLocationChanged)

   /**
    *  Emitted when the endian changes with
    *  boost::any<\link ::EndianType EndianType\endlink> containing the new
    *  endian value.
    *
    *  @see     setEndian()
    */
   SIGNAL_METHOD(FileDescriptor, EndianChanged)

   /**
    *  Sets the location on disk for the file data.
    *
    *  @param   filename
    *           The location on disk of the file data.
    *
    *  @notify  This method notifies signalFilenameChanged() when the filename
    *           is successfully changed.
    *
    *  @see     setFilename(const Filename&)
    */
   virtual void setFilename(const std::string& filename) = 0;

   /**
    *  Sets the location on disk for the file data.
    *
    *  @param   filename
    *           The Filename object containing the location on disk of the file
    *           data.
    *
    *  @notify  This method notifies signalFilenameChanged() when the filename
    *           is successfully changed.
    *
    *  @see     setFilename(const std::string&)
    */
   virtual void setFilename(const Filename& filename) = 0;

   /**
    *  Returns the location on disk for the file data.
    *
    *  @return  The Filename object containing the location on disk of the file
    *           data.
    */
   virtual const Filename& getFilename() const = 0;

   /**
    *  Sets the location of the data set within the file.
    *
    *  The data set location is a string representation of where the data set
    *  resides within the file returned by getFilename().  This is useful when
    *  a single file contains multiple data sets.  The default value is an
    *  empty string.
    *
    *  @param   datasetLocation
    *           The string representation of where the data set resides within
    *           the file on disk.
    *
    *  @notify  This method notifies signalDatasetLocationChanged() when the
    *           data set location is successfully changed.
    *
    *  @see     setFilename(const std::string&)
    */
   virtual void setDatasetLocation(const std::string& datasetLocation) = 0;

   /**
    *  Returns the location of the data set within the file.
    *
    *  @return  The string representation of where the data set resides within
    *           the file on disk.  An empty string is returned if a location
    *           has not been set, which may indicate that the file only
    *           contains one data set.
    */
   virtual const std::string& getDatasetLocation() const = 0;

   /**
    *  Sets the endian format of the data on disk.
    *
    *  This method sets the endian format of how the data is stored on disk.
    *  An importer uses this value to determine whether to swap bytes when
    *  importing the data.
    *
    *  @param   endian
    *           The endian format of the data on disk.
    *
    *  @notify  This method notifies signalEndianChanged() when the endian value
    *           is successfully changed.
    */
   virtual void setEndian(EndianType endian) = 0;

   /**
    *  Returns the endian format of the data on disk.
    *
    *  @return  The endian format of the data on disk.
    */
   virtual EndianType getEndian() const = 0;

   /**
    *  Creates a new file descriptor based on this file descriptor.
    *
    *  This method creates a new file descriptor and sets the values of the
    *  created descriptor to this object's values.
    *
    *  @return  The new file descriptor containing values identical to the
    *           values in this file descriptor.
    *
    *  @see     clone()
    */
   virtual FileDescriptor* copy() const = 0;

   /**
    *  Sets all values in this file descriptor to those of another file
    *  descriptor.
    *
    *  @param   pFileDescriptor
    *           The file descriptor from which to set all data values in this
    *           file descriptor.  No signal/slot attachments currently defined
    *           in \em pFileDescriptor are set into this descriptor.  This
    *           method does nothing and returns \c false if \c NULL is passed
    *           in.
    *
    *  @return  Returns \c true if all values in this file descriptor were
    *           successfully set to the values in the given file descriptor;
    *           otherwise returns \c false.
    *
    *  @notify  This method notifies one or more signals defined in this class
    *           or its subclasses based on data values that are actually
    *           changed.
    *
    *  @see     copy()
    */
   virtual bool clone(const FileDescriptor* pFileDescriptor) = 0;

   /**
    *  Adds the current values in this file descriptor to a given message.
    *
    *  This convenience method adds the values in this file descriptor as
    *  properties to the given message in a message log.
    *
    *  @param   pMessage
    *           The message in the message log for which to add this file
    *           descriptor's values.
    */
   virtual void addToMessageLog(Message* pMessage) const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~FileDescriptor() {}
};

#endif
