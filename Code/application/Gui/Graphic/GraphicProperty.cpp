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

////////////////////////
// GraphicProperty //
////////////////////////

GraphicProperty::GraphicProperty(const string& name)
{
   mName = name;
}

GraphicProperty::~GraphicProperty()
{
}

string GraphicProperty::getName() const
{
   return mName;
}

void GraphicProperty::setName(const std::string& name)
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

AlphaProperty::AlphaProperty(double alpha)
   : GraphicProperty("Alpha")
{
   mAlpha = alpha;
}

bool AlphaProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Alpha") return false;
   return mAlpha == ((AlphaProperty*)pProp)->getAlpha();
}

GraphicProperty* AlphaProperty::copy() const
{
   return new AlphaProperty(mAlpha);
}

double AlphaProperty::getAlpha() const
{
   return mAlpha;
}

bool AlphaProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Alpha"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mAlpha;
   xml->addAttr("alpha", buf.str());
   xml->popAddPoint();

   return true;
}

bool AlphaProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   stringstream buf(A(elmnt->getAttribute(X("alpha"))));
   buf >> mAlpha;

   return true;
}

//////////////////
// ApexProperty //
//////////////////

ApexProperty::ApexProperty(double apex)
   : GraphicProperty("Apex")
{
   mApexPosition = apex;
}

bool ApexProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Apex") return false;
   return mApexPosition == ((ApexProperty*)pProp)->getApex();
}

GraphicProperty* ApexProperty::copy() const
{
   return new ApexProperty(mApexPosition);
}

double ApexProperty::getApex() const
{
   return mApexPosition;
}

bool ApexProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Apex"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mApexPosition;
   xml->addAttr("position", buf.str());
   xml->popAddPoint();

   return true;
}

bool ApexProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   stringstream buf(A(elmnt->getAttribute(X("position"))));
   buf >> mApexPosition;

   return true;
}

///////////////////////
// ArcRegionProperty //
///////////////////////

ArcRegionProperty::ArcRegionProperty(ArcRegion eRegion)
   : GraphicProperty("ArcRegion")
{
   mRegion = eRegion;
}

bool ArcRegionProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "ArcRegion")
   {
      return false;
   }

   return mRegion == ((ArcRegionProperty*) pProp)->getRegion();
}

GraphicProperty* ArcRegionProperty::copy() const
{
   return new ArcRegionProperty(mRegion);
}

ArcRegion ArcRegionProperty::getRegion() const
{
   return mRegion;
}

bool ArcRegionProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("ArcRegion"));
   GraphicProperty::toXml(xml);
   switch(mRegion)
   {
      case ARC_CENTER:
         xml->addAttr("region", "center");
         break;
      case ARC_CHORD:
         xml->addAttr("region", "chord");
         break;
      case ARC_OPEN:
         xml->addAttr("region", "open");
         break;
   }
   xml->popAddPoint();

   return true;
}

bool ArcRegionProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   string buf(A(elmnt->getAttribute(X("region"))));

   if(buf == "center")
      mRegion = ARC_CENTER;
   else if(buf == "chord")
      mRegion = ARC_CHORD;
   else if(buf == "open")
      mRegion = ARC_OPEN;

   return true;
}

/////////////////////////
// BoundingBoxProperty //
/////////////////////////

BoundingBoxProperty::BoundingBoxProperty(LocationType llCorner, LocationType urCorner, bool geoCoords)
   : GraphicProperty("BoundingBox")
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
}

BoundingBoxProperty::BoundingBoxProperty(LocationType llCorner, LocationType urCorner, LocationType llLatLong,
   LocationType urLatLong) : GraphicProperty("BoundingBox")
{
   mLlCorner = llCorner;
   mUrCorner = urCorner;
   mLlLatLong = llLatLong;
   mUrLatLong = urLatLong;
   mHasGeoCoords = true;
   mHasPixelCoords = true;
}

bool BoundingBoxProperty::hasGeoCoords() const
{
   return mHasGeoCoords;
}

bool BoundingBoxProperty::hasPixelCoords() const
{
   return mHasPixelCoords;
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

bool BoundingBoxProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "BoundingBox") return false;
   LocationType llCorner = ((BoundingBoxProperty*)pProp)->getLlCorner();
   LocationType urCorner = ((BoundingBoxProperty*)pProp)->getUrCorner();
   LocationType llLatLong = ((BoundingBoxProperty*)pProp)->getLlLatLong();
   LocationType urLatLong = ((BoundingBoxProperty*)pProp)->getUrLatLong();

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
   return new BoundingBoxProperty(mLlCorner, mUrCorner, mLlLatLong, mUrLatLong);
}

