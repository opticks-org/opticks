/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LocatorImp.h"
#include "glCommon.h"
#include "PlotViewImp.h"

#include <sstream>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

LocatorImp::LocatorImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mMinX(0.0),
   mMinY(0.0),
   mMaxX(0.0),
   mMaxY(0.0),
   mLocation(LocationType()),
   mStyle(Locator::CROSSHAIR_LOCATOR),
   mColor(Qt::black),
   mLineWidth(1),
   mLineStyle(SOLID_LINE)
{
   if (pPlot != NULL)
   {
      connect(pPlot, SIGNAL(displayAreaChanged()), this, SLOT(updateExtents()));
   }

   connect(this, SIGNAL(locationChanged(const LocationType&)),
      this, SIGNAL(extentsChanged()));
}

LocatorImp::~LocatorImp()
{
}

LocatorImp& LocatorImp::operator= (const LocatorImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mMinX = object.mMinX;
      mMinY = object.mMinY;
      mMaxX = object.mMaxX;
      mMaxY = object.mMaxY;
      mLocation = object.mLocation;
      mStyle = object.mStyle;
      mColor = object.mColor;
      mLineWidth = object.mLineWidth;
      mLineStyle = object.mLineStyle;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& LocatorImp::getObjectType() const
{
   static string type("LocatorImp");
   return type;
}

bool LocatorImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Locator"))
   {
      return true;
   }

   return PlotObjectImp::isKindOf(className);
}

PlotObjectType LocatorImp::getType() const
{
   return LOCATOR;
}

void LocatorImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(getPlot());
   if (pPlot == NULL)
   {
      return;
   }

   // Set the draw properties
   glColor3ub(mColor.red(), mColor.green(), mColor.blue());
   glLineWidth(mLineWidth);

   if (mLineStyle != SOLID_LINE)
   {
      glEnable(GL_LINE_STIPPLE);

      if (mLineStyle == DASHED)
      {
         glLineStipple(3, 0x3f3f);
      }
      else if (mLineStyle == DOT)
      {
         glLineStipple(2, 0x1111);
      }
      else if (mLineStyle == DASH_DOT)
      {
         glLineStipple(2, 0x11ff);
      }
      else if (mLineStyle == DASH_DOT_DOT)
      {
         glLineStipple(2, 0x24ff);
      }
   }

   // Convert the location to world coordinates
   double worldX = 0.0;
   double worldY = 0.0;
   pPlot->translateDataToWorld(mLocation.mX, mLocation.mY, worldX, worldY);

   // Draw the horozontal line
   glBegin(GL_LINES);
   if ((mStyle == Locator::HORIZONTAL_LOCATOR) || (mStyle == Locator::CROSSHAIR_LOCATOR))
   {
      glVertex2d(mMinX, worldY);
      glVertex2d(mMaxX, worldY);
   }

   // Draw the vertical line
   if ((mStyle == Locator::VERTICAL_LOCATOR) || (mStyle == Locator::CROSSHAIR_LOCATOR))
   {
      glVertex2d(worldX, mMinY);
      glVertex2d(worldX, mMaxY);
   }

   glEnd();
   glLineWidth(1.0);

   if (mLineStyle != SOLID_LINE)
   {
      glDisable(GL_LINE_STIPPLE);
   }

   // Draw the location text
   if (mStyle == Locator::CROSSHAIR_LOCATOR)
   {
      glColor3ub(0, 0, 0);

      QString strLocation = QString::number(mLocation.mX) + ", " + QString::number(mLocation.mY);

      LocationType screenCoord;
      pPlot->translateWorldToScreen(worldX, worldY, screenCoord.mX, screenCoord.mY);

      int screenX = static_cast<int>(screenCoord.mX) + 3;
      int screenY = pPlot->height() - (static_cast<int>(screenCoord.mY) + 3);
      pPlot->renderText(screenX, screenY, strLocation);
   }
}

LocationType LocatorImp::getLocation() const
{
   return mLocation;
}

