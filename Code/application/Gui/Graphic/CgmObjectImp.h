/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CGMOBJECTIMP_H
#define CGMOBJECTIMP_H

#include "EnumWrapper.h"
#include "GraphicObject.h"
#include "GraphicGroupImp.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class GraphicLayer;

class CgmObjectImp : public GraphicGroupImp
{
public:
   CgmObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   ~CgmObjectImp();

   void setMetafileName(const std::string& metafileName);
   const std::string& getMetafileName() const;
   void setPictureName(const std::string& metafileName);
   const std::string& getPictureName() const;
   void setMetafileDescription(const std::string& metafileDescription);
   const std::string& getMetafileDescription() const;
   void setFontList(const std::vector<std::string>& fontList);
   void getFontList(std::vector<std::string>& fontList) const;

   short getVersion() const;
   short getColorSelectionMode() const;
   short getEdgeWidthMode() const;
   short getLineWidthMode() const;

   void destroy();

   virtual bool replicateObject(const GraphicObject* pObject);
   CgmObject* convertToCgm();

   virtual short* toCgm(int& lBytes);
   virtual int fromCgm(short* pData);

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);
   virtual bool serializeCgm(const std::string& fileName);
   virtual bool deserializeCgm(const std::string& fileName);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

protected:
   short* makeCgm(int& lBytes, bool bCountOnly);
   short readElement(short* pData, int& lByteIndex);
   std::string readStringElement(short* pData, int& lByteIndex);
   bool writeElement(short sElement, short* pData, int& lByteIndex, bool bCountOnly);
   bool writeStringElement(const char* pText, short* pData, int& lByteIndex, bool bCountOnly);
   short getElementClass(short sElement);
   short getElementID(short sElement);
   short getElementParameterLength(short sElement);

   bool serializeCgmProperties(const GraphicObject* pObject, short* pData, int& lByteIndex, bool bCountOnly);
   bool serializeCgmObject(const GraphicObject* pObject, short* pData, int& lByteIndex, bool bCountOnly);

private:
   CgmObjectImp(const CgmObjectImp& rhs);
   enum MetafileConstantEnum
   {
      // Delimiters
      BEGIN_METAFILE = 0x003f,
      BEGIN_PICTURE = 0x007f,
      BEGIN_PICTURE_BODY = 0x0080,
      END_PICTURE = 0x00a0,
      END_METAFILE = 0x0040,

      // Metafile descriptors
      METAFILE_VERSION = 0x1022,
      METAFILE_ELEMENT_LIST = 0x1166,
      METAFILE_DESCRIPTION = 0x105f,
      FONT_LIST = 0x11bf,

      // Picture descriptors
      COLOR_SELECTION_MODE = 0x2042,
      EDGE_WIDTH_SPECIFICATION_MODE = 0x20a2,
      LINE_WIDTH_SPECIFICATION_MODE = 0x2062,
      VDC_EXTENT = 0x20c8,

      // Control elements
      AUXILLIARY_COLOR = 0x3063,
      TRANSPARENCY = 0x3082,

      // Attributes
      TEXT_COLOR = 0x51c3,
      CHARACTER_HEIGHT = 0x51e2,
      TEXT_FONT_INDEX = 0x5142,
      CHARACTER_ORIENTATION = 0x5208,
      FILL_COLOR = 0x52e3,
      INTERIOR_STYLE = 0x52c2,
      HATCH_INDEX = 0x5302,
      EDGE_VISIBILITY = 0x53c2,
      EDGE_WIDTH = 0x5382,
      EDGE_TYPE = 0x5362,
      EDGE_COLOR = 0x53a3,
      LINE_WIDTH = 0x5062,
      LINE_TYPE = 0x5042,
      LINE_COLOR = 0x5083,

      // Graphical primitives
      TEXT = 0x409f,
      POLYGON = 0x40ff,
      POLYGON_SET = 0x411f,
      ELLIPSE = 0x422c,
      POLYLINE = 0x403f,
      ELLIPTICAL_ARC = 0x4254,
      ELLIPTICAL_ARC_CLOSE = 0x4276,
      RECTANGLE = 0x4168,
      CIRCLE = 0x4186,
      CIRCULAR_ARC_CENTER = 0x41ee,
      CIRCULAR_ARC_CENTER_CLOSE = 0x4210
   };

   /**
    * @EnumWrapper CgmObjectImp::MetafileConstantEnum.
    */
   typedef EnumWrapper<MetafileConstantEnum> MetafileConstant;

   std::string mMetafileName;
   std::string mPictureName;
   short msVersion;
   std::string mDescription;
   std::vector<short> mElementList;
   std::vector<std::string> mFontList;
   short msColorSelectionMode;
   short msEdgeWidthMode;
   short msLineWidthMode;

   short* mpCgm;
};

#define CGMOBJECTADAPTEREXTENSION_CLASSES \
   GRAPHICGROUPADAPTEREXTENSION_CLASSES

#define CGMOBJECTADAPTER_METHODS(impClass) \
   GRAPHICGROUPADAPTER_METHODS(impClass) \
   virtual short* toCgm(int& lBytes) \
   { \
      return impClass::toCgm(lBytes); \
   } \
   int fromCgm(short* pData) \
   { \
      return impClass::fromCgm(pData); \
   } \
   bool serializeCgm(const std::string& fileName) \
   { \
      return impClass::serializeCgm(fileName); \
   } \
   bool deserializeCgm(const std::string& fileName) \
   { \
      return impClass::deserializeCgm(fileName); \
   } \
   void setMetafileName(const std::string& metafileName) \
   { \
      return impClass::setMetafileName(metafileName); \
   } \
   const std::string& getMetafileName() const \
   { \
      return impClass::getMetafileName(); \
   } \
   void setPictureName(const std::string& metafileName) \
   { \
      return impClass::setPictureName(metafileName); \
   } \
   const std::string& getPictureName() const \
   { \
      return impClass::getPictureName(); \
   } \
   void setMetafileDescription(const std::string& metafileDescription) \
   { \
      return impClass::setMetafileDescription(metafileDescription); \
   } \
   const std::string& getMetafileDescription() const \
   { \
      return impClass::getMetafileDescription(); \
   } \
   void setFontList(const std::vector<std::string>& fontList) \
   { \
      return impClass::setFontList(fontList); \
   } \
   void getFontList(std::vector<std::string>& fontList) const \
   { \
      return impClass::getFontList(fontList); \
   } \
   short getVersion() const \
   { \
      return impClass::getVersion(); \
   } \
   short getColorSelectionMode() const \
   { \
      return impClass::getColorSelectionMode(); \
   } \
   short getEdgeWidthMode() const \
   { \
      return impClass::getEdgeWidthMode(); \
   } \
   short getLineWidthMode() const \
   { \
      return impClass::getLineWidthMode(); \
   } \
   virtual void destroy() \
   { \
      return impClass::destroy(); \
   }

#endif   // CGMOBJECTIMP_H