bool BoundingBoxProperty::toXml(XMLWriter* xml) const
{
   if (xml == NULL)
   {
      return false;
   }

   xml->pushAddPoint(xml->addElement("BoundingBox"));
   GraphicProperty::toXml(xml);

   if (mHasPixelCoords)
   {
      stringstream buf;
      buf.precision(12);
      buf << mLlCorner.mX << ' ' << mLlCorner.mY << ' ' << mUrCorner.mX << ' ' << mUrCorner.mY;
      xml->addText(buf.str(), xml->addElement("boundingBox"));
   }
   if (mHasGeoCoords)
   {
      stringstream buf;
      buf.precision(12);
      buf << mLlLatLong.mX << ' ' << mLlLatLong.mY << ' ' << mUrLatLong.mX << ' ' << mUrLatLong.mY;
      xml->addText(buf.str(), xml->addElement("geoBox"));
   }

   xml->popAddPoint();
   return true;
}

bool BoundingBoxProperty::fromXml(DOMNode* document, unsigned int version)
{
   if (document == NULL)
   {
      return false;
   }

   mLlCorner = LocationType();
   mUrCorner = LocationType();
   mLlLatLong = LocationType();
   mUrLatLong = LocationType();
   mHasGeoCoords = false;
   mHasPixelCoords = false;

   GraphicProperty::fromXml(document, version);

   for (DOMNode* chld = document->getFirstChild(); chld != NULL; chld = chld->getNextSibling())
   {
      if (XMLString::equals(chld->getNodeName(), X("boundingBox")))
      {
         XmlReader::StrToQuadCoord(chld->getTextContent(), mLlCorner.mX, mLlCorner.mY,
            mUrCorner.mX, mUrCorner.mY);
         mHasPixelCoords = true;
      }
      else if (XMLString::equals(chld->getNodeName(), X("geoBox")))
      {
         XmlReader::StrToQuadCoord(chld->getTextContent(), mLlLatLong.mX, mLlLatLong.mY,
            mUrLatLong.mX, mUrLatLong.mY);
         mHasGeoCoords = true;
      }
   }

   return true;
}

///////////////////////
// FillColorProperty //
///////////////////////

FillColorProperty::FillColorProperty(ColorType color)
   : GraphicProperty("FillColor")
{
   mColor = color;
}

bool FillColorProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "FillColor") return false;
   return mColor == ((FillColorProperty*)pProp)->getColor();
}

GraphicProperty* FillColorProperty::copy() const
{
   return new FillColorProperty(mColor);
}

ColorType FillColorProperty::getColor() const
{
   return mColor;
}

bool FillColorProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("FillColor"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   xml->addText(buf.str(), xml->addElement("color"));
   xml->popAddPoint();

   return true;
}

bool FillColorProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(
                                                                     X("color")));
   double r,g,b,dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r,g,b);

   return true;
}

///////////////////////
// FillStyleProperty //
///////////////////////

FillStyleProperty::FillStyleProperty(FillStyle eStyle)
   : GraphicProperty("FillStyle")
{
   mStyle = eStyle;
}

bool FillStyleProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "FillStyle") return false;
   return mStyle == ((FillStyleProperty*)pProp)->getFillStyle();
}

GraphicProperty* FillStyleProperty::copy() const
{
   return new FillStyleProperty(mStyle);
}

FillStyle FillStyleProperty::getFillStyle() const
{
   return mStyle;
}

bool FillStyleProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("FillStyle"));
   GraphicProperty::toXml(xml);
   switch(mStyle)
   {
      case SOLID_FILL:
         xml->addAttr("style", "solid");
         break;
      case HATCH:
         xml->addAttr("style", "hatch");
         break;
      case EMPTY_FILL:
         xml->addAttr("style", "empty");
         break;
   }
   xml->popAddPoint();

   return true;
}

bool FillStyleProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
   string style(A(elmnt->getAttribute(X("style"))));

   if(style == "solid")
      mStyle = SOLID_FILL;
   else if(style == "hatch")
      mStyle = HATCH;
   else if(style == "empty")
      mStyle = EMPTY_FILL;

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

bool FontProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Font") return false;
   return mFont == ((FontProperty*)pProp)->getFont();
}

GraphicProperty* FontProperty::copy() const
{
   return new FontProperty(mFont);
}

const FontImp& FontProperty::getFont() const
{
   return mFont;
}

bool FontProperty::toXml(XMLWriter* xml) const
{
   const QFont& font = mFont.getQFont();

   xml->pushAddPoint(xml->addElement("Font"));
   GraphicProperty::toXml(xml);

   xml->addAttr("face", font.family().toStdString());

   stringstream buf;
   buf << font.pointSize();
   xml->addAttr("size", buf.str());
   xml->addAttr("bold", (font.bold()) ? "true" : "false");
   xml->addAttr("italic", (font.italic()) ? "true" : "false");
   xml->addAttr("underline", (font.underline()) ? "true" : "false");
   xml->popAddPoint();

   return true;
}

