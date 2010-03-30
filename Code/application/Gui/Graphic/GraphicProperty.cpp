/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayerImp.h"
#include "GraphicProperty.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "xmlreader.h"

#include <sstream>
#include "XercesIncludes.h"
#include <boost/lexical_cast.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE

/////////////////////
// GraphicProperty //
/////////////////////

GraphicProperty::GraphicProperty(const string& name) :
   mName(name)
{
}

GraphicProperty::~GraphicProperty()
{
}

string GraphicProperty::getName() const
{
   return mName;
}

void GraphicProperty::setName(const string& name)
{
   if (name.empty() == false)
   {
      mName = name;
   }
}

bool GraphicProperty::toXml(XMLWriter* pXml) const
{
   return true;
}

// most properties do not check this return
// value as they want to deserialize as much as
// possible...since this doesn't currently do
// anything, there's little problem
bool GraphicProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   return true;
}

///////////////////
// AlphaProperty //
///////////////////

AlphaProperty::AlphaProperty(double alpha) :
   GraphicProperty("Alpha"),
   mAlpha(alpha)
{
}

bool AlphaProperty::set(const GraphicProperty* pProp)
{
   const AlphaProperty* pAlphaProp = dynamic_cast<const AlphaProperty*>(pProp);
   if (pAlphaProp == NULL)
   {
      return false;
   }

   *this = *pAlphaProp;
   return true;
}

bool AlphaProperty::compare(const GraphicProperty* pProp) const
{
   const AlphaProperty* pAlphaProp = dynamic_cast<const AlphaProperty*>(pProp);
   if (pAlphaProp == NULL)
   {
      return false;
   }

   return mAlpha == pAlphaProp->getAlpha();
}

GraphicProperty* AlphaProperty::copy() const
{
   return new AlphaProperty(mAlpha);
}

double AlphaProperty::getAlpha() const
{
   return mAlpha;
}

bool AlphaProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Alpha"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mAlpha;
   pXml->addAttr("alpha", buf.str());
   pXml->popAddPoint();

   return true;
}

bool AlphaProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream buf(A(elmnt->getAttribute(X("alpha"))));
   buf >> mAlpha;

   return true;
}

//////////////////
// ApexProperty //
//////////////////

ApexProperty::ApexProperty(double apex) :
   GraphicProperty("Apex"),
   mApexPosition(apex)
{
}

bool ApexProperty::set(const GraphicProperty* pProp)
{
   const ApexProperty* pApexProp = dynamic_cast<const ApexProperty*>(pProp);
   if (pApexProp == NULL)
   {
      return false;
   }

   *this = *pApexProp;
   return true;
}

bool ApexProperty::compare(const GraphicProperty* pProp) const
{
   const ApexProperty* pApexProp = dynamic_cast<const ApexProperty*>(pProp);
   if (pApexProp == NULL)
   {
      return false;
   }

   return mApexPosition == pApexProp->getApex();
}

GraphicProperty* ApexProperty::copy() const
{
   return new ApexProperty(mApexPosition);
}

double ApexProperty::getApex() const
{
   return mApexPosition;
}

bool ApexProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Apex"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mApexPosition;
   pXml->addAttr("position", buf.str());
   pXml->popAddPoint();

   return true;
}

bool ApexProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream buf(A(elmnt->getAttribute(X("position"))));
   buf >> mApexPosition;

   return true;
}

///////////////////////
// ArcRegionProperty //
///////////////////////

ArcRegionProperty::ArcRegionProperty(ArcRegion eRegion) :
   GraphicProperty("ArcRegion"),
   mRegion(eRegion)
{
}

bool ArcRegionProperty::set(const GraphicProperty* pProp)
{
   const ArcRegionProperty* pRegionProp = dynamic_cast<const ArcRegionProperty*>(pProp);
   if (pRegionProp == NULL)
   {
      return false;
   }

   *this = *pRegionProp;
   return true;
}

bool ArcRegionProperty::compare(const GraphicProperty* pProp) const
{
   const ArcRegionProperty* pRegionProp = dynamic_cast<const ArcRegionProperty*>(pProp);
   if (pRegionProp == NULL)
   {
      return false;
   }

   return mRegion == pRegionProp->getRegion();
}

GraphicProperty* ArcRegionProperty::copy() const
{
   return new ArcRegionProperty(mRegion);
}

ArcRegion ArcRegionProperty::getRegion() const
{
   return mRegion;
}

bool ArcRegionProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("ArcRegion"));
   GraphicProperty::toXml(pXml);
   switch (mRegion)
   {
      case ARC_CENTER:
         pXml->addAttr("region", "center");
         break;
      case ARC_CHORD:
         pXml->addAttr("region", "chord");
         break;
      case ARC_OPEN:
         pXml->addAttr("region", "open");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool ArcRegionProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string buf(A(elmnt->getAttribute(X("region"))));

   if (buf == "center")
   {
      mRegion = ARC_CENTER;
   }
   else if (buf == "chord")
   {
      mRegion = ARC_CHORD;
   }
   else if (buf == "open")
   {
      mRegion = ARC_OPEN;
   }

   return true;
}

/////////////////////////
// BoundingBoxProperty //
/////////////////////////

BoundingBoxProperty::BoundingBoxProperty(LocationType llCorner, LocationType urCorner, bool geoCoords) :
   GraphicProperty("BoundingBox")
{
   if (geoCoords)
   {
      mLlLatLong = llCorner;
      mUrLatLong = urCorner;
      mHasGeoCoords = true;
   }
   else
   {
      mLlCorner = llCorner;
      mUrCorner = urCorner;
      mHasGeoCoords = false;
   }

   mHasPixelCoords = !mHasGeoCoords;
   mGeoCoordsMatchPixelCoords = false;
}

BoundingBoxProperty::BoundingBoxProperty(LocationType llCorner, LocationType urCorner, LocationType llLatLong,
                                         LocationType urLatLong, bool geoCoordsMatchPixelCoords) :
   GraphicProperty("BoundingBox"),
   mLlCorner(llCorner),
   mUrCorner(urCorner),
   mLlLatLong(llLatLong),
   mUrLatLong(urLatLong),
   mHasGeoCoords(true),
   mHasPixelCoords(true),
   mGeoCoordsMatchPixelCoords(geoCoordsMatchPixelCoords)
{
}

bool BoundingBoxProperty::hasGeoCoords() const
{
   return mHasGeoCoords;
}

bool BoundingBoxProperty::hasPixelCoords() const
{
   return mHasPixelCoords;
}

