/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICOBJECTIMP_H
#define GRAPHICOBJECTIMP_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>

#include "AttachmentPtr.h"
#include "BitMaskImp.h"
#include "EnumWrapper.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "GraphicProperty.h"
#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "xmlreader.h"

#include <string>
#include <vector>
#include <sstream>

// Minimum size for use when using ratios based on handle differences that could 
// otherwise be truncated
#define HANDLE_DELTA_MINIMUM 1e-10

// Size at which to take action to prevent ratio problems.  Smaller than
// HANDLE_DELTA_MINIMUM to ensure that rounding errors do not
// result false positives
#define HANDLE_DELTA_THRESHOLD (0.9 * HANDLE_DELTA_MINIMUM)

class GraphicElement;
class GraphicLayer;
class RasterElement;

//                0            1              2             3             4          5           6         7
enum HandleTypeEnum { BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT, MIDDLE_RIGHT, TOP_RIGHT, TOP_CENTER, TOP_LEFT, MIDDLE_LEFT };

/**
 * @EnumWrapper ::HandleTypeEnum.
 */
typedef EnumWrapper<HandleTypeEnum> HandleType;

Q_DECLARE_METATYPE(GraphicObject*)

class GraphicObjectImp : public QObject, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   GraphicObjectImp(const std::string& id, GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord);
   virtual ~GraphicObjectImp();

   virtual void draw (double zoomFactor) const = 0;
   virtual void drawLabel() const;
   virtual bool replicateObject(const GraphicObject* pObject);
   virtual CgmObject* convertToCgm();

   // Properties
   GraphicProperty* createProperty(const std::string& propertyName);
   GraphicProperty* addProperty(const std::string& propertyName);
   bool addProperty(GraphicProperty* pProperty);
   virtual bool hasProperty(const std::string& name) const;
   virtual bool setProperty(const GraphicProperty* pProp);
   virtual GraphicProperty* getProperty(const std::string &name) const;
   void setProperties(const std::vector<GraphicProperty*>& properties);
   virtual const std::vector<GraphicProperty*>& getProperties() const { return mProperties; }
   virtual void updateGeo();
   virtual void enableGeo();

   // Handles
   void setHandles(const std::vector<LocationType>& handles) { mHandles = handles; }
   virtual const std::vector<LocationType>& getHandles() const;
   virtual LocationType getHandle(int iHandle) const;
   virtual void moveHandle(int handle, LocationType pixel, bool bMaintainAspect = false);
   virtual void rotateHandle(int handle, LocationType pixel);
   virtual void updateHandles();

   // Position
   virtual void move(LocationType delta);
   void rotateViewMatrix() const;
   virtual bool hit(LocationType pixelCoord) const = 0;
   virtual const BitMask* getPixels();
   virtual const BitMask* getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow);

   // Object properties
   bool setBoundingBox(LocationType llCorner, LocationType urCorner);
   LocationType getLlCorner() const;
   LocationType getUrCorner() const;
   void getRotatedBoundingBox(LocationType& llCorner, LocationType& urCorner) const;
   bool setRotation(double dAngle);
   double getRotation() const;
   void setName(const std::string &newName);

   // Line
   bool setLineState(bool bLine);
   bool getLineState(bool* pSuccess = NULL) const;
   bool setLineColor(ColorType lineColor);
   ColorType getLineColor() const;
   bool setLineWidth(double dWidth);
   double getLineWidth() const;
   bool setLineStyle(LineStyle eLine);
   LineStyle getLineStyle() const;

   // Arc
   bool setArcRegion(ArcRegion eRegion);
   ArcRegion getArcRegion() const;
   bool setAngles(double dStart, double dStop);
   bool setStartAngle(double dStart);
   bool setStopAngle(double dStop);
   double getStartAngle() const;
   double getStopAngle() const;

   // Filled object
   virtual bool setFillState(bool bFill);
   virtual bool getFillState(bool* pSuccess = NULL) const;
   bool setFillColor(ColorType fillColor);
   ColorType getFillColor() const;
   bool setFillStyle(FillStyle eFill);
   FillStyle getFillStyle() const;
   bool setHatchStyle(SymbolType eHatch);
   SymbolType getHatchStyle() const;

   // Triangle
   bool setApex(double dApex);
   double getApex() const;

   // Text
   bool setText(const std::string& objectText);
   std::string getText() const;
   virtual bool setFont(const QFont& textFont);
   QFont getFont() const;
   bool setTextColor(ColorType textColor);
   ColorType getTextColor() const;
   bool setTextAlignment(int iAlignment);
   int getTextAlignment() const;

   // Scale
   bool setScale(double dScale);
   double getScale() const;

   // Image
   bool setImageFile(const char *pFilename);
   const char* getImageFile() const;
   virtual bool setObjectImage(const unsigned int *pData, int width, int height, ColorType transparent = ColorType(-1,-1,-1)) { return false; }
   virtual const unsigned int* getObjectImage(int &width, int &height, ColorType &transparent) const { return NULL; }
   double getAlpha() const;
   bool setAlpha(double alpha);

   // View
   bool setObjectView(View* pView);
   View* getObjectView() const;

   // Polyline / polygon
   virtual bool addVertices(const std::vector<LocationType>& vertices);
   virtual bool addGeoVertices(const std::vector<LocationType>& geoVertices);
   virtual bool newPath();
   bool setLineScaled(bool scaled);
   bool getLineScaled() const;
   

   // Lat/Lon
   virtual bool setLatLon(LatLonPoint latLonPoint);
   virtual LatLonPoint getLatLon() const;

   // symbols
   virtual bool setPixelSymbol(SymbolType symbol);
   virtual SymbolType getPixelSymbol() const;
   virtual bool setSymbolName(const std::string &symbolName);
   virtual const std::string &getSymbolName() const;
   virtual bool setSymbolSize(unsigned int symbolSize);
   virtual unsigned int getSymbolSize() const;

   // AOI types
   virtual bool setDrawMode(ModeType mode);
   virtual ModeType getDrawMode() const;

   // File I/O
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   virtual GraphicObjectType getGraphicObjectType() const;

   // Used for initial object creation
   /**
    * Return true if the object used the event.
    */
   virtual bool processMousePress(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   /**
    * Return true if the object used the event.
    */
   virtual bool processMouseMove(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);
   /**
    * Return true if the object used the event.
    */
   virtual bool processMouseRelease(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers);

   /**
    * Return true if the object used the event.
    */
   virtual bool processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button,
                                        Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   /**
    * Override to allow the user to edit the object.  Return true if the user was able
    * to edit the object regardless of whether the object properties were modified.
    */
   virtual bool edit();

   /**
    * Override to return false if this is not a visible object (ie, it
    * is a tool, not a shape).
    */
   virtual bool isVisible() const;

   /**
    * Override to return false if the object does not have handles along the bounding box
    */
   virtual bool hasCornerHandles() const;

   /**
    * Override to return false if the object insertion does not require an undo group
    */
   virtual bool insertionUndoable() const;

   virtual GraphicLayer* getLayer() const;
   virtual void setLayer(GraphicLayer *pLayer);

   virtual void temporaryGlContextChange() {}

   virtual bool canRename() const;

signals:
   void propertyModified(GraphicProperty* pProperty);
   void modified();
   void nameChanged(const QString& strName);


protected slots:
   void subjectModified();

protected:
   GraphicElement *getElement() const;

   const RasterElement *getGeoreferenceElement() const;

protected:
   std::vector<GraphicProperty*> mProperties;
   std::vector<LocationType> mHandles;
   BitMaskImp mPixelMask;

   LocationType getLabelPosition() const;
   LocationType getPixelSize() const;

protected slots:
   void setCacheDirty();

protected:
   mutable bool mDisplayListDirty;
   mutable bool mBitMaskDirty;
   AttachmentPtr<GraphicLayer> mpLayer;

private:
   void adjustHandles(int handle, LocationType point, bool bMaintainAspect);

   GraphicObjectType mType;
};