bool FontProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));

   string face = GraphicLayer::getSettingTextFont();
   int iSize = GraphicLayer::getSettingTextFontSize();
   bool bBold = GraphicLayer::getSettingTextBold();
   bool bItalic = GraphicLayer::getSettingTextItalics();
   bool bUnderline = GraphicLayer::getSettingTextUnderline();

   if(elmnt->hasAttribute(X("face")))
      face = A(elmnt->getAttribute(X("face")));
   if(elmnt->hasAttribute(X("size")))
      iSize = atoi(A(elmnt->getAttribute(X("size"))));
   if(elmnt->hasAttribute(X("bold")))
   {
      string tmp(A(elmnt->getAttribute(X("bold"))));
      if(tmp == "0" || tmp == "f" || tmp == "false")
         bBold = false;
      else
         bBold = true;
   }
   if(elmnt->hasAttribute(X("italic")))
   {
      string tmp(A(elmnt->getAttribute(X("italic"))));
      if(tmp == "0" || tmp == "f" || tmp == "false")
         bItalic = false;
      else
         bItalic = true;
   }
   if(elmnt->hasAttribute(X("underline")))
   {
      string tmp(A(elmnt->getAttribute(X("underline"))));
      if(tmp == "0" || tmp == "f" || tmp == "false")
         bUnderline = false;
      else
         bUnderline = true;
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

HatchStyleProperty::HatchStyleProperty(SymbolType eHatch)
   : GraphicProperty("HatchStyle")
{
   mStyle = eHatch;
}

bool HatchStyleProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "HatchStyle") return false;
   return mStyle == ((HatchStyleProperty*)pProp)->getHatchStyle();
}

GraphicProperty* HatchStyleProperty::copy() const
{
   return new HatchStyleProperty(mStyle);
}

SymbolType HatchStyleProperty::getHatchStyle() const
{
   return mStyle;
}

bool HatchStyleProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("HatchStyle"));
   GraphicProperty::toXml(xml);
   switch(mStyle)
   {
      case SOLID:
         xml->addAttr("boxed","false");
         xml->addAttr("style","solid");
         break;
      case X:
         xml->addAttr("boxed","false");
         xml->addAttr("style","x");
         break;
      case CROSS_HAIR:
         xml->addAttr("boxed","false");
         xml->addAttr("style","cross hair");
         break;
      case ASTERISK:
         xml->addAttr("boxed","false");
         xml->addAttr("style","asterisk");
         break;
      case HORIZONTAL_LINE:
         xml->addAttr("boxed","false");
         xml->addAttr("style","horizontal line");
         break;
      case VERTICAL_LINE:
         xml->addAttr("boxed","false");
         xml->addAttr("style","vertical line");
         break;
      case FORWARD_SLASH:
         xml->addAttr("boxed","false");
         xml->addAttr("style","forward slash");
         break;
      case BACK_SLASH:
         xml->addAttr("boxed","false");
         xml->addAttr("style","back slash");
         break;
      case BOX:
         xml->addAttr("boxed","true");
         xml->addAttr("style","solid");
         break;
      case BOXED_X:
         xml->addAttr("boxed","true");
         xml->addAttr("style","x");
         break;
      case BOXED_CROSS_HAIR:
         xml->addAttr("boxed","true");
         xml->addAttr("style","cross hair");
         break;
      case BOXED_ASTERISK:
         xml->addAttr("boxed","true");
         xml->addAttr("style","asterisk");
         break;
      case BOXED_HORIZONTAL_LINE:
         xml->addAttr("boxed","true");
         xml->addAttr("style","horizontal line");
         break;
      case BOXED_VERTICAL_LINE:
         xml->addAttr("boxed","true");
         xml->addAttr("style","vertical line");
         break;
      case BOXED_FORWARD_SLASH:
         xml->addAttr("boxed","true");
         xml->addAttr("style","forward slash");
         break;
      case BOXED_BACK_SLASH:
         xml->addAttr("boxed","true");
         xml->addAttr("style","back slash");
         break;
   }
   xml->popAddPoint();

   return true;
}