bool BoundingBoxProperty::geoCoordsMatchPixelCoords() const
{
   return mGeoCoordsMatchPixelCoords;
}

LocationType BoundingBoxProperty::getLlCorner() const
{
   return mLlCorner;
}

LocationType BoundingBoxProperty::getUrCorner() const
{
   return mUrCorner;
}

LocationType BoundingBoxProperty::getLlLatLong() const
{
   return mLlLatLong;
}

LocationType BoundingBoxProperty::getUrLatLong() const
{
   return mUrLatLong;
}

bool BoundingBoxProperty::set(const GraphicProperty* pProp)
{
   const BoundingBoxProperty* pBoxProp = dynamic_cast<const BoundingBoxProperty*>(pProp);
   if (pBoxProp == NULL)
   {
      return false;
   }

   *this = *pBoxProp;
   return true;
}

bool BoundingBoxProperty::compare(const GraphicProperty* pProp) const
{
   const BoundingBoxProperty* pBoxProp = dynamic_cast<const BoundingBoxProperty*>(pProp);
   if (pBoxProp == NULL)
   {
      return false;
   }

   LocationType llCorner = pBoxProp->getLlCorner();
   LocationType urCorner = pBoxProp->getUrCorner();
   LocationType llLatLong = pBoxProp->getLlLatLong();
   LocationType urLatLong = pBoxProp->getUrLatLong();

   bool match = true;
   if (mHasGeoCoords)
   {
      match = match && (mLlLatLong == llLatLong && mUrLatLong == urLatLong);
   }

   if (mHasPixelCoords)
   {
      match = match && (mLlCorner == llCorner && mUrCorner == urCorner);
   }

   return match;
}

GraphicProperty* BoundingBoxProperty::copy() const
{
   BoundingBoxProperty* pProperty = new BoundingBoxProperty(mLlCorner, mUrCorner, mLlLatLong, mUrLatLong,
      mGeoCoordsMatchPixelCoords);
   if (pProperty != NULL)
   {
      pProperty->mHasGeoCoords = mHasGeoCoords;
      pProperty->mHasPixelCoords = mHasPixelCoords;
   }

   return pProperty;
}

bool BoundingBoxProperty::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   pXml->pushAddPoint(pXml->addElement("BoundingBox"));
   GraphicProperty::toXml(pXml);

   if (mHasPixelCoords)
   {
      stringstream buf;
      buf.precision(12);
      buf << mLlCorner.mX << ' ' << mLlCorner.mY << ' ' << mUrCorner.mX << ' ' << mUrCorner.mY;
      pXml->addText(buf.str(), pXml->addElement("boundingBox"));
   }

   if (mHasGeoCoords)
   {
      stringstream buf;
      buf.precision(12);
      buf << mLlLatLong.mX << ' ' << mLlLatLong.mY << ' ' << mUrLatLong.mX << ' ' << mUrLatLong.mY;
      pXml->addText(buf.str(), pXml->addElement("geoBox"));
   }

   pXml->popAddPoint();
   return true;
}

bool BoundingBoxProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   mLlCorner = LocationType();
   mUrCorner = LocationType();
   mLlLatLong = LocationType();
   mUrLatLong = LocationType();
   mHasGeoCoords = false;
   mHasPixelCoords = false;

   GraphicProperty::fromXml(pDocument, version);

   for (DOMNode* chld = pDocument->getFirstChild(); chld != NULL; chld = chld->getNextSibling())
   {
      if (XMLString::equals(chld->getNodeName(), X("boundingBox")))
      {
         XmlReader::StrToQuadCoord(chld->getTextContent(), mLlCorner.mX, mLlCorner.mY, mUrCorner.mX, mUrCorner.mY);
         mHasPixelCoords = true;
      }
      else if (XMLString::equals(chld->getNodeName(), X("geoBox")))
      {
         XmlReader::StrToQuadCoord(chld->getTextContent(), mLlLatLong.mX, mLlLatLong.mY, mUrLatLong.mX, mUrLatLong.mY);
         mHasGeoCoords = true;
      }
   }

   return true;
}

///////////////////////
// FillColorProperty //
///////////////////////

FillColorProperty::FillColorProperty(ColorType color) :
   GraphicProperty("FillColor"),
   mColor(color)
{
}

bool FillColorProperty::set(const GraphicProperty* pProp)
{
   const FillColorProperty* pColorProp = dynamic_cast<const FillColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   *this = *pColorProp;
   return true;
}

bool FillColorProperty::compare(const GraphicProperty* pProp) const
{
   const FillColorProperty* pColorProp = dynamic_cast<const FillColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   return mColor == pColorProp->getColor();
}

GraphicProperty* FillColorProperty::copy() const
{
   return new FillColorProperty(mColor);
}

ColorType FillColorProperty::getColor() const
{
   return mColor;
}

bool FillColorProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("FillColor"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   pXml->addText(buf.str(), pXml->addElement("color"));
   pXml->popAddPoint();

   return true;
}

bool FillColorProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("color")));
   double r;
   double g;
   double b;
   double dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r, g, b);

   return true;
}

///////////////////////
// FillStyleProperty //
///////////////////////

FillStyleProperty::FillStyleProperty(FillStyle eStyle) :
   GraphicProperty("FillStyle"),
   mStyle(eStyle)
{
}

bool FillStyleProperty::set(const GraphicProperty* pProp)
{
   const FillStyleProperty* pStyleProp = dynamic_cast<const FillStyleProperty*>(pProp);
   if (pStyleProp == NULL)
   {
      return false;
   }

   *this = *pStyleProp;
   return true;
}

bool FillStyleProperty::compare(const GraphicProperty* pProp) const
{
   const FillStyleProperty* pStyleProp = dynamic_cast<const FillStyleProperty*>(pProp);
   if (pStyleProp == NULL)
   {
      return false;
   }

   return mStyle == pStyleProp->getFillStyle();
}

GraphicProperty* FillStyleProperty::copy() const
{
   return new FillStyleProperty(mStyle);
}

FillStyle FillStyleProperty::getFillStyle() const
{
   return mStyle;
}

bool FillStyleProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("FillStyle"));
   GraphicProperty::toXml(pXml);
   switch (mStyle)
   {
      case SOLID_FILL:
         pXml->addAttr("style", "solid");
         break;
      case HATCH:
         pXml->addAttr("style", "hatch");
         break;
      case EMPTY_FILL:
         pXml->addAttr("style", "empty");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool FillStyleProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string style(A(elmnt->getAttribute(X("style"))));

   if (style == "solid")
   {
      mStyle = SOLID_FILL;
   }
   else if (style == "hatch")
   {
      mStyle = HATCH;
   }
   else if (style == "empty")
   {
      mStyle = EMPTY_FILL;
   }

   return true;
}