void LocatorImp::setLocation(const LocationType& location, bool updateText)
{
   if (location != mLocation)
   {
      mLocation = location;
      emit locationChanged(location);
      notify(SIGNAL_NAME(Locator, LocationChanged), boost::any(location));
   }

   if (updateText)
   {
      QString strTextX;
      if ((mStyle == Locator::VERTICAL_LOCATOR) || (mStyle == Locator::CROSSHAIR_LOCATOR))
      {
         strTextX = QString::number(location.mX);
      }

      QString strTextY;
      if ((mStyle == Locator::HORIZONTAL_LOCATOR) || (mStyle == Locator::CROSSHAIR_LOCATOR))
      {
         strTextY = QString::number(location.mY);
      }

      setText(strTextX, strTextY);
   }
}

void LocatorImp::setText(const QString& strTextX, const QString& strTextY)
{
   if ((strTextX != mTextX) || (strTextY != mTextY))
   {
      mTextX = strTextX;
      mTextY = strTextY;

      emit textChanged(mTextX, mTextY);
      notify(SIGNAL_NAME(Locator, TextChanged), boost::any(
         std::pair<std::string,std::string>(mTextX.toStdString(), mTextY.toStdString())));
   }
}

void LocatorImp::getText(QString& strTextX, QString& strTextY) const
{
   strTextX = mTextX;
   strTextY = mTextY;
}

void LocatorImp::setStyle(Locator::LocatorStyle style)
{
   if (style != mStyle)
   {
      mStyle = style;
      emit styleChanged(mStyle);
      notify(SIGNAL_NAME(Locator, StyleChanged), boost::any(mStyle));
   }
}

Locator::LocatorStyle LocatorImp::getStyle() const
{
   return mStyle;
}

void LocatorImp::setColor(const QColor& locatorColor)
{
   if (locatorColor.isValid() == false)
   {
      return;
   }

   if (locatorColor != mColor)
   {
      mColor = locatorColor;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

QColor LocatorImp::getColor() const
{
   return mColor;
}

void LocatorImp::setLineWidth(int iWidth)
{
   if (iWidth < 0)
   {
      iWidth = 0;
   }

   if (iWidth != mLineWidth)
   {
      mLineWidth = iWidth;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

int LocatorImp::getLineWidth() const
{
   return mLineWidth;
}

void LocatorImp::setLineStyle(LineStyle eStyle)
{
   if (eStyle != mLineStyle)
   {
      mLineStyle = eStyle;
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

LineStyle LocatorImp::getLineStyle() const
{
   return mLineStyle;
}

void LocatorImp::updateExtents()
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot != NULL)
   {
      // Get the plot extents in world coordinates
      pPlot->translateScreenToWorld(0, 0, mMinX, mMinY);
      pPlot->translateScreenToWorld(pPlot->width(), pPlot->height(), mMaxX, mMaxY);
   }
}

bool LocatorImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("lineWidth", mLineWidth);
   pXml->addAttr("lineStyle", mLineStyle);
   pXml->addAttr("lowerLeft", LocationType(mMinX, mMinY));
   pXml->addAttr("upperRight", LocationType(mMaxX, mMaxY));
   pXml->addAttr("locatorStyle", mStyle);
   pXml->addAttr("textX", mTextX.toStdString());
   pXml->addAttr("textY", mTextY.toStdString());
   pXml->addAttr("location", mLocation);

   return true;
}

bool LocatorImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   ColorType color = StringUtilities::fromXmlString<ColorType>(
      A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);
   mLineWidth = StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("lineWidth"))));
   mLineStyle = StringUtilities::fromXmlString<LineStyle>(
      A(pElem->getAttribute(X("lineStyle"))));
   LocationType ll = StringUtilities::fromXmlString<LocationType>(
      A(pElem->getAttribute(X("lowerLeft"))));
   mMinX = ll.mX;
   mMinY = ll.mY;
   LocationType ur = StringUtilities::fromXmlString<LocationType>(
      A(pElem->getAttribute(X("upperRight"))));
   mMaxX = ur.mX;
   mMaxY = ur.mY;
   mStyle = StringUtilities::fromXmlString<Locator::LocatorStyle>(
      A(pElem->getAttribute(X("locatorStyle"))));
   mTextX = A(pElem->getAttribute(X("textX")));
   mTextY = A(pElem->getAttribute(X("textY")));
   mLocation = StringUtilities::fromXmlString<LocationType>(
      A(pElem->getAttribute(X("location"))));

   return true;
}