bool HatchStyleProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
   string style(A(elmnt->getAttribute(X("style"))));
   bool boxed(true);
   {
      string boxedS(A(elmnt->getAttribute(X("boxed"))));
      if(boxedS == "0" || boxedS == "f" || boxedS == "false")
         boxed = false;
   }

   if(style == "solid" && !boxed)
      mStyle = SOLID;
   else if(style == "x" && !boxed)
      mStyle = X;
   else if(style == "cross hair" && !boxed)
      mStyle = CROSS_HAIR;
   else if(style == "asterisk" && !boxed)
      mStyle = ASTERISK;
   else if(style == "horizontal line" && !boxed)
      mStyle = HORIZONTAL_LINE;
   else if(style == "vertical line" && !boxed)
      mStyle = VERTICAL_LINE;
   else if(style == "forward slash" && !boxed)
      mStyle = FORWARD_SLASH;
   else if(style == "back slash" && !boxed)
      mStyle = BACK_SLASH;
   else if(style == "solid" && boxed)
      mStyle = BOX;
   else if(style == "x" && boxed)
      mStyle = BOXED_X;
   else if(style == "cross hair" && boxed)
      mStyle = BOXED_CROSS_HAIR;
   else if(style == "asterisk" && boxed)
      mStyle = BOXED_ASTERISK;
   else if(style == "horizontal line" && boxed)
      mStyle = BOXED_HORIZONTAL_LINE;
   else if(style == "vertical line" && boxed)
      mStyle = BOXED_VERTICAL_LINE;
   else if(style == "forward slash" && boxed)
      mStyle = BOXED_FORWARD_SLASH;
   else if(style == "back slash" && boxed)
      mStyle = BOXED_BACK_SLASH;

   return true;
}

////////////////////
// LatLonProperty //
////////////////////

bool LatLonProperty::set(const GraphicProperty *pProp)
{
   *this = *(LatLonProperty*)pProp;
   return true;
}

bool LatLonProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "LatLon") return false;
   return getLatLon() == ((LatLonProperty*)pProp)->getLatLon();
}

GraphicProperty* LatLonProperty::copy() const
{
   return new LatLonProperty(mLatLon);
}

bool LatLonProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LatLon"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << getLatLon().getLatitude().getValue() << ' '
       << getLatLon().getLongitude().getValue();
   xml->addText(buf.str(), xml->addElement("point"));
   xml->popAddPoint();

   return true;
}

bool LatLonProperty::fromXml(DOMNode *document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(X("point")));
   XMLStringTokenizer t(elmnts->item(0)->getTextContent());
   string lat,lon;
   if(t.hasMoreTokens())
      lat = A(t.nextToken());
   if(t.hasMoreTokens())
      lon = A(t.nextToken());
   mLatLon = LatLonPoint(lat, lon);

   return true;
}

///////////////////////
// LineColorProperty //
///////////////////////

LineColorProperty::LineColorProperty(ColorType color)
   : GraphicProperty("LineColor")
{
   mColor = color;
}

bool LineColorProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "LineColor") return false;
   return mColor == ((LineColorProperty*)pProp)->getColor();
}

GraphicProperty* LineColorProperty::copy() const
{
   return new LineColorProperty(mColor);
}

ColorType LineColorProperty::getColor() const
{
   return mColor;
}

bool LineColorProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LineColor"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   xml->addText(buf.str(), xml->addElement("color"));
   xml->popAddPoint();

   return true;
}

bool LineColorProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(X("color")));
   double r,g,b,dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r,g,b);

   return true;
}

////////////////////
// LineOnProperty //
////////////////////

LineOnProperty::LineOnProperty(bool state)
   : GraphicProperty("LineOn")
{
   mState = state;
}

bool LineOnProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "LineOn") return false;
   return mState == ((LineOnProperty*)pProp)->getState();
}

GraphicProperty* LineOnProperty::copy() const
{
   return new LineOnProperty(mState);
}

bool LineOnProperty::getState() const
{
   return mState;
}

bool LineOnProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LineOn"));
   GraphicProperty::toXml(xml);
   xml->addAttr("on", (mState) ? "true" : "false");
   xml->popAddPoint();

   return true;
}

bool LineOnProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   string v(A(static_cast<DOMElement*>(document)->getAttribute(X("on"))));
   if(v == "0" || v == "f" || v == "false")
      mState = false;
   else
      mState = true;

   return true;
}

////////////////////
// LineScaledProperty //
////////////////////

LineScaledProperty::LineScaledProperty(bool scaled)
   : GraphicProperty("LineScaled"), mScaled(scaled)
{
}

bool LineScaledProperty::set(const GraphicProperty *pProp)
{
   const LineScaledProperty *pScaledProp = 
      dynamic_cast<const LineScaledProperty*>(pProp);
   if (pScaledProp != NULL)
   {
      mScaled = pScaledProp->getScaled();
      return true;
   }
   return false;
}

bool LineScaledProperty::compare(const GraphicProperty* pProp) const
{
   const LineScaledProperty *pScaledProp = 
      dynamic_cast<const LineScaledProperty*>(pProp);
   if (pScaledProp != NULL)
   {
      return mScaled == pScaledProp->getScaled();
   }
   return false;
}