//////////////////
// FontProperty //
//////////////////

FontProperty::FontProperty(const FontImp& font) :
   GraphicProperty("Font"),
   mFont(font)
{
}

bool FontProperty::set(const GraphicProperty* pProp)
{
   const FontProperty* pFontProp = dynamic_cast<const FontProperty*>(pProp);
   if (pFontProp == NULL)
   {
      return false;
   }

   *this = *pFontProp;
   return true;
}

bool FontProperty::compare(const GraphicProperty* pProp) const
{
   const FontProperty* pFontProp = dynamic_cast<const FontProperty*>(pProp);
   if (pFontProp == NULL)
   {
      return false;
   }

   return mFont == pFontProp->getFont();
}

GraphicProperty* FontProperty::copy() const
{
   return new FontProperty(mFont);
}

const FontImp& FontProperty::getFont() const
{
   return mFont;
}

bool FontProperty::toXml(XMLWriter* pXml) const
{
   const QFont& font = mFont.getQFont();

   pXml->pushAddPoint(pXml->addElement("Font"));
   GraphicProperty::toXml(pXml);

   pXml->addAttr("face", font.family().toStdString());

   stringstream buf;
   buf << font.pointSize();
   pXml->addAttr("size", buf.str());
   pXml->addAttr("bold", (font.bold()) ? "true" : "false");
   pXml->addAttr("italic", (font.italic()) ? "true" : "false");
   pXml->addAttr("underline", (font.underline()) ? "true" : "false");
   pXml->popAddPoint();

   return true;
}

bool FontProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));

   string face = GraphicLayer::getSettingTextFont();
   int iSize = GraphicLayer::getSettingTextFontSize();
   bool bBold = GraphicLayer::getSettingTextBold();
   bool bItalic = GraphicLayer::getSettingTextItalics();
   bool bUnderline = GraphicLayer::getSettingTextUnderline();

   if (elmnt->hasAttribute(X("face")))
   {
      face = A(elmnt->getAttribute(X("face")));
   }

   if (elmnt->hasAttribute(X("size")))
   {
      iSize = atoi(A(elmnt->getAttribute(X("size"))));
   }

   if (elmnt->hasAttribute(X("bold")))
   {
      string tmp(A(elmnt->getAttribute(X("bold"))));
      if (tmp == "0" || tmp == "f" || tmp == "false")
      {
         bBold = false;
      }
      else
      {
         bBold = true;
      }
   }

   if (elmnt->hasAttribute(X("italic")))
   {
      string tmp(A(elmnt->getAttribute(X("italic"))));
      if (tmp == "0" || tmp == "f" || tmp == "false")
      {
         bItalic = false;
      }
      else
      {
         bItalic = true;
      }
   }

   if (elmnt->hasAttribute(X("underline")))
   {
      string tmp(A(elmnt->getAttribute(X("underline"))));
      if (tmp == "0" || tmp == "f" || tmp == "false")
      {
         bUnderline = false;
      }
      else
      {
         bUnderline = true;
      }
   }

   mFont = QFont(QString::fromStdString(face), iSize);
   mFont.setBold(bBold);
   mFont.setItalic(bItalic);
   mFont.setUnderline(bUnderline);

   return true;
}

////////////////////////
// HatchStyleProperty //
////////////////////////

HatchStyleProperty::HatchStyleProperty(SymbolType eHatch) :
   GraphicProperty("HatchStyle"),
   mStyle(eHatch)
{
}

bool HatchStyleProperty::set(const GraphicProperty* pProp)
{
   const HatchStyleProperty* pHatchProp = dynamic_cast<const HatchStyleProperty*>(pProp);
   if (pHatchProp == NULL)
   {
      return false;
   }

   *this = *pHatchProp;
   return true;
}

bool HatchStyleProperty::compare(const GraphicProperty* pProp) const
{
   const HatchStyleProperty* pHatchProp = dynamic_cast<const HatchStyleProperty*>(pProp);
   if (pHatchProp == NULL)
   {
      return false;
   }

   return mStyle == pHatchProp->getHatchStyle();
}

GraphicProperty* HatchStyleProperty::copy() const
{
   return new HatchStyleProperty(mStyle);
}

SymbolType HatchStyleProperty::getHatchStyle() const
{
   return mStyle;
}

bool HatchStyleProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("HatchStyle"));
   GraphicProperty::toXml(pXml);
   switch (mStyle)
   {
      case SOLID:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "solid");
         break;
      case X:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "x");
         break;
      case CROSS_HAIR:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "cross hair");
         break;
      case ASTERISK:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "asterisk");
         break;
      case HORIZONTAL_LINE:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "horizontal line");
         break;
      case VERTICAL_LINE:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "vertical line");
         break;
      case FORWARD_SLASH:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "forward slash");
         break;
      case BACK_SLASH:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("style", "back slash");
         break;
      case BOX:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "solid");
         break;
      case BOXED_X:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "x");
         break;
      case BOXED_CROSS_HAIR:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "cross hair");
         break;
      case BOXED_ASTERISK:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "asterisk");
         break;
      case BOXED_HORIZONTAL_LINE:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "horizontal line");
         break;
      case BOXED_VERTICAL_LINE:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "vertical line");
         break;
      case BOXED_FORWARD_SLASH:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "forward slash");
         break;
      case BOXED_BACK_SLASH:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("style", "back slash");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool HatchStyleProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string style(A(elmnt->getAttribute(X("style"))));
   bool boxed(true);
   {
      string boxedS(A(elmnt->getAttribute(X("boxed"))));
      if (boxedS == "0" || boxedS == "f" || boxedS == "false")
      {
         boxed = false;
      }
   }

   if (style == "solid" && !boxed)
   {
      mStyle = SOLID;
   }
   else if (style == "x" && !boxed)
   {
      mStyle = X;
   }
   else if (style == "cross hair" && !boxed)
   {
      mStyle = CROSS_HAIR;
   }
   else if (style == "asterisk" && !boxed)
   {
      mStyle = ASTERISK;
   }
   else if (style == "horizontal line" && !boxed)
   {
      mStyle = HORIZONTAL_LINE;
   }
   else if (style == "vertical line" && !boxed)
   {
      mStyle = VERTICAL_LINE;
   }
   else if (style == "forward slash" && !boxed)
   {
      mStyle = FORWARD_SLASH;
   }
   else if (style == "back slash" && !boxed)
   {
      mStyle = BACK_SLASH;
   }
   else if (style == "solid" && boxed)
   {
      mStyle = BOX;
   }
   else if (style == "x" && boxed)
   {
      mStyle = BOXED_X;
   }
   else if (style == "cross hair" && boxed)
   {
      mStyle = BOXED_CROSS_HAIR;
   }
   else if (style == "asterisk" && boxed)
   {
      mStyle = BOXED_ASTERISK;
   }
   else if (style == "horizontal line" && boxed)
   {
      mStyle = BOXED_HORIZONTAL_LINE;
   }
   else if (style == "vertical line" && boxed)
   {
      mStyle = BOXED_VERTICAL_LINE;
   }
   else if (style == "forward slash" && boxed)
   {
      mStyle = BOXED_FORWARD_SLASH;
   }
   else if (style == "back slash" && boxed)
   {
      mStyle = BOXED_BACK_SLASH;
   }

   return true;
}

