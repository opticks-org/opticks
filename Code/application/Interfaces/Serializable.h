/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include "XercesIncludes.h"

using XERCES_CPP_NAMESPACE_QUALIFIER DOMNode;
class XMLWriter;

/**
 *  Converts data into formats that can be read from and saved to disk.
 *
 *  Objects that can save/restore themselves from a file inherit this
 *  interface. Each class that ultimately derives from Serializable 
 *  must implement an override of these interfaces, even if an intermediate
 *  parent already has an an implementation.
 *
 *  The structure of the serialized output generally has the form of a
 *  string with the class name, followed by the data comprising the
 *  object plus any data in the object's inherited parent(s).
 */
class Serializable
{
public:
   /**
    *  Converts the contents of this object to XML data.
    *
    *  @param   pXml
    *           Pointer to an XMLWriter object in which the object's contents
    *           are written.
    *
    *  @throw   XmlBase::XmlException
    *           This exception (or a subclass) is thrown if there is a problem
    *           serializing the object.
    *
    *  @return  Returns true if the object was successfully converted to XML
    *           data.  Returns false if the object cannot be represented in
    *           XML format.
    */
   virtual bool toXml(XMLWriter* pXml) const = 0;

   /**
    *  Sets the contents of this object from given XML data.
    *
    *  @param   pDocument
    *           The Xerces DOM node.
    *
    *  @param   version
    *           This is the version of the XML which is being deserialized.
    *
    *  @throw   XmlBase::XmlException
    *           This exception (or a subclass) is thrown if there is a problem
    *           deserializing the object.
    *
    *  @return  Returns true if the object's data values were successfully
    *           set from the given XML data.  Returns false if the object
    *           cannot be represented in XML format.
    */
   virtual bool fromXml(DOMNode* pDocument, unsigned int version) = 0;

protected:
   /**
    * This should not be deleted directly.  It should be deleted according to
    * the instructions provided for the relevant subclass.
    */
   virtual ~Serializable() {}
};

#endif
