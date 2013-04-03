/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QPainter>

#include "glCommon.h"
#include "Text.h"
#include "TextImp.h"
#include "PlotView.h"
#include "PlotViewImp.h"

#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

TextImp::TextImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(LocationType()),
   mText(QString()),
   mFont(QApplication::font()),
   mColor(Qt::black)
{
   connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(textChanged(const QString&)), this, SIGNAL(extentsChanged()));
}

TextImp::TextImp(PlotViewImp* pPlot, bool bPrimary, const LocationType& point) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(point),
   mText(QString()),
   mFont(QApplication::font()),
   mColor(Qt::black)
{
   connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(textChanged(const QString&)), this, SIGNAL(extentsChanged()));
}

TextImp::TextImp(PlotViewImp* pPlot, bool bPrimary, double dX, double dY) :
   PlotObjectImp(pPlot, bPrimary),
   mLocation(LocationType(dX, dY)),
   mText(QString()),
   mFont(QApplication::font()),
   mColor(Qt::black)
{
   connect(this, SIGNAL(locationChanged(const LocationType&)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(textChanged(const QString&)), this, SIGNAL(extentsChanged()));
}

TextImp::~TextImp()
{
}

TextImp& TextImp::operator= (const TextImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mLocation = object.mLocation;
      mText = object.mText;
      mFont = object.mFont;
      mColor = object.mColor;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType TextImp::getType() const
{
   return TEXT_OBJECT_TYPE;
}

void TextImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   PlotViewImp* pPlot = dynamic_cast<PlotViewImp*>(getPlot());
   if (pPlot != NULL)
   {
      glColor3ub(mColor.red(), mColor.green(), mColor.blue());

      double dScreenX = 0.0;
      double dScreenY = 0.0;
      pPlot->translateDataToScreen(mLocation.mX, mLocation.mY, dScreenX, dScreenY);

      int screenX = static_cast<int>(dScreenX);
      int screenY = pPlot->height() - static_cast<int>(dScreenY);
      pPlot->renderText(screenX, screenY, mText, mFont);
   }
}

double TextImp::getXLocation() const
{
   return mLocation.mX;
}

double TextImp::getYLocation() const
{
   return mLocation.mY;
}

const LocationType& TextImp::getLocation() const
{
   return mLocation;
}

QString TextImp::getText() const
{
   return mText;
}

QFont TextImp::getFont() const
{
   return mFont;
}

QColor TextImp::getColor() const
{
   return mColor;
}

bool TextImp::hit(LocationType point) const
{
   double dMinX = 0.0;
   double dMinY = 0.0;
   double dMaxX = 0.0;
   double dMaxY = 0.0;

   bool bSuccess = const_cast<TextImp*>(this)->getExtents(dMinX, dMinY, dMaxX, dMaxY);
   if (bSuccess == true)
   {
      if ((point.mX >= dMinX) && (point.mX <= dMaxX) && (point.mY >= dMinY) && (point.mY <= dMaxY))
      {
         return true;
      }
   }

   return false;
}

bool TextImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   PlotViewImp* pPlot = getPlot();
   if (pPlot == NULL)
   {
      return false;
   }

   double dScreenX = 0;
   double dScreenY = 0;
   pPlot->translateDataToScreen(mLocation.mX, mLocation.mY, dScreenX, dScreenY);

   QFontMetrics ftMetrics(mFont);
   int iHeight = ftMetrics.height();
   int iWidth = ftMetrics.width(mText);

   double dScreenExtentX = dScreenX + iWidth;
   double dScreenExtentY = dScreenY + iHeight;

   pPlot->translateDataToWorld(mLocation.mX, mLocation.mY, dMinX, dMinY);
   pPlot->translateScreenToWorld(dScreenExtentX, dScreenExtentY, dMaxX, dMaxY);

   return true;
}

const QPixmap& TextImp::getLegendPixmap(bool bSelected) const
{
   // QPixmap must be destroyed before QApplication. This can't be guaranteed with
   // a static object. A heap object will leak but since the lifespan of this object
   // is the life of the application this is ok.
   static QPixmap* spPix(NULL);
   static QPixmap* spSelectedPix(NULL);
   if (!spPix)
   {
      spPix = new QPixmap(25, 15);
   }
   if (!spSelectedPix)
   {
      spSelectedPix = new QPixmap(25, 15);
   }
   static QColor pixColor;
   static QFont pixFont;
   static QColor selectedPixColor;
   static QFont selectedPixFont;

   if ((bSelected == true) && (spSelectedPix->isNull() == false))
   {
      if ((selectedPixColor != mColor) || (selectedPixFont != mFont))
      {
         selectedPixColor = mColor;
         selectedPixFont = mFont;
         spSelectedPix->fill(Qt::transparent);

         QFont ftPix = mFont;
         ftPix.setPointSize(8);
         ftPix.setBold(bSelected);

         QPainter p(spSelectedPix);
         p.setFont(ftPix);
         p.setPen(mColor);
         p.drawText(QPoint(2, 12), "ABC");
         p.end();
      }

      return *spSelectedPix;
   }
   else if ((bSelected == false) && (spPix->isNull() == false))
   {
      if ((pixColor != mColor) || (pixFont != mFont))
      {
         pixColor = mColor;
         pixFont = mFont;
         spPix->fill(Qt::transparent);

         QFont ftPix = mFont;
         ftPix.setPointSize(8);
         ftPix.setBold(bSelected);

         QPainter p(spPix);
         p.setFont(ftPix);
         p.setPen(mColor);
         p.drawText(QPoint(2, 12), "ABC");
         p.end();
      }

      return *spPix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void TextImp::setLocation(const LocationType& location)
{
   if (location != mLocation)
   {
      mLocation = location;
      emit locationChanged(mLocation);
      notify(SIGNAL_NAME(Text, LocationChanged), boost::any(mLocation));
   }
}

void TextImp::setLocation(double dX, double dY)
{
   LocationType location(dX, dY);
   setLocation(location);
}

void TextImp::setText(const QString& strText)
{
   if (strText != mText)
   {
      mText = strText;
      emit textChanged(mText);
      notify(SIGNAL_NAME(Text, TextChanged), boost::any(mText.toStdString()));
   }
}

void TextImp::setFont(const QFont& textFont)
{
   if (textFont != mFont)
   {
      mFont = textFont;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void TextImp::setColor(const QColor& clrText)
{
   if (clrText.isValid() == false)
   {
      return;
   }

   if (clrText != mColor)
   {
      mColor = clrText;
      emit legendPixmapChanged();
      notify(SIGNAL_NAME(Text, ColorChanged), boost::any(
         ColorType(mColor.red(), mColor.green(), mColor.blue())));
   }
}

bool TextImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }
   pXml->addAttr("color", QCOLOR_TO_COLORTYPE(mColor));
   pXml->addAttr("font", mFont.toString().toStdString());
   pXml->addAttr("text", mText.toStdString());
   pXml->addAttr("location", mLocation);
   return true;
}

bool TextImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElem->getAttribute(X("color"))));
   mColor = COLORTYPE_TO_QCOLOR(color);
   mFont.fromString(A(pElem->getAttribute(X("font"))));
   mText = A(pElem->getAttribute(X("text")));
   mLocation = StringUtilities::fromXmlString<LocationType>(A(pElem->getAttribute(X("location"))));

   return true;
}