////////////////////
// LatLonProperty //
////////////////////

LatLonProperty::LatLonProperty(LatLonPoint latLonPoint) :
   GraphicProperty("LatLon"),
   mLatLon(latLonPoint)
{
}

LatLonPoint LatLonProperty::getLatLon() const
{
   return mLatLon;
}

bool LatLonProperty::set(const GraphicProperty* pProp)
{
   const LatLonProperty* pLatLonProp = dynamic_cast<const LatLonProperty*>(pProp);
   if (pLatLonProp == NULL)
   {
      return false;
   }

   *this = *pLatLonProp;
   return true;
}

bool LatLonProperty::compare(const GraphicProperty* pProp) const
{
   const LatLonProperty* pLatLonProp = dynamic_cast<const LatLonProperty*>(pProp);
   if (pLatLonProp == NULL)
   {
      return false;
   }

   return mLatLon == pLatLonProp->getLatLon();
}

GraphicProperty* LatLonProperty::copy() const
{
   return new LatLonProperty(mLatLon);
}

bool LatLonProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LatLon"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << getLatLon().getLatitude().getValue() << ' ' << getLatLon().getLongitude().getValue();
   pXml->addText(buf.str(), pXml->addElement("point"));
   pXml->popAddPoint();

   return true;
}

bool LatLonProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("point")));
   XMLStringTokenizer t(elmnts->item(0)->getTextContent());

   string lat;
   string lon;
   if (t.hasMoreTokens())
   {
      lat = A(t.nextToken());
   }

   if (t.hasMoreTokens())
   {
      lon = A(t.nextToken());
   }

   mLatLon = LatLonPoint(lat, lon);
   return true;
}

///////////////////////
// LineColorProperty //
///////////////////////

LineColorProperty::LineColorProperty(ColorType color) :
   GraphicProperty("LineColor"),
   mColor(color)
{
}

bool LineColorProperty::set(const GraphicProperty* pProp)
{
   const LineColorProperty* pColorProp = dynamic_cast<const LineColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   *this = *pColorProp;
   return true;
}

bool LineColorProperty::compare(const GraphicProperty* pProp) const
{
   const LineColorProperty* pColorProp = dynamic_cast<const LineColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   return mColor == pColorProp->getColor();
}

GraphicProperty* LineColorProperty::copy() const
{
   return new LineColorProperty(mColor);
}

ColorType LineColorProperty::getColor() const
{
   return mColor;
}

bool LineColorProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LineColor"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   pXml->addText(buf.str(), pXml->addElement("color"));
   pXml->popAddPoint();

   return true;
}

bool LineColorProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("color")));
   double r;
   double g;
   double b;
   double dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r, g, b);

   return true;
}

////////////////////
// LineOnProperty //
////////////////////

LineOnProperty::LineOnProperty(bool state) :
   GraphicProperty("LineOn"),
   mState(state)
{
}

bool LineOnProperty::set(const GraphicProperty* pProp)
{
   const LineOnProperty* pLineOnProp = dynamic_cast<const LineOnProperty*>(pProp);
   if (pLineOnProp == NULL)
   {
      return false;
   }

   *this = *pLineOnProp;
   return true;
}

bool LineOnProperty::compare(const GraphicProperty* pProp) const
{
   const LineOnProperty* pLineOnProp = dynamic_cast<const LineOnProperty*>(pProp);
   if (pLineOnProp == NULL)
   {
      return false;
   }

   return mState == pLineOnProp->getState();
}

GraphicProperty* LineOnProperty::copy() const
{
   return new LineOnProperty(mState);
}

bool LineOnProperty::getState() const
{
   return mState;
}

bool LineOnProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LineOn"));
   GraphicProperty::toXml(pXml);
   pXml->addAttr("on", mState ? "true" : "false");
   pXml->popAddPoint();

   return true;
}

bool LineOnProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   string v(A(static_cast<DOMElement*>(pDocument)->getAttribute(X("on"))));
   if (v == "0" || v == "f" || v == "false")
   {
      mState = false;
   }
   else
   {
      mState = true;
   }

   return true;
}

////////////////////////
// LineScaledProperty //
////////////////////////

LineScaledProperty::LineScaledProperty(bool scaled) :
   GraphicProperty("LineScaled"),
   mScaled(scaled)
{
}

bool LineScaledProperty::set(const GraphicProperty* pProp)
{
   const LineScaledProperty* pScaledProp = dynamic_cast<const LineScaledProperty*>(pProp);
   if (pScaledProp == NULL)
   {
      return false;
   }

   mScaled = pScaledProp->getScaled();
   return true;
}

bool LineScaledProperty::compare(const GraphicProperty* pProp) const
{
   const LineScaledProperty* pScaledProp = dynamic_cast<const LineScaledProperty*>(pProp);
   if (pScaledProp == NULL)
   {
      return false;
   }

   return mScaled == pScaledProp->getScaled();
}

GraphicProperty* LineScaledProperty::copy() const
{
   return new LineScaledProperty(mScaled);
}

bool LineScaledProperty::getScaled() const
{
   return mScaled;
}

bool LineScaledProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LineScaled"));
   GraphicProperty::toXml(pXml);
   pXml->addAttr("scaled", (mScaled) ? "true" : "false");
   pXml->popAddPoint();

   return true;
}

bool LineScaledProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   string v(A(static_cast<DOMElement*>(pDocument)->getAttribute(X("scaled"))));
   if (v == "0" || v == "f" || v == "false")
   {
      mScaled = false;
   }
   else
   {
      mScaled = true;
   }

   return true;
}

///////////////////////
// LineStyleProperty //
///////////////////////

