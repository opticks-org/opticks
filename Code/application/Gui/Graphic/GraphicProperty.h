/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICPROPERTY_H
#define GRAPHICPROPERTY_H

#include "ColorType.h"
#include "FontImp.h"
#include "LocationType.h"
#include "GeoPoint.h"
#include "TypesFile.h"
#include "xmlwriter.h"

#include <string>

class GraphicProperty
{
public:
   GraphicProperty(const std::string& name);
   virtual ~GraphicProperty();

   std::string getName() const;
   virtual bool set(const GraphicProperty* pProp) = 0;
   virtual bool compare(const GraphicProperty* pProp) const = 0;
   virtual GraphicProperty* copy() const = 0;

   virtual bool toXml(XMLWriter* pXml) const = 0;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version) = 0;

protected:
   void setName(const std::string& name);

private:
   std::string mName;
};


class AlphaProperty : public GraphicProperty
{
public:
   AlphaProperty(double alpha);

   double getAlpha() const;
   bool set(const GraphicProperty *pProp) { *this = *(AlphaProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   double mAlpha;
};


class ApexProperty : public GraphicProperty
{
public:
   ApexProperty(double apex);

   double getApex() const;
   bool set(const GraphicProperty *pProp) { *this = *(ApexProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   double mApexPosition;
};


class ArcRegionProperty : public GraphicProperty
{
public:
   ArcRegionProperty(ArcRegion eRegion);

   ArcRegion getRegion() const;
   bool set(const GraphicProperty *pProp) { *this = *(ArcRegionProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   ArcRegion mRegion;
};


class BoundingBoxProperty : public GraphicProperty
{
public:
   BoundingBoxProperty(LocationType llCorner, LocationType urCorner, bool geoCoords = false);
   BoundingBoxProperty(LocationType llCorner, LocationType urCorner, LocationType llLatLong, LocationType urLatLong);

   bool hasGeoCoords() const;
   bool hasPixelCoords() const;
   LocationType getLlCorner() const;
   LocationType getUrCorner() const;
   LocationType getLlLatLong() const;
   LocationType getUrLatLong() const;
   bool set(const GraphicProperty* pProp) { *this = *(BoundingBoxProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   LocationType mLlCorner;
   LocationType mUrCorner;
   LocationType mLlLatLong;
   LocationType mUrLatLong;
   bool mHasGeoCoords;
   bool mHasPixelCoords;
};


class FillColorProperty : public GraphicProperty
{
public:
   FillColorProperty(ColorType color);

   ColorType getColor() const;
   bool set(const GraphicProperty *pProp) { *this = *(FillColorProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   ColorType mColor;
};


class FillStyleProperty : public GraphicProperty
{
public:
   FillStyleProperty(FillStyle eStyle);

   FillStyle getFillStyle() const;
   bool set(const GraphicProperty *pProp) { *this = *(FillStyleProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   FillStyle mStyle;
};


class FontProperty : public GraphicProperty
{
public:
   FontProperty(const FontImp& font);

   const FontImp& getFont() const;
   bool set(const GraphicProperty *pProp) { *this = *(FontProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   FontImp mFont;
};


class HatchStyleProperty : public GraphicProperty
{
public:
   HatchStyleProperty(SymbolType eHatch);

   SymbolType getHatchStyle() const;
   bool set(const GraphicProperty *pProp) { *this = *(HatchStyleProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   SymbolType mStyle;
};


class LatLonProperty : public GraphicProperty
{
public:
   LatLonProperty(LatLonPoint latLonPoint) : GraphicProperty("LatLon"), mLatLon(latLonPoint) { }

   LatLonPoint getLatLon() const { return mLatLon; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   LatLonPoint mLatLon;
};


class LineColorProperty : public GraphicProperty
{
public:
   LineColorProperty(ColorType color);

   ColorType getColor() const;
   bool set(const GraphicProperty *pProp) { *this = *(LineColorProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   ColorType mColor;
};


class LineOnProperty : public GraphicProperty
{
public:
   LineOnProperty(bool state);

   bool getState() const;
   bool set(const GraphicProperty *pProp) { *this = *(LineOnProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   bool mState;
};

class LineScaledProperty : public GraphicProperty
{
public:
   LineScaledProperty(bool scaled);

   bool getScaled() const;
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   bool mScaled;
};

class LineStyleProperty : public GraphicProperty
{
public:
   LineStyleProperty(LineStyle eStyle);

   LineStyle getStyle() const;
   bool set(const GraphicProperty *pProp) { *this = *(LineStyleProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   LineStyle mStyle;
};


class LineWidthProperty : public GraphicProperty
{
public:
   LineWidthProperty(double width);

   double getWidth() const;
   bool set(const GraphicProperty *pProp) { *this = *(LineWidthProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   double mWidth;
};


class RotationProperty : public GraphicProperty
{
public:
   RotationProperty(double rotate);

   double getRotation() const;
   bool set(const GraphicProperty *pProp) { *this = *(RotationProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   double mRotate;
};


class ScaleProperty : public GraphicProperty
{
public:
   ScaleProperty(double scale);

   double getScale() const;
   bool set(const GraphicProperty *pProp) { *this = *(ScaleProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   double mScale;
};


class TextAlignmentProperty : public GraphicProperty
{
public:
   TextAlignmentProperty(int iAlignment);

   int getAlignment() const;
   bool set(const GraphicProperty* pProperty);
   bool compare(const GraphicProperty* pProperty) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   int mAlignment;
};


class TextColorProperty : public GraphicProperty
{
public:
   TextColorProperty(ColorType color);

   ColorType getColor() const;
   bool set(const GraphicProperty *pProp) { *this = *(TextColorProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   ColorType mColor;
};


class TextStringProperty : public GraphicProperty
{
public:
   TextStringProperty(const std::string& text);

   const std::string& getString() const;
   bool set(const GraphicProperty *pProp) { *this = *(TextStringProperty*)pProp; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   std::string mString;
};


class WedgeProperty : public GraphicProperty
{
public:
   WedgeProperty(double dStartAngle, double dStopAngle);

   double getStartAngle() const;
   double getStopAngle() const;
   bool set(const GraphicProperty* pProperty) { *this = *(WedgeProperty*)pProperty; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

protected:
   void normalizeAngles();

private:
   double mStartAngle;
   double mStopAngle;
};


class PaperSizeProperty : public GraphicProperty
{
public:
   PaperSizeProperty(LocationType paperSize);

   LocationType getSize() const;
   bool set(const GraphicProperty* pProperty) { *this = *(PaperSizeProperty*)pProperty; return true; }
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   LocationType mSize;
};


class FileNameProperty : public GraphicProperty
{
public:
   FileNameProperty(const std::string& filename);

   const std::string & getFileName() const { return mFileName; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   std::string mFileName;
};

class PixelSymbolProperty : public GraphicProperty
{
public:
   PixelSymbolProperty(SymbolType symbol);

   SymbolType getPixelSymbol() const { return mSymbol; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   SymbolType mSymbol;
};

class DrawModeProperty : public GraphicProperty
{
public:
   DrawModeProperty(ModeType mode);

   ModeType getDrawMode() const { return mMode; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   ModeType mMode;
};

class GraphicSymbolProperty : public GraphicProperty
{
public:
   GraphicSymbolProperty(const std::string &symbolName);

   const std::string &getSymbolName() const { return mSymbolName; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   std::string mSymbolName;
};

class GraphicSymbolSizeProperty : public GraphicProperty
{
public:
   GraphicSymbolSizeProperty(unsigned int symbolSize);

   const unsigned int getSymbolSize() const { return mSymbolSize; }
   bool set(const GraphicProperty *pProp);
   bool compare(const GraphicProperty* pProp) const;
   GraphicProperty* copy() const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

private:
   unsigned int mSymbolSize;
};

#endif