GraphicProperty* LineScaledProperty::copy() const
{
   return new LineScaledProperty(mScaled);
}

bool LineScaledProperty::getScaled() const
{
   return mScaled;
}

bool LineScaledProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LineScaled"));
   GraphicProperty::toXml(xml);
   xml->addAttr("scaled", (mScaled) ? "true" : "false");
   xml->popAddPoint();

   return true;
}

bool LineScaledProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   string v(A(static_cast<DOMElement*>(document)->getAttribute(X("scaled"))));
   if(v == "0" || v == "f" || v == "false")
      mScaled = false;
   else
      mScaled = true;

   return true;
}

///////////////////////
// LineStyleProperty //
///////////////////////

LineStyleProperty::LineStyleProperty(LineStyle eStyle)
   : GraphicProperty("LineStyle")
{
   mStyle = eStyle;
}

bool LineStyleProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "LineStyle") return false;
   return mStyle == ((LineStyleProperty*)pProp)->getStyle();
}

GraphicProperty* LineStyleProperty::copy() const
{
   return new LineStyleProperty(mStyle);
}

LineStyle LineStyleProperty::getStyle() const
{
   return mStyle;
}

bool LineStyleProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LineStyle"));
   GraphicProperty::toXml(xml);
   switch(mStyle)
   {
      case SOLID_LINE:
         xml->addAttr("style", "solid");
         break;
      case DASHED:
         xml->addAttr("style", "dashed");
         break;
      case DOT:
         xml->addAttr("style", "dot");
         break;
      case DASH_DOT:
         xml->addAttr("style", "dash dot");
         break;
      case DASH_DOT_DOT:
         xml->addAttr("style", "dash dot dot");
         break;
   }
   xml->popAddPoint();

   return true;
}

bool LineStyleProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   string style(A(static_cast<DOMElement*>(document)->getAttribute(X("style"))));
   
   if(style == "solid")
      mStyle = SOLID_LINE;
   else if(style == "dashed")
      mStyle = DASHED;
   else if(style == "dot")
      mStyle = DOT;
   else if(style == "dash dot")
      mStyle = DASH_DOT;
   else if(style == "dash dot dot")
      mStyle = DASH_DOT_DOT;

   return true;
}

///////////////////////
// LineWidthProperty //
///////////////////////

LineWidthProperty::LineWidthProperty(double width)
   : GraphicProperty("LineWidth")
{
   mWidth = width;
}

bool LineWidthProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "LineWidth") return false;
   return mWidth == ((LineWidthProperty*)pProp)->getWidth();
}

GraphicProperty* LineWidthProperty::copy() const
{
   return new LineWidthProperty(mWidth);
}

double LineWidthProperty::getWidth() const
{
   return mWidth;
}

bool LineWidthProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("LineWidth"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mWidth;
   xml->addAttr("width", buf.str());
   xml->popAddPoint();

   return true;
}

bool LineWidthProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   stringstream buf(A(elmnt->getAttribute(X("width"))));
   buf >> mWidth;

   return true;
}

//////////////////////
// RotationProperty //
//////////////////////

RotationProperty::RotationProperty(double rotate)
   : GraphicProperty("Rotation")
{
   mRotate = rotate;
}

bool RotationProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Rotation") return false;
   return mRotate == ((RotationProperty*)pProp)->getRotation();
}

GraphicProperty* RotationProperty::copy() const
{
   return new RotationProperty(mRotate);
}

double RotationProperty::getRotation() const
{
   return mRotate;
}

bool RotationProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Rotation"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mRotate;
   xml->addAttr("rotation", buf.str());
   xml->popAddPoint();

   return true;
}

bool RotationProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   stringstream buf(A(elmnt->getAttribute(X("rotation"))));
   buf >> mRotate;

   return true;
}

///////////////////
// ScaleProperty //
///////////////////

ScaleProperty::ScaleProperty(double scale)
   : GraphicProperty("Scale")
{
   mScale = scale;
}

bool ScaleProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Scale") return false;
   return mScale == ((ScaleProperty*)pProp)->getScale();
}

GraphicProperty* ScaleProperty::copy() const
{
   return new ScaleProperty(mScale);
}

double ScaleProperty::getScale() const
{
   return mScale;
}

bool ScaleProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Scale"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mScale;
   xml->addAttr("scale", buf.str());
   xml->popAddPoint();

   return true;
}

bool ScaleProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   stringstream buf(A(elmnt->getAttribute(X("scale"))));
   buf >> mScale;

   return true;
}

///////////////////////////
// TextAlignmentProperty //
///////////////////////////

TextAlignmentProperty::TextAlignmentProperty(int iAlignment)
   : GraphicProperty("TextAlignment")
{
   mAlignment = iAlignment;
}