LineStyleProperty::LineStyleProperty(LineStyle eStyle) :
   GraphicProperty("LineStyle"),
   mStyle(eStyle)
{
}

bool LineStyleProperty::set(const GraphicProperty* pProp)
{
   const LineStyleProperty* pStyleProp = dynamic_cast<const LineStyleProperty*>(pProp);
   if (pStyleProp == NULL)
   {
      return false;
   }

   *this = *pStyleProp;
   return true;
}

bool LineStyleProperty::compare(const GraphicProperty* pProp) const
{
   const LineStyleProperty* pStyleProp = dynamic_cast<const LineStyleProperty*>(pProp);
   if (pStyleProp == NULL)
   {
      return false;
   }

   return mStyle == pStyleProp->getStyle();
}

GraphicProperty* LineStyleProperty::copy() const
{
   return new LineStyleProperty(mStyle);
}

LineStyle LineStyleProperty::getStyle() const
{
   return mStyle;
}

bool LineStyleProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LineStyle"));
   GraphicProperty::toXml(pXml);
   switch (mStyle)
   {
      case SOLID_LINE:
         pXml->addAttr("style", "solid");
         break;
      case DASHED:
         pXml->addAttr("style", "dashed");
         break;
      case DOT:
         pXml->addAttr("style", "dot");
         break;
      case DASH_DOT:
         pXml->addAttr("style", "dash dot");
         break;
      case DASH_DOT_DOT:
         pXml->addAttr("style", "dash dot dot");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool LineStyleProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   string style(A(static_cast<DOMElement*>(pDocument)->getAttribute(X("style"))));
   if (style == "solid")
   {
      mStyle = SOLID_LINE;
   }
   else if (style == "dashed")
   {
      mStyle = DASHED;
   }
   else if (style == "dot")
   {
      mStyle = DOT;
   }
   else if (style == "dash dot")
   {
      mStyle = DASH_DOT;
   }
   else if (style == "dash dot dot")
   {
      mStyle = DASH_DOT_DOT;
   }

   return true;
}

///////////////////////
// LineWidthProperty //
///////////////////////

LineWidthProperty::LineWidthProperty(double width) :
   GraphicProperty("LineWidth"),
   mWidth(width)
{
}

bool LineWidthProperty::set(const GraphicProperty* pProp)
{
   const LineWidthProperty* pWidthProp = dynamic_cast<const LineWidthProperty*>(pProp);
   if (pWidthProp == NULL)
   {
      return false;
   }

   *this = *pWidthProp;
   return true;
}

bool LineWidthProperty::compare(const GraphicProperty* pProp) const
{
   const LineWidthProperty* pWidthProp = dynamic_cast<const LineWidthProperty*>(pProp);
   if (pWidthProp == NULL)
   {
      return false;
   }

   return mWidth == pWidthProp->getWidth();
}

GraphicProperty* LineWidthProperty::copy() const
{
   return new LineWidthProperty(mWidth);
}

double LineWidthProperty::getWidth() const
{
   return mWidth;
}

bool LineWidthProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("LineWidth"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mWidth;
   pXml->addAttr("width", buf.str());
   pXml->popAddPoint();

   return true;
}

bool LineWidthProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream buf(A(elmnt->getAttribute(X("width"))));
   buf >> mWidth;

   return true;
}

//////////////////////
// RotationProperty //
//////////////////////

RotationProperty::RotationProperty(double rotate) :
   GraphicProperty("Rotation"),
   mRotate(rotate)
{
}

bool RotationProperty::set(const GraphicProperty* pProp)
{
   const RotationProperty* pRotationProp = dynamic_cast<const RotationProperty*>(pProp);
   if (pRotationProp == NULL)
   {
      return false;
   }

   *this = *pRotationProp;
   return true;
}

bool RotationProperty::compare(const GraphicProperty* pProp) const
{
   const RotationProperty* pRotationProp = dynamic_cast<const RotationProperty*>(pProp);
   if (pRotationProp == NULL)
   {
      return false;
   }

   return mRotate == pRotationProp->getRotation();
}

GraphicProperty* RotationProperty::copy() const
{
   return new RotationProperty(mRotate);
}

double RotationProperty::getRotation() const
{
   return mRotate;
}

bool RotationProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Rotation"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mRotate;
   pXml->addAttr("rotation", buf.str());
   pXml->popAddPoint();

   return true;
}

bool RotationProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream buf(A(elmnt->getAttribute(X("rotation"))));
   buf >> mRotate;

   return true;
}

///////////////////
// ScaleProperty //
///////////////////

ScaleProperty::ScaleProperty(double scale) :
   GraphicProperty("Scale"),
   mScale(scale)
{
}

bool ScaleProperty::set(const GraphicProperty* pProp)
{
   const ScaleProperty* pScaleProp = dynamic_cast<const ScaleProperty*>(pProp);
   if (pScaleProp == NULL)
   {
      return false;
   }

   *this = *pScaleProp;
   return true;
}

bool ScaleProperty::compare(const GraphicProperty* pProp) const
{
   const ScaleProperty* pScaleProp = dynamic_cast<const ScaleProperty*>(pProp);
   if (pScaleProp == NULL)
   {
      return false;
   }

   return mScale == pScaleProp->getScale();
}

GraphicProperty* ScaleProperty::copy() const
{
   return new ScaleProperty(mScale);
}

double ScaleProperty::getScale() const
{
   return mScale;
}

bool ScaleProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Scale"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mScale;
   pXml->addAttr("scale", buf.str());
   pXml->popAddPoint();

   return true;
}

bool ScaleProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream buf(A(elmnt->getAttribute(X("scale"))));
   buf >> mScale;

   return true;
}

///////////////////////////
// TextAlignmentProperty //
///////////////////////////

TextAlignmentProperty::TextAlignmentProperty(int iAlignment) :
   GraphicProperty("TextAlignment"),
   mAlignment(iAlignment)
{
}

int TextAlignmentProperty::getAlignment() const
{
   return mAlignment;
}

bool TextAlignmentProperty::set(const GraphicProperty* pProperty)
{
   const TextAlignmentProperty* pTextProperty = dynamic_cast<const TextAlignmentProperty*>(pProperty);
   if (pTextProperty == NULL)
   {
      return false;
   }

   *this = *pTextProperty;
   return true;
}

bool TextAlignmentProperty::compare(const GraphicProperty* pProperty) const
{
   const TextAlignmentProperty* pTextProperty = dynamic_cast<const TextAlignmentProperty*>(pProperty);
   if (pTextProperty == NULL)
   {
      return false;
   }

   return mAlignment == pTextProperty->getAlignment();
}

