/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANYDATA_H
#define ANYDATA_H

class SessionItemDeserializer;
class SessionItemSerializer;

/**
 *  Container for custom data stored in an Any element.
 *
 *  This class serves as a common base class for custom data classes to be set
 *  into an Any element.  The constructor is protected since this base class
 *  does not contain any actual data and should not be instantiated.
 *
 *  @see AnyDataExt1
 */
class AnyData
{
public:
   /**
    *  Destroys the custom data object.
    *
    *  The destructor is called by the Any element when the element is
    *  destroyed.  Subclasses should perform any necessary cleanup in their own
    *  destructor.
    */
   virtual ~AnyData()
   {
   }

   /**
    *  Creates a copy of the custom data.
    *
    *  This method is called when the Any::copy() method is called to copy the
    *  data element.  The default implementation of this method returns \b NULL
    *  so subclasses should override this method to create a new instance of
    *  the object and copy the actual data.  Each subclass can define whether
    *  it performs a deep copy or a shallow copy.
    *
    *  @return  A pointer to a new data object containing a copy of this data.
    *           The default implementation returns \b NULL.  If \b NULL is
    *           returned, the Any::copy() method will also return \b NULL.
    */
   virtual AnyData* copy() const
   {
      return 0;
   }

protected:
   /**
    *  Creates the custom data object.
    *
    *  The constructor is protected since this base class does not contain any
    *  actual data and should not be instantiated.
    */
   AnyData()
   {
   }
};

/**
 *  Extension interface for AnyData session serialization.
 *
 *  This class provides additional capability for the AnyData interface class.
 *  A pointer to this class can be obtained by performing a dynamic cast on a
 *  pointer to AnyData.
 *
 *  @warning A pointer to this class can only be used to call methods contained
 *           in this extension class and cannot be used to call any methods in AnyData.
 */
class AnyDataExt1
{
public:
   /**
    *  Serialize the AnyData during save session.
    *
    *  This is equivalent to SessionItem::serialize() for AnyData objects.
    *
    *  @param serializer
    *         The object to use to save the item as part of the current session.
    *
    *  @return  True if the item was successfully saved and false otherwise.
    *
    *  @see SessionItem::serialize()
    */
   virtual bool serialize(SessionItemSerializer &serializer) const = 0;

   /**
    *  Serialize the AnyData during restore session.
    *
    *  This is equivalent to SessionItem::deserialize() for AnyData objects.
    *
    *  @param deserializer
    *         The object to use to restore the item from a saved session.
    *
    *  @return  True if the item was successfully restored and false otherwise.
    *
    *  @see SessionItem::deserialize()
    */
   virtual bool deserialize(SessionItemDeserializer &deserializer) = 0;

   /**
    * @copydoc AnyData::~AnyData()
    */
   virtual ~AnyDataExt1() {}
};

#endif
