/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAVARIANTANYDATA_H
#define DATAVARIANTANYDATA_H

#include "AnyData.h"
#include "DataVariant.h"
#include "ObjectResource.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "xmlreader.h"
#include "xmlwriter.h"

/**
 *  A custom data container for an Any element that contains a DataVariant.
 *
 *  This subclass of AnyData is provided as a convenience for data that can be
 *  stored in a DataVariant.
 */
class DataVariantAnyData : public AnyData
{
public:
   /**
    *  Creates the custom data object.
    *
    *  This constructor creates an empty data object with an empty DataVariant.
    *  It can be called directly by plug-ins to create the custom data object
    *  to set into an Any element provided that the plug-in module will remain
    *  loaded for the lifetime of the element.  Plug-ins should create this
    *  object from the ObjectFactory if the plug-in module will be unloaded
    *  before the Any element is destroyed.
    */
   DataVariantAnyData()
   {
   }

   /**
    *  Creates and initializes the custom data object.
    *
    *  This constructor creates a data object and initializes the internal
    *  DataVariant with the given value.  It can be called directly by plug-ins
    *  to create the custom data object to set into an Any element provided
    *  that the plug-in module will remain loaded for the lifetime of the
    *  element.  Plug-ins must create an empty object from the ObjectFactory if
    *  the plug-in module will be unloaded before the Any element is destroyed.
    *
    *  @param   value
    *           The data value with which the internal DataVariant is
    *           initialized.
    *
    *  @see     setAttribute()
    */
   template<typename T>
   DataVariantAnyData(const T& value) :
      mData(value)
   {
   }

   /**
    *  Destroys the custom data object.
    *
    *  The destructor is called by the Any element when the element is
    *  destroyed.  The internal DataVariant is deleted, thereby deleting the
    *  data it contains.
    *
    *  @see     DataVariant::~DataVariant()
    */
   ~DataVariantAnyData()
   {
   }

   /**
    *  Creates a copy of the custom data.
    *
    *  This method is called when the Any::copy() method is called to copy the
    *  data element.  The default implementation of this method creates a new
    *  DataVariantAnyData and sets the new internal DataVariant equal to this
    *  DataVariant.
    *
    *  @return  A pointer to a new DataVariantAnyData object containing a copy
    *           of this data.
    *
    *  @see     DataVariant::operator=(const DataVariant&)
    */
   AnyData* copy() const
   {
      FactoryResource<DataVariantAnyData> pData;
      pData->mData = mData;

      return pData.release();
   }

   /**
    *  Saves the DataVariant as part of a full session save.
    *
    *  @param   serializer
    *           The object to use to save the DataVariant as part of the current
    *           session.
    *
    *  @return  Returns \c true if the DataVariant was successfully saved and
    *           \c false otherwise.
    */
   bool serialize(SessionItemSerializer& serializer) const
   {
      XMLWriter xml("DataVariant");
      if (mData.toXml(&xml) == false)
      {
         return false;
      }

      return serializer.serialize(xml);
   }

   /**
    *  Restores the DataVariant from a saved session.
    *
    *  @param   deserializer
    *           The object to use to restore the DataVariant from a saved
    *           session.
    *
    *  @return  Returns \c true if the DataVariant was successfully restored and
    *           \c false otherwise.
    */
   bool deserialize(SessionItemDeserializer& deserializer)
   {
      XmlReader reader(NULL, false);

      XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pRoot = deserializer.deserialize(reader, "DataVariant");
      if (pRoot == NULL)
      {
         return false;
      }

      return mData.fromXml(pRoot, XmlBase::VERSION);
   }

   /**
    *  Sets the custom data to set into an Any element.
    *
    *  @param   data
    *           The custom data.
    */
   void setAttribute(const DataVariant& data)
   {
      mData = data;
   }

   /**
    *  Returns the custom data to set into an Any element.
    *
    *  @return  A reference to the data variant storing the custom data.  The
    *           data can be modified directly.
    */
   DataVariant& getAttribute()
   {
      return mData;
   }

   /**
    *  Returns read-only access to the custom data to set into an Any element.
    *
    *  @return  A const reference to the data variant storing the custom data.
    *           The data represented by the returned reference should not be
    *           modified.  To modify the values, call the non-const version of
    *           getAttribute().
    */
   const DataVariant& getAttribute() const
   {
      return mData;
   }

private:
   DataVariant mData;
};

#endif