int TextAlignmentProperty::getAlignment() const
{
   return mAlignment;
}

bool TextAlignmentProperty::set(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   if (pProperty->getName() != "TextAlignment")
   {
      return false;
   }

   const TextAlignmentProperty* pTextProperty = static_cast<const TextAlignmentProperty*> (pProperty);
   *this = *pTextProperty;

   return true;
}

bool TextAlignmentProperty::compare(const GraphicProperty* pProperty) const
{
   if (pProperty == NULL)
   {
      return false;
   }

   if (pProperty->getName() != "TextAlignment")
   {
      return false;
   }

   const TextAlignmentProperty* pTextProperty = static_cast<const TextAlignmentProperty*> (pProperty);
   int iAlignment = pTextProperty->getAlignment();

   return (iAlignment == mAlignment);
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

TextColorProperty::TextColorProperty(ColorType color)
   : GraphicProperty("TextColor")
{
   mColor = color;
}

bool TextColorProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "TextColor") return false;
   return mColor == ((TextColorProperty*)pProp)->getColor();
}

GraphicProperty* TextColorProperty::copy() const
{
   return new TextColorProperty(mColor);
}

ColorType TextColorProperty::getColor() const
{
   return mColor;
}

bool TextColorProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("TextColor"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mColor.mRed << ' ' << mColor.mGreen << ' ' << mColor.mBlue;
   xml->addText(buf.str(), xml->addElement("color"));
   xml->popAddPoint();

   return true;
}

bool TextColorProperty::fromXml(DOMNode* document, unsigned int version)
{
   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(X("color")));
   double r,g,b,dummy;

   XmlReader::StrToQuadCoord(elmnts->item(0)->getTextContent(), r, g, b, dummy);
   mColor = ColorType(r,g,b);
   return true;
}

////////////////////////
// TextStringProperty //
////////////////////////

TextStringProperty::TextStringProperty(const string& text)
   : GraphicProperty("TextString")
{
   mString = text.c_str();
}

bool TextStringProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "TextString") return false;
   return mString == ((TextStringProperty*)pProp)->getString();
}

GraphicProperty* TextStringProperty::copy() const
{
   return new TextStringProperty(mString);
}

const string& TextStringProperty::getString() const
{
   return mString;
}

static bool endsWith(const char *pText, const char *pEnding)
{
   unsigned int textLen = strlen(pText);
   unsigned int endingLen = strlen(pEnding);
   if (textLen < endingLen) return false;
   return strcmp(&pText[textLen - endingLen], pEnding) == 0;
}

bool TextStringProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("TextString"));
   GraphicProperty::toXml(xml);
   xml->addText(mString, xml->addElement("string"));
   xml->popAddPoint();

   return true;
}

bool TextStringProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(X("string")));
   mString = A(elmnts->item(0)->getTextContent());

   return true;
}

///////////////////
// WedgeProperty //
///////////////////

WedgeProperty::WedgeProperty(double dStartAngle, double dStopAngle)
   : GraphicProperty("Wedge")
{
   mStartAngle = dStartAngle;
   mStopAngle = dStopAngle;
   normalizeAngles();
}

bool WedgeProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Wedge") return false;
   double start = ((WedgeProperty*)pProp)->getStartAngle();
   double stop = ((WedgeProperty*)pProp)->getStartAngle();
   return mStartAngle == start && mStopAngle == stop;
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

bool WedgeProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Wedge"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mStartAngle;
   xml->addAttr("start", buf.str());

   buf.str("");
   buf << mStopAngle;
   xml->addAttr("stop", buf.str());
   xml->popAddPoint();

   return true;
}

bool WedgeProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);
   DOMElement *elmnt(static_cast<DOMElement*>(document));
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

PaperSizeProperty::PaperSizeProperty(LocationType size)
   : GraphicProperty("PaperSize"),
   mSize(size)
{
}

bool PaperSizeProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "PaperSize") return false;
   return mSize == ((PaperSizeProperty*)pProp)->getSize();
}

GraphicProperty* PaperSizeProperty::copy() const
{
   return new PaperSizeProperty(mSize);
}

LocationType PaperSizeProperty::getSize() const
{
   return mSize;
}

bool PaperSizeProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("PaperSize"));
   GraphicProperty::toXml(xml);
   stringstream buf;
   buf << mSize.mX << ' ' << mSize.mY;
   xml->addText(buf.str(), xml->addElement("size"));
   xml->popAddPoint();

   return true;
}

bool PaperSizeProperty::fromXml(DOMNode *document, unsigned int version)
{
   DOMNodeList *elmnts(static_cast<DOMElement*>(document)->getElementsByTagName(X("size")));
   return XmlReader::StrToLocation(elmnts->item(0)->getTextContent(), mSize);
}