GraphicProperty* TextAlignmentProperty::copy() const
{
   return new TextAlignmentProperty(mAlignment);
}

bool TextAlignmentProperty::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   pXml->pushAddPoint(pXml->addElement("TextAlignment"));

   bool bSuccess = GraphicProperty::toXml(pXml);
   if (bSuccess == true)
   {
      stringstream buf;
      buf << mAlignment;
      pXml->addAttr("alignment", buf.str());
   }

   pXml->popAddPoint();
   return bSuccess;
}

bool TextAlignmentProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicProperty::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      DOMElement* pElement = static_cast<DOMElement*> (pDocument);

      stringstream buf(A(pElement->getAttribute(X("alignment"))));
      buf >> mAlignment;
   }

   return bSuccess;
}

///////////////////////
// TextColorProperty //
///////////////////////

TextColorProperty::TextColorProperty(ColorType color) :
   GraphicProperty("TextColor"),
   mColor(color)
{
}

bool TextColorProperty::set(const GraphicProperty* pProp)
{
   const TextColorProperty* pColorProp = dynamic_cast<const TextColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   *this = *pColorProp;
   return true;
}

bool TextColorProperty::compare(const GraphicProperty* pProp) const
{
   const TextColorProperty* pColorProp = dynamic_cast<const TextColorProperty*>(pProp);
   if (pColorProp == NULL)
   {
      return false;
   }

   return mColor == pColorProp->getColor();
}

GraphicProperty* TextColorProperty::copy() const
{
   return new TextColorProperty(mColor);
}

ColorType TextColorProperty::getColor() const
{
   return mColor;
}

bool TextColorProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("TextColor"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   pXml->addText(buf.str(), pXml->addElement("color"));
   pXml->popAddPoint();

   return true;
}

bool TextColorProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("color")));
   double r;
   double g;
   double b;
   double dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r, g, b);
   return true;
}

////////////////////////
// TextStringProperty //
////////////////////////

TextStringProperty::TextStringProperty(const string& text) :
   GraphicProperty("TextString"),
   mString(text)
{
}

bool TextStringProperty::set(const GraphicProperty* pProp)
{
   const TextStringProperty* pTextProp = dynamic_cast<const TextStringProperty*>(pProp);
   if (pTextProp == NULL)
   {
      return false;
   }

   *this = *pTextProp;
   return true;
}

bool TextStringProperty::compare(const GraphicProperty* pProp) const
{
   const TextStringProperty* pTextProp = dynamic_cast<const TextStringProperty*>(pProp);
   if (pTextProp == NULL)
   {
      return false;
   }

   return mString == pTextProp->getString();
}

GraphicProperty* TextStringProperty::copy() const
{
   return new TextStringProperty(mString);
}

const string& TextStringProperty::getString() const
{
   return mString;
}

bool TextStringProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("TextString"));
   GraphicProperty::toXml(pXml);
   pXml->addText(mString, pXml->addElement("string"));
   pXml->popAddPoint();

   return true;
}

bool TextStringProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("string")));
   mString = A(elmnts->item(0)->getTextContent());

   return true;
}

///////////////////
// WedgeProperty //
///////////////////

WedgeProperty::WedgeProperty(double dStartAngle, double dStopAngle) :
   GraphicProperty("Wedge"),
   mStartAngle(dStartAngle),
   mStopAngle(dStopAngle)
{
   normalizeAngles();
}

bool WedgeProperty::set(const GraphicProperty* pProperty)
{
   const WedgeProperty* pWedgeProp = dynamic_cast<const WedgeProperty*>(pProperty);
   if (pWedgeProp == NULL)
   {
      return false;
   }

   *this = *pWedgeProp;
   return true;
}

bool WedgeProperty::compare(const GraphicProperty* pProp) const
{
   const WedgeProperty* pWedgeProp = dynamic_cast<const WedgeProperty*>(pProp);
   if (pWedgeProp == NULL)
   {
      return false;
   }

   double start = pWedgeProp->getStartAngle();
   double stop = pWedgeProp->getStopAngle();

   const double compareTolerance = 0.001;
   return (fabs(mStartAngle - start) < compareTolerance) && (fabs(mStopAngle - stop) < compareTolerance);
}

GraphicProperty* WedgeProperty::copy() const
{
   return new WedgeProperty(mStartAngle, mStopAngle);
}

double WedgeProperty::getStartAngle() const
{
   return mStartAngle;
}

double WedgeProperty::getStopAngle() const
{
   return mStopAngle;
}

bool WedgeProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Wedge"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mStartAngle;
   pXml->addAttr("start", buf.str());

   buf.str("");
   buf << mStopAngle;
   pXml->addAttr("stop", buf.str());
   pXml->popAddPoint();

   return true;
}

bool WedgeProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   stringstream startStream(A(elmnt->getAttribute(X("start"))));
   startStream >> mStartAngle;

   stringstream stopStream(A(elmnt->getAttribute(X("stop"))));
   stopStream >> mStopAngle;

   return true;
}

void WedgeProperty::normalizeAngles()
{
   while (mStartAngle < 0.0)
   {
      mStartAngle += 360.0;
   }

   while (mStartAngle >= 360.0)
   {
      mStartAngle -= 360.0;
   }

   while (mStopAngle < 0.0)
   {
      mStopAngle += 360.0;
   }

   while (mStopAngle >= 360.0)
   {
      mStopAngle -= 360.0;
   }

   if (mStartAngle > mStopAngle)
   {
      if (mStartAngle <= 0.0)
      {
         mStopAngle += 360.0;
      }
      else if (mStartAngle > 0.0)
      {
         mStartAngle -= 360.0;
      }
   }

   while ((mStopAngle - mStartAngle) >= 360.0)
   {
      mStopAngle -= 360.0;
   }
}

///////////////////////
// PaperSizeProperty //
///////////////////////

PaperSizeProperty::PaperSizeProperty(LocationType size) :
   GraphicProperty("PaperSize"),
   mSize(size)
{
}

bool PaperSizeProperty::set(const GraphicProperty* pProperty)
{
   const PaperSizeProperty* pSizeProp = dynamic_cast<const PaperSizeProperty*>(pProperty);
   if (pSizeProp == NULL)
   {
      return false;
   }

   *this = *pSizeProp;
   return true;
}

bool PaperSizeProperty::compare(const GraphicProperty* pProp) const
{
   const PaperSizeProperty* pSizeProp = dynamic_cast<const PaperSizeProperty*>(pProp);
   if (pSizeProp == NULL)
   {
      return false;
   }

   return mSize == pSizeProp->getSize();
}

