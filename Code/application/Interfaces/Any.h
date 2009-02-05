/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANY_H
#define ANY_H

#include "DataElement.h"

#include <string>

class AnyData;

/**
 *  Provides custom data storage.
 *
 *  The %Any element provides storage for custom data types that are not
 *  inherently recognized by the data model.  The element provides the means to
 *  set and get the custom data stored in the element via the AnyData class
 *  which serves as a common base class for all custom data objects.
 *
 *  The application provides one concrete subclass, DataVariantAnyData, that stores a
 *  single DataVariant value.  If the plug-in module creating the
 *  DataVariantAnyData object will remain loaded for the lifetime of the %Any
 *  element, the plug-in can create the object directly on the heap.  If the
 *  plug-in module will be unloaded before the %Any element is destroyed, the
 *  plug-in must create the DataVariantAnyData object from the ObjectFactory,
 *  which ensures that the %Any element containing the object will be able to
 *  successfully delete it when the element is destroyed.
 *
 *  A plug-in can also create a subclass of AnyData to store custom data that
 *  cannot be set into a DataVariant, but the module containing that plug-in
 *  must remain loaded so that the %Any element will be able to successfully
 *  delete the custom data when the element is destroyed.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The setData() method is called.
 *  - Everything else documented in DataElement.
 *
 *  @see     DataElement
 */
class Any : public DataElement
{
public:
   /**
    *  Sets the custom data managed by this element.
    *
    *  Ownership of the data is transferred to the element, so the calling
    *  object is not responsible for deleting the data.  The element will
    *  delete the data when the element is destroyed.
    *
    *  @param   pData
    *           A pointer to the custom data object.
    *
    *  @notify  This method will notify Subject::signalModified with any<AnyData*>.
    */
   virtual void setData(AnyData* pData) = 0;

   /**
    *  Returns a pointer to the custom data.
    *
    *  The calling object must cast the data to the proper pointer type to
    *  further manipulate it.
    *
    *  @return  A pointer to the stored data.  \b NULL is returned if no custom
    *           data has been set.
    */
   virtual AnyData* getData() = 0;

   /**
    *  Returns read-only access to the custom data.
    *
    *  The calling object must cast the data to the proper pointer type to
    *  further access it.
    *
    *  @return  A pointer to the stored data.  The custom data represented by
    *           the returned pointer should not be modified.  To modify the
    *           values, call the non-const version of getData().  \b NULL is
    *           returned if no custom data has been set.
    */
   virtual const AnyData* getData() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement().
    */
   virtual ~Any() {}
};

#endif