///////////////////////
// FileName Property //
///////////////////////

FileNameProperty::FileNameProperty(const string &filename) :
   GraphicProperty("Filename"),
   mFileName(filename.c_str())
{
}

bool FileNameProperty::set(const GraphicProperty *pProp)
{
   *this = *(FileNameProperty*)pProp;
   return true;
}

bool FileNameProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "Filename")
   {
      return false;
   }

   return ((FileNameProperty*) pProp)->getFileName() == getFileName();
}

GraphicProperty* FileNameProperty::copy() const
{
   return new FileNameProperty(mFileName);
}

bool FileNameProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("Filename"));
   GraphicProperty::toXml(xml);
   xml->addAttr("source", XmlBase::PathToURL(getFileName()));
   xml->popAddPoint();

   return true;
}

bool FileNameProperty::fromXml(DOMNode* document, unsigned int version)
{
   bool bSuccess = GraphicProperty::fromXml(document, version);
   if (bSuccess == true)
   {
      mFileName = XmlBase::URLtoPath(static_cast<DOMElement*>(document)->getAttribute(X("source")));
   }

   return bSuccess;
}

/////////////////////
// Symbol Property //
/////////////////////
PixelSymbolProperty::PixelSymbolProperty(SymbolType symbol)
   : GraphicProperty("PixelSymbol"), mSymbol(symbol)
{
}

bool PixelSymbolProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "PixelSymbol") return false;
   return mSymbol == static_cast<const PixelSymbolProperty*>(pProp)->getPixelSymbol();
}

GraphicProperty* PixelSymbolProperty::copy() const
{
   return new PixelSymbolProperty(mSymbol);
}

bool PixelSymbolProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("PixelSymbol"));
   GraphicProperty::toXml(xml);
   switch(mSymbol)
   {
      case SOLID:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","solid");
         break;
      case X:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","x");
         break;
      case CROSS_HAIR:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","cross hair");
         break;
      case ASTERISK:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","asterisk");
         break;
      case HORIZONTAL_LINE:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","horizontal line");
         break;
      case VERTICAL_LINE:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","vertical line");
         break;
      case FORWARD_SLASH:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","forward slash");
         break;
      case BACK_SLASH:
         xml->addAttr("boxed","false");
         xml->addAttr("symbol","back slash");
         break;
      case BOX:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","solid");
         break;
      case BOXED_X:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","x");
         break;
      case BOXED_CROSS_HAIR:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","cross hair");
         break;
      case BOXED_ASTERISK:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","asterisk");
         break;
      case BOXED_HORIZONTAL_LINE:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","horizontal line");
         break;
      case BOXED_VERTICAL_LINE:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","vertical line");
         break;
      case BOXED_FORWARD_SLASH:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","forward slash");
         break;
      case BOXED_BACK_SLASH:
         xml->addAttr("boxed","true");
         xml->addAttr("symbol","back slash");
         break;
      default:
         break;
   }
   xml->popAddPoint();

   return true;
}

bool PixelSymbolProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
   string symbol(A(elmnt->getAttribute(X("symbol"))));
   bool boxed(true);
   {
      string boxedS(A(elmnt->getAttribute(X("boxed"))));
      if(boxedS == "0" || boxedS == "f" || boxedS == "false")
         boxed = false;
   }

   if(symbol == "solid" && !boxed)
   {
      mSymbol = SOLID;
   }
   else if(symbol == "x" && !boxed)
   {
      mSymbol = X;
   }
   else if(symbol == "cross hair" && !boxed)
   {
      mSymbol = CROSS_HAIR;
   }
   else if(symbol == "asterisk" && !boxed)
   {
      mSymbol = ASTERISK;
   }
   else if(symbol == "horizontal line" && !boxed)
   {
      mSymbol = HORIZONTAL_LINE;
   }
   else if(symbol == "vertical line" && !boxed)
   {
      mSymbol = VERTICAL_LINE;
   }
   else if(symbol == "forward slash" && !boxed)
   {
      mSymbol = FORWARD_SLASH;
   }
   else if(symbol == "back slash" && !boxed)
   {
      mSymbol = BACK_SLASH;
   }
   else if(symbol == "solid" && boxed)
   {
      mSymbol = BOX;
   }
   else if(symbol == "x" && boxed)
   {
      mSymbol = BOXED_X;
   }
   else if(symbol == "cross hair" && boxed)
   {
      mSymbol = BOXED_CROSS_HAIR;
   }
   else if(symbol == "asterisk" && boxed)
   {
      mSymbol = BOXED_ASTERISK;
   }
   else if(symbol == "horizontal line" && boxed)
   {
      mSymbol = BOXED_HORIZONTAL_LINE;
   }
   else if(symbol == "vertical line" && boxed)
   {
      mSymbol = BOXED_VERTICAL_LINE;
   }
   else if(symbol == "forward slash" && boxed)
   {
      mSymbol = BOXED_FORWARD_SLASH;
   }
   else if(symbol == "back slash" && boxed)
   {
      mSymbol = BOXED_BACK_SLASH;
   }

   return true;
}