GraphicProperty* PaperSizeProperty::copy() const
{
   return new PaperSizeProperty(mSize);
}

LocationType PaperSizeProperty::getSize() const
{
   return mSize;
}

bool PaperSizeProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("PaperSize"));
   GraphicProperty::toXml(pXml);
   stringstream buf;
   buf << mSize.mX << ' ' << mSize.mY;
   pXml->addText(buf.str(), pXml->addElement("size"));
   pXml->popAddPoint();

   return true;
}

bool PaperSizeProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMNodeList* elmnts(static_cast<DOMElement*>(pDocument)->getElementsByTagName(X("size")));
   return XmlReader::StrToLocation(elmnts->item(0)->getTextContent(), mSize);
}

//////////////////////
// FileNameProperty //
//////////////////////

FileNameProperty::FileNameProperty(const string &filename) :
   GraphicProperty("Filename"),
   mFileName(filename)
{
}

const string& FileNameProperty::getFileName() const
{
   return mFileName;
}

bool FileNameProperty::set(const GraphicProperty* pProp)
{
   const FileNameProperty* pFileNameProp = dynamic_cast<const FileNameProperty*>(pProp);
   if (pFileNameProp == NULL)
   {
      return false;
   }

   *this = *pFileNameProp;
   return true;
}

bool FileNameProperty::compare(const GraphicProperty* pProp) const
{
   const FileNameProperty* pFileNameProp = dynamic_cast<const FileNameProperty*>(pProp);
   if (pFileNameProp == NULL)
   {
      return false;
   }

   return mFileName == pFileNameProp->getFileName();
}

GraphicProperty* FileNameProperty::copy() const
{
   return new FileNameProperty(mFileName);
}

bool FileNameProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("Filename"));
   GraphicProperty::toXml(pXml);
   pXml->addAttr("source", XmlBase::PathToURL(getFileName()));
   pXml->popAddPoint();

   return true;
}

bool FileNameProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   bool bSuccess = GraphicProperty::fromXml(pDocument, version);
   if (bSuccess == true)
   {
      mFileName = XmlBase::URLtoPath(static_cast<DOMElement*>(pDocument)->getAttribute(X("source")));
   }

   return bSuccess;
}

/////////////////////////
// PixelSymbolProperty //
/////////////////////////

PixelSymbolProperty::PixelSymbolProperty(SymbolType symbol) :
   GraphicProperty("PixelSymbol"),
   mSymbol(symbol)
{
}

SymbolType PixelSymbolProperty::getPixelSymbol() const
{
   return mSymbol;
}

bool PixelSymbolProperty::compare(const GraphicProperty* pProp) const
{
   const PixelSymbolProperty* pSymbolProp = dynamic_cast<const PixelSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   return mSymbol == pSymbolProp->getPixelSymbol();
}

GraphicProperty* PixelSymbolProperty::copy() const
{
   return new PixelSymbolProperty(mSymbol);
}

bool PixelSymbolProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("PixelSymbol"));
   GraphicProperty::toXml(pXml);
   switch (mSymbol)
   {
      case SOLID:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "solid");
         break;
      case X:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "x");
         break;
      case CROSS_HAIR:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "cross hair");
         break;
      case ASTERISK:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "asterisk");
         break;
      case HORIZONTAL_LINE:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "horizontal line");
         break;
      case VERTICAL_LINE:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "vertical line");
         break;
      case FORWARD_SLASH:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "forward slash");
         break;
      case BACK_SLASH:
         pXml->addAttr("boxed", "false");
         pXml->addAttr("symbol", "back slash");
         break;
      case BOX:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "solid");
         break;
      case BOXED_X:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "x");
         break;
      case BOXED_CROSS_HAIR:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "cross hair");
         break;
      case BOXED_ASTERISK:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "asterisk");
         break;
      case BOXED_HORIZONTAL_LINE:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "horizontal line");
         break;
      case BOXED_VERTICAL_LINE:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "vertical line");
         break;
      case BOXED_FORWARD_SLASH:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "forward slash");
         break;
      case BOXED_BACK_SLASH:
         pXml->addAttr("boxed", "true");
         pXml->addAttr("symbol", "back slash");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool PixelSymbolProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string symbol(A(elmnt->getAttribute(X("symbol"))));
   bool boxed(true);

   string boxedS(A(elmnt->getAttribute(X("boxed"))));
   if (boxedS == "0" || boxedS == "f" || boxedS == "false")
   {
      boxed = false;
   }

   if (!boxed)
   {
      if (symbol == "solid")
      {
         mSymbol = SOLID;
      }
      else if (symbol == "x")
      {
         mSymbol = X;
      }
      else if (symbol == "cross hair")
      {
         mSymbol = CROSS_HAIR;
      }
      else if (symbol == "asterisk")
      {
         mSymbol = ASTERISK;
      }
      else if (symbol == "horizontal line")
      {
         mSymbol = HORIZONTAL_LINE;
      }
      else if (symbol == "vertical line")
      {
         mSymbol = VERTICAL_LINE;
      }
      else if (symbol == "forward slash")
      {
         mSymbol = FORWARD_SLASH;
      }
      else if (symbol == "back slash")
      {
         mSymbol = BACK_SLASH;
      }
   }
   else
   {
      if (symbol == "solid")
      {
         mSymbol = BOX;
      }
      else if (symbol == "x")
      {
         mSymbol = BOXED_X;
      }
      else if (symbol == "cross hair")
      {
         mSymbol = BOXED_CROSS_HAIR;
      }
      else if (symbol == "asterisk")
      {
         mSymbol = BOXED_ASTERISK;
      }
      else if (symbol == "horizontal line")
      {
         mSymbol = BOXED_HORIZONTAL_LINE;
      }
      else if (symbol == "vertical line")
      {
         mSymbol = BOXED_VERTICAL_LINE;
      }
      else if (symbol == "forward slash")
      {
         mSymbol = BOXED_FORWARD_SLASH;
      }
      else if (symbol == "back slash")
      {
         mSymbol = BOXED_BACK_SLASH;
      }
   }

   return true;
}

bool PixelSymbolProperty::set(const GraphicProperty* pProp)
{
   const PixelSymbolProperty* pSymbolProp = dynamic_cast<const PixelSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   mSymbol = pSymbolProp->mSymbol;
   return true;
}

//////////////////////
// DrawModeProperty //
//////////////////////

DrawModeProperty::DrawModeProperty(ModeType mode) :
   GraphicProperty("DrawMode"),
   mMode(mode)
{
}

