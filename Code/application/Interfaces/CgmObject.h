/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CGMOBJECT_H
#define CGMOBJECT_H

#include "GraphicGroup.h"

/**
 *  Provides access to CGM data for an graphic object.
 *
 *  %Any graphic object can be converted to a CGM format using the CgmObject
 *  class.  A CgmObject is obtained using the GraphicObject::convertToCgm()
 *  method.  The class provides access functions to the CGM metadata as well as
 *  conversion methods to read and write the object in CGM binary format.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - All notifications documented in GraphicGroup.
 *
 *  @see    GraphicObject, GraphicObject::convertToCgm()
 */
class CgmObject : public GraphicGroup
{
public:
   /**
    *  Converts the graphic object to CGM binary format.
    *
    *  @param   lBytes
    *           Populated with the total length in bytes of the CGM data.
    *
    *  @return  A pointer to the CGM data for the graphic object.  NULL
    *           is returned if an error occurs.
    *
    *  @see     fromCgm()
    */
   virtual short* toCgm(int& lBytes) = 0;

   /**
    *  Modifies the properties of the graphic object to that contained
    *  in the given CGM binary data.
    *
    *  @param   pData
    *           A pointer to the CGM data to use to convert the graphic
    *           object.
    *
    *  @return  int
    *           The total number of bytes read of the CGM data.  A value
    *           of -1 is returned if an error occurs.
    *
    *  @see     toCgm()
    */
   virtual int fromCgm(short* pData) = 0;

   /**
    *  Save a CGM to a file in binary format.
    *
    *  @param   fileName
    *           The name of the file to serialize the data to.
    *
    *  @see     deserializeCgm()
    */
   virtual bool serializeCgm(const std::string& fileName) = 0;

   /**
    *  Load a CGM from a file in binary format.
    *
    *  @param   fileName
    *           The name of the file to deserialize the data from.
    *
    *  @see     serializeCgm()
    */
   virtual bool deserializeCgm(const std::string& fileName) = 0;

   /**
    *  Sets the Begin Metafile element name.
    *
    *  @param   metafileName
    *           The new name for the element.
    */
   virtual void setMetafileName(const std::string& metafileName) = 0;

   /**
    *  Returns the Begin Metafile element name.
    *
    *  @return  The element name.
    */
   virtual const std::string& getMetafileName() const = 0;

   /**
    *  Sets the Begin Picture element name.
    *
    *  @param   metafileName
    *           The new name for the element.
    */
   virtual void setPictureName(const std::string& metafileName) = 0;

   /**
    *  Returns the Begin Picture element name.
    *
    *  @return  The element name.
    */
   virtual const std::string& getPictureName() const = 0;

   /**
    *  Sets the Metafile %Description element value.
    *
    *  @param   metafileDescription
    *           The new description.
    */
   virtual void setMetafileDescription(const std::string& metafileDescription) = 0;

   /**
    *  Returns the Metafile %Description element value.
    *
    *  @return  The metafile description.
    */
   virtual const std::string& getMetafileDescription() const = 0;

   /**
    *  Sets the font names for the Font List element.
    *
    *  @param   fontList
    *           The font names.
    */
   virtual void setFontList(const std::vector<std::string>& fontList) = 0;

   /**
    *  Returns the font names for the Font List element.
    *
    *  @param   fontList
    *           Populated with the font names from the font list.
    */
   virtual void getFontList(std::vector<std::string>& fontList) const = 0;

   /**
    *  Returns the Metafile Version element value.
    *
    *  @return  The metafile version.
    */
   virtual short getVersion() const = 0;

   /**
    *  Returns the Color Selection Mode element value.
    *
    *  @return  The color selection mode.
    */
   virtual short getColorSelectionMode() const = 0;

   /**
    *  Returns the Edge Width Specification Mode element value.
    *
    *  @return  The edge width specification mode.
    */
   virtual short getEdgeWidthMode() const = 0;

   /**
    *  Returns the Line Width Specification Mode element value.
    *
    *  @return  The line width specification mode.
    */
   virtual short getLineWidthMode() const = 0;

   /**
    *  Destroys the CGM graphic object.
    *
    *  The CGM object becomes invalid after this method is called, so
    *  pointers to the object should be reset to NULL.
    */
   virtual void destroy() = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~CgmObject() {}
};

#endif