bool PixelSymbolProperty::set(const GraphicProperty *pProp)
{
   const PixelSymbolProperty *pSymbolProp = dynamic_cast<const PixelSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }
   mSymbol = pSymbolProp->mSymbol;
   return true;
}

////////////////////////
// Draw Mode Property //
////////////////////////
DrawModeProperty::DrawModeProperty(ModeType mode)
   : GraphicProperty("DrawMode"), mMode(mode)
{
}

bool DrawModeProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != "DrawMode") return false;
   return mMode == static_cast<const DrawModeProperty*>(pProp)->getDrawMode();
}

GraphicProperty* DrawModeProperty::copy() const
{
   return new DrawModeProperty(mMode);
}

bool DrawModeProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("DrawMode"));
   GraphicProperty::toXml(xml);
   switch(mMode)
   {
      case DRAW:
         xml->addAttr("mode", "draw");
         break;
      case ERASE:
         xml->addAttr("mode", "erase");
         break;
      case TOGGLE:
         xml->addAttr("mode", "toggle");
         break;
      default:
         break;
   }
   xml->popAddPoint();

   return true;
}

bool DrawModeProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
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
   const DrawModeProperty *pModeProp = dynamic_cast<const DrawModeProperty*>(pProp);
   if (pModeProp == NULL)
   {
      return false;
   }
   mMode = pModeProp->mMode;
   return true;
}

////////////////////////
// Graphic Symbol Property //
////////////////////////
GraphicSymbolProperty::GraphicSymbolProperty(const std::string &symbolName)
   : GraphicProperty("GraphicSymbol"), mSymbolName(symbolName)
{
}

bool GraphicSymbolProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != getName()) return false;
   return mSymbolName == static_cast<const GraphicSymbolProperty*>(pProp)->getSymbolName();
}

GraphicProperty* GraphicSymbolProperty::copy() const
{
   return new GraphicSymbolProperty(mSymbolName);
}

bool GraphicSymbolProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("GraphicSymbol"));
   GraphicProperty::toXml(xml);
   xml->addAttr("name", mSymbolName);
   xml->popAddPoint();

   return true;
}

bool GraphicSymbolProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
   mSymbolName = A(elmnt->getAttribute(X("name")));

   return true;
}

bool GraphicSymbolProperty::set(const GraphicProperty *pProp)
{
   const GraphicSymbolProperty *pSymbolProp = dynamic_cast<const GraphicSymbolProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }
   mSymbolName = pSymbolProp->mSymbolName;
   return true;
}

////////////////////////
// Graphic Symbol Size Property //
////////////////////////
GraphicSymbolSizeProperty::GraphicSymbolSizeProperty(unsigned int symbolSize)
   : GraphicProperty("GraphicSymbolSize"), mSymbolSize(symbolSize)
{
}

bool GraphicSymbolSizeProperty::compare(const GraphicProperty* pProp) const
{
   if (pProp->getName() != getName()) return false;
   return mSymbolSize == static_cast<const GraphicSymbolSizeProperty*>(pProp)->getSymbolSize();
}

GraphicProperty* GraphicSymbolSizeProperty::copy() const
{
   return new GraphicSymbolSizeProperty(mSymbolSize);
}

bool GraphicSymbolSizeProperty::toXml(XMLWriter* xml) const
{
   xml->pushAddPoint(xml->addElement("GraphicSymbolSize"));
   GraphicProperty::toXml(xml);
   xml->addAttr("size", mSymbolSize);
   xml->popAddPoint();

   return true;
}

bool GraphicSymbolSizeProperty::fromXml(DOMNode* document, unsigned int version)
{
   GraphicProperty::fromXml(document, version);

   DOMElement *elmnt(static_cast<DOMElement*>(document));
   string symbolSize = A(elmnt->getAttribute(X("size")));
   mSymbolSize = boost::lexical_cast<unsigned int>(symbolSize);

   return true;
}

bool GraphicSymbolSizeProperty::set(const GraphicProperty *pProp)
{
   const GraphicSymbolSizeProperty *pSymbolProp = dynamic_cast<const GraphicSymbolSizeProperty*>(pProp);
   if (pSymbolProp == NULL)
   {
      return false;
   }
   mSymbolSize = pSymbolProp->mSymbolSize;
   return true;
}