ModeType DrawModeProperty::getDrawMode() const
{
   return mMode;
}

bool DrawModeProperty::compare(const GraphicProperty* pProp) const
{
   const DrawModeProperty* pModeProp = dynamic_cast<const DrawModeProperty*>(pProp);
   if (pModeProp == NULL)
   {
      return false;
   }

   return mMode == pModeProp->getDrawMode();
}

GraphicProperty* DrawModeProperty::copy() const
{
   return new DrawModeProperty(mMode);
}

bool DrawModeProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("DrawMode"));
   GraphicProperty::toXml(pXml);
   switch (mMode)
   {
      case DRAW:
         pXml->addAttr("mode", "draw");
         break;
      case ERASE:
         pXml->addAttr("mode", "erase");
         break;
      case TOGGLE:
         pXml->addAttr("mode", "toggle");
         break;
      default:
         break;
   }

   pXml->popAddPoint();
   return true;
}

bool DrawModeProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string mode(A(elmnt->getAttribute(X("mode"))));

   if (mode == "draw")
   {
      mMode = DRAW;
   }
   else if (mode == "erase")
   {
      mMode = ERASE;
   }
   else if (mode == "toggle")
   {
      mMode = TOGGLE;
   }

   return true;
}

bool DrawModeProperty::set(const GraphicProperty *pProp)
{
   const DrawModeProperty* pModeProp = dynamic_cast<const DrawModeProperty*>(pProp);
   if (pModeProp == NULL)
   {
      return false;
   }

   mMode = pModeProp->mMode;
   return true;
}

///////////////////////////
// GraphicSymbolProperty //
///////////////////////////

GraphicSymbolProperty::GraphicSymbolProperty(const string& symbolName) :
   GraphicProperty("GraphicSymbol"),
   mSymbolName(symbolName)
{
}

const string& GraphicSymbolProperty::getSymbolName() const
{
   return mSymbolName;
}

bool GraphicSymbolProperty::compare(const GraphicProperty* pProp) const
{
   const GraphicSymbolProperty* pSymbolProp = dynamic_cast<const GraphicSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   return mSymbolName == pSymbolProp->getSymbolName();
}

GraphicProperty* GraphicSymbolProperty::copy() const
{
   return new GraphicSymbolProperty(mSymbolName);
}

bool GraphicSymbolProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("GraphicSymbol"));
   GraphicProperty::toXml(pXml);
   pXml->addAttr("name", mSymbolName);
   pXml->popAddPoint();

   return true;
}

bool GraphicSymbolProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   mSymbolName = A(elmnt->getAttribute(X("name")));

   return true;
}

bool GraphicSymbolProperty::set(const GraphicProperty *pProp)
{
   const GraphicSymbolProperty* pSymbolProp = dynamic_cast<const GraphicSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   mSymbolName = pSymbolProp->mSymbolName;
   return true;
}

///////////////////////////////
// GraphicSymbolSizeProperty //
///////////////////////////////

GraphicSymbolSizeProperty::GraphicSymbolSizeProperty(unsigned int symbolSize) :
   GraphicProperty("GraphicSymbolSize"),
   mSymbolSize(symbolSize)
{
}

const unsigned int GraphicSymbolSizeProperty::getSymbolSize() const
{
   return mSymbolSize;
}

bool GraphicSymbolSizeProperty::compare(const GraphicProperty* pProp) const
{
   const GraphicSymbolSizeProperty* pSymbolProp = dynamic_cast<const GraphicSymbolSizeProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   return mSymbolSize == pSymbolProp->getSymbolSize();
}

GraphicProperty* GraphicSymbolSizeProperty::copy() const
{
   return new GraphicSymbolSizeProperty(mSymbolSize);
}

bool GraphicSymbolSizeProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("GraphicSymbolSize"));
   GraphicProperty::toXml(pXml);
   pXml->addAttr("size", mSymbolSize);
   pXml->popAddPoint();

   return true;
}

bool GraphicSymbolSizeProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string symbolSize = A(elmnt->getAttribute(X("size")));
   mSymbolSize = boost::lexical_cast<unsigned int>(symbolSize);

   return true;
}

bool GraphicSymbolSizeProperty::set(const GraphicProperty* pProp)
{
   const GraphicSymbolSizeProperty* pSymbolProp = dynamic_cast<const GraphicSymbolSizeProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }

   mSymbolSize = pSymbolProp->mSymbolSize;
   return true;
}

//////////////////////////
// GraphicUnitsProperty //
//////////////////////////

GraphicUnitsProperty::GraphicUnitsProperty(UnitSystem units) :
   GraphicProperty("UnitSystem"),
   mUnits(units)
{}

UnitSystem GraphicUnitsProperty::getUnitSystem() const
{
   return mUnits;
}

bool GraphicUnitsProperty::compare(const GraphicProperty* pProp) const
{
   const GraphicUnitsProperty* pUnitsProp = dynamic_cast<const GraphicUnitsProperty*>(pProp);
   if (pUnitsProp == NULL)
   {
      return false;
   }

   return mUnits == pUnitsProp->getUnitSystem();
}

GraphicProperty* GraphicUnitsProperty::copy() const
{
   return new GraphicUnitsProperty(mUnits);
}

bool GraphicUnitsProperty::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement("UnitSystem"));
   GraphicProperty::toXml(pXml);
   switch (mUnits)
   {
      case UNIT_KM:
         pXml->addAttr("system", "m-km");
         break;
      case UNIT_KFT:
         pXml->addAttr("system", "f-kft");
         break;
      case UNIT_MI:
         pXml->addAttr("system", "f-mi");
         break;
      default:
         return false;
   }

   pXml->popAddPoint();
   return true;
}

bool GraphicUnitsProperty::fromXml(DOMNode* pDocument, unsigned int version)
{
   GraphicProperty::fromXml(pDocument, version);

   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   string system(A(elmnt->getAttribute(X("system"))));

   if (system == "m-km")
   {
      mUnits = UNIT_KM;
   }
   else if (system == "f-kft")
   {
      mUnits = UNIT_KFT;
   }
   else if (system == "f-mi")
   {
      mUnits = UNIT_MI;
   }
   else
   {
      return false;
   }

   return true;
}

bool GraphicUnitsProperty::set(const GraphicProperty* pProp)
{
   const GraphicUnitsProperty* pUnitsProp = dynamic_cast<const GraphicUnitsProperty*>(pProp);
   if (pUnitsProp == NULL)
   {
      return false;
   }

   mUnits = pUnitsProp->mUnits;
   return true;
}