#define GRAPHICOBJECTADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES \
   , public GraphicObjectExt1

#define GRAPHICOBJECTADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   GraphicObjectType getGraphicObjectType() const \
   { \
      return impClass::getGraphicObjectType(); \
   } \
   void setName(const std::string& name) \
   { \
      return impClass::setName(name); \
   } \
   CgmObject* convertToCgm() \
   { \
      return impClass::convertToCgm(); \
   } \
   const BitMask* getPixels() \
   { \
      return impClass::getPixels(); \
   } \
   bool setBoundingBox(LocationType llCorner, LocationType urCorner) \
   { \
      return impClass::setBoundingBox(llCorner, urCorner); \
   } \
   LocationType getLlCorner() const \
   { \
      return impClass::getLlCorner(); \
   } \
   LocationType getUrCorner() const \
   { \
      return impClass::getUrCorner(); \
   } \
   bool setRotation(double dAngle) \
   { \
      return impClass::setRotation(dAngle); \
   } \
   double getRotation() const \
   { \
      return impClass::getRotation(); \
   } \
   bool setLineState(bool bLine) \
   { \
      return impClass::setLineState(bLine); \
   } \
   bool getLineState(bool* pSuccess = NULL) const \
   { \
      return impClass::getLineState(pSuccess); \
   } \
   bool setLineColor(ColorType lineColor) \
   { \
      return impClass::setLineColor(lineColor); \
   } \
   ColorType getLineColor() const \
   { \
      return impClass::getLineColor(); \
   } \
   bool setLineWidth(double dWidth) \
   { \
      return impClass::setLineWidth(dWidth); \
   } \
   double getLineWidth() const \
   { \
      return impClass::getLineWidth(); \
   } \
   bool setLineStyle(LineStyle eLine) \
   { \
      return impClass::setLineStyle(eLine); \
   } \
   LineStyle getLineStyle() const \
   { \
      return impClass::getLineStyle(); \
   } \
   bool setArcRegion(ArcRegion eRegion) \
   { \
      return impClass::setArcRegion(eRegion); \
   } \
   ArcRegion getArcRegion() const \
   { \
      return impClass::getArcRegion(); \
   } \
   bool setAngles(double dStart, double dStop) \
   { \
      return impClass::setAngles(dStart, dStop); \
   } \
   bool setStartAngle(double dStart) \
   { \
      return impClass::setStartAngle(dStart); \
   } \
   bool setStopAngle(double dStop) \
   { \
      return impClass::setStopAngle(dStop); \
   } \
   double getStartAngle() const \
   { \
      return impClass::getStartAngle(); \
   } \
   double getStopAngle() const \
   { \
      return impClass::getStopAngle(); \
   } \
   bool setFillState(bool bFill) \
   { \
      return impClass::setFillState(bFill); \
   } \
   bool getFillState(bool* pSuccess = NULL) const \
   { \
      return impClass::getFillState(pSuccess); \
   } \
   bool setFillColor(ColorType fillColor) \
   { \
      return impClass::setFillColor(fillColor); \
   } \
   ColorType getFillColor() const \
   { \
      return impClass::getFillColor(); \
   } \
   bool setFillStyle(FillStyle eFill) \
   { \
      return impClass::setFillStyle(eFill); \
   } \
   FillStyle getFillStyle() const \
   { \
      return impClass::getFillStyle(); \
   } \
   bool setHatchStyle(SymbolType eHatch) \
   { \
      return impClass::setHatchStyle(eHatch); \
   } \
   SymbolType getHatchStyle() const \
   { \
      return impClass::getHatchStyle(); \
   } \
   bool setApex(double dApex) \
   { \
      return impClass::setApex(dApex); \
   } \
   double getApex() const \
   { \
      return impClass::getApex(); \
   } \
   bool setText(const std::string& objectText) \
   { \
      return impClass::setText(objectText); \
   } \
   std::string getText() const \
   { \
      return impClass::getText(); \
   } \
   bool setFont(const Font* pTextFont) \
   { \
      if (pTextFont == NULL) \
      { \
         return false; \
      } \
      return impClass::setFont(pTextFont->getQFont()); \
   } \
   const Font* getFont() const \
   { \
      FontProperty* pFont = dynamic_cast<FontProperty*>(impClass::getProperty("Font")); \
      if (pFont != NULL) \
      { \
         return &(pFont->getFont()); \
      } \
      return NULL; \
   } \
   bool setTextColor(ColorType textColor) \
   { \
      return impClass::setTextColor(textColor); \
   } \
   ColorType getTextColor() const \
   { \
      return impClass::getTextColor(); \
   } \
   bool setTextAlignment(int iAlignment) \
   { \
      return impClass::setTextAlignment(iAlignment); \
   } \
   int getTextAlignment() const \
   { \
      return impClass::getTextAlignment(); \
   } \
   bool setScale(double dScale) \
   { \
      return impClass::setScale(dScale); \
   } \
   double getScale() const \
   { \
      return impClass::getScale(); \
   } \
   bool setImageFile(const char *pFilename) \
   { \
      return impClass::setImageFile(pFilename); \
   } \
   bool setObjectImage(const unsigned int* pData, int width, int height, \
      ColorType transparent = ColorType()) \
   { \
      return impClass::setObjectImage(pData, width, height, transparent); \
   } \
   const unsigned int* getObjectImage(int &width, \
      int &height, ColorType &transparent) const \
   { \
      return impClass::getObjectImage(width, height, transparent); \
   } \
   double getAlpha() const \
   { \
      return impClass::getAlpha(); \
   } \
   bool setAlpha(double alpha) \
   { \
      return impClass::setAlpha(alpha); \
   } \
   bool setObjectView(View* pView) \
   { \
      return impClass::setObjectView(pView); \
   } \
   View* getObjectView() const \
   { \
      return impClass::getObjectView(); \
   } \
   bool addVertices(const std::vector<LocationType>& vertices) \
   { \
      return impClass::addVertices(vertices); \
   } \
   bool addGeoVertices(const std::vector<LocationType>& geoVertices) \
   { \
      return impClass::addGeoVertices(geoVertices); \
   } \
   bool newPath() \
   { \
      return impClass::newPath(); \
   } \
   bool isVisible() const \
   { \
      return impClass::isVisible(); \
   } \
   bool setPixelSymbol(SymbolType symbol) \
   { \
      return impClass::setPixelSymbol(symbol); \
   } \
   SymbolType getPixelSymbol() const \
   { \
      return impClass::getPixelSymbol(); \
   } \
   bool setSymbolName(const std::string &symbolName) \
   { \
      return impClass::setSymbolName(symbolName); \
   } \
   const std::string &getSymbolName() const \
   { \
      return impClass::getSymbolName(); \
   } \
   bool setSymbolSize(unsigned int symbolSize) \
   { \
      return impClass::setSymbolSize(symbolSize); \
   } \
   unsigned int getSymbolSize() const \
   { \
      return impClass::getSymbolSize(); \
   } \
   bool setDrawMode(ModeType mode) \
   { \
      return impClass::setDrawMode(mode); \
   } \
   ModeType getDrawMode() const \
   { \
      return impClass::getDrawMode(); \
   } \
   bool setLineScaled(bool scaled) \
   { \
      return impClass::setLineScaled(scaled); \
   } \
   bool getLineScaled() const \
   { \
      return impClass::getLineScaled(); \
   } \
   GraphicLayer* getLayer() const \
   { \
      return impClass::getLayer(); \
   }

#endif
