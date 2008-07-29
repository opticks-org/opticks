/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QPolygon>

#include "glCommon.h"
#include "RegionObject.h"
#include "RegionObjectImp.h"
#include "ColorMap.h"
#include "AppVerify.h"
#include "PlotViewImp.h"
#include "xmlreader.h"

#include <boost/tuple/tuple.hpp>
#include <sstream>

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

RegionObjectImp::RegionObjectImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mMinX(0.0),
   mMinY(0.0),
   mMaxX(0.0),
   mMaxY(0.0),
   mTransparency(255),
   mBorder(false)
{
   VERIFYNR(connect(this, SIGNAL(regionChanged(double, double, double, double)), this, SIGNAL(extentsChanged())));
   VERIFYNR(connect(this, SIGNAL(colorsChanged(const std::vector<ColorType>&)), this, SIGNAL(legendPixmapChanged())));
}

RegionObjectImp::~RegionObjectImp()
{
}

RegionObjectImp& RegionObjectImp::operator= (const RegionObjectImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mMinX = object.mMinX;
      mMinY = object.mMinY;
      mMaxX = object.mMaxX;
      mMaxY = object.mMaxY;
      mColors = object.mColors;
      mTransparency = object.mTransparency;
      mBorder = object.mBorder;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

const string& RegionObjectImp::getObjectType() const
{
   static string type("RegionObjectImp");
   return type;
}

bool RegionObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RegionObject") || (className == "Region"))
   {
      return true;
   }

   return PlotObjectImp::isKindOf(className);
}

PlotObjectType RegionObjectImp::getType() const
{
   return REGION;
}

void RegionObjectImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   if (isValid() == false)
   {
      return;
   }

   // Loop below needs at least 2 colors
   if (mColors.empty() == true)
   {
      return;
   }

   if (mColors.size() == 1)
   {
      mColors.push_back(mColors.front());
   }

   glPushAttrib(GL_COLOR_BUFFER_BIT);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   PlotViewImp* pPlot = getPlot();
   VERIFYNRV(pPlot != NULL);

   double worldMinX, worldMinY;
   double worldMaxX, worldMaxY;
   pPlot->translateDataToWorld(mMinX, mMinY, worldMinX, worldMinY);
   pPlot->translateDataToWorld(mMaxX, mMaxY, worldMaxX, worldMaxY);

   double screenMinX, screenMinY;
   double screenMaxX, screenMaxY;
   pPlot->translateWorldToScreen(worldMinX, worldMinY, screenMinX, screenMinY);
   pPlot->translateWorldToScreen(worldMaxX, worldMaxY, screenMaxX, screenMaxY);

   double dLineWidth = fabs(screenMaxX - screenMinX);
   double dColorWidth = (worldMaxX - worldMinX) / (mColors.size() - 1);

   if (dLineWidth < 1.5)
   {
      ColorType color = mColors.front();
      glColor4ub(color.mRed, color.mGreen, color.mBlue, mTransparency * color.mAlpha / 255);

      glBegin(GL_LINES);
      glVertex2d(worldMinX, worldMinY);
      glVertex2d(worldMinX, worldMaxY);
   }
   else
   {
      glShadeModel(GL_SMOOTH);
      glBegin(GL_QUAD_STRIP);

      double dCurrentX = worldMinX;
      for (unsigned int i = 0; i < mColors.size(); ++i)
      {
         ColorType color = mColors.at(i);
         glColor4ub(color.mRed, color.mGreen, color.mBlue, mTransparency * color.mAlpha / 255);
         glVertex2d(dCurrentX, worldMinY);
         glVertex2d(dCurrentX, worldMaxY);
         dCurrentX += dColorWidth;
     }
   }

   glEnd();

   glShadeModel(GL_FLAT);
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_BLEND);
   glPopAttrib();

   // Draw the border if necessary
   if (mBorder == true)
   {
      glColor4ub(0, 0, 0, mTransparency);
      glBegin(GL_LINE_STRIP);
      glVertex2d(worldMinX, worldMinY);
      glVertex2d(worldMinX, worldMaxY);
      glVertex2d(worldMaxX, worldMaxY);
      glVertex2d(worldMaxX, worldMinY);
      glVertex2d(worldMinX, worldMinY);
      glEnd();
   }
}

bool RegionObjectImp::isValid() const
{
   PlotViewImp* pPlot = getPlot();
   VERIFY(pPlot != NULL);

   double worldMinX = 0.0;
   double worldMinY = 0.0;
   double worldMaxX = 0.0;
   double worldMaxY = 0.0;
   pPlot->translateDataToWorld(mMinX, mMinY, worldMinX, worldMinY);
   pPlot->translateDataToWorld(mMaxX, mMaxY, worldMaxX, worldMaxY);

   if ((worldMinX != worldMaxX) && (worldMinY != worldMaxY))
   {
      return true;
   }

   return false;
}

bool RegionObjectImp::isSolidColor() const
{
   if (mColors.empty() == true)
   {
      return false;
   }

   ColorType baseColor = mColors.front();

   vector<ColorType>::const_iterator iter = mColors.begin();
   while (iter != mColors.end())
   {
      ColorType currentColor = *iter;
      if (currentColor != baseColor)
      {
         return false;
      }

      ++iter;
   }

   return true;
}

QColor RegionObjectImp::getColor() const
{
   QColor currentColor;
   if (isSolidColor() == true)
   {
      ColorType color = mColors.front();
      if (color.isValid() == true)
      {
         currentColor.setRgb(color.mRed, color.mGreen, color.mBlue);
      }
   }

   return currentColor;
}

const vector<ColorType>& RegionObjectImp::getColors() const
{
   return mColors;
}

int RegionObjectImp::getTransparency() const
{
   return mTransparency;
}

bool RegionObjectImp::getDrawBorder() const
{
   return mBorder;
}

bool RegionObjectImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   if (isValid() == false)
   {
      return false;
   }

   PlotViewImp* pPlot = getPlot();
   VERIFY(pPlot != NULL);

   pPlot->translateDataToWorld(mMinX, mMinY, dMinX, dMinY);
   pPlot->translateDataToWorld(mMaxX, mMaxY, dMaxX, dMaxY);

   return true;
}

const QPixmap& RegionObjectImp::getLegendPixmap(bool bSelected) const
{
   static QPixmap pix(25, 15);
   static QColor pixColor;

   if (pix.isNull() == false)
   {
      QColor currentColor = getColor();
      currentColor.setAlpha(mTransparency);

      if (pixColor != currentColor)
      {
         pixColor = currentColor;
         pix.fill(Qt::white);

         QRect rcPixmap = pix.rect();

         QPolygon points(4);
         points.setPoint(0, rcPixmap.left() + 2, rcPixmap.bottom() - 2);
         points.setPoint(1, rcPixmap.right() - 2, rcPixmap.bottom() - 2);
         points.setPoint(2, rcPixmap.right() - 2, rcPixmap.top() + 2);
         points.setPoint(3, rcPixmap.left() + 2, rcPixmap.top() + 2);

         QPainter p(&pix);
         p.setBrush(currentColor);
         p.setPen(Qt::black);
         p.drawPolygon(points);
         p.end();
      }

      return pix;
   }

   return PlotObjectImp::getLegendPixmap(bSelected);
}

void RegionObjectImp::setRegion(double dMinX, double dMinY, double dMaxX, double dMaxY)
{
   if ((dMinX != mMinX) || (dMinY != mMinY) || (dMaxX != mMaxX) || (dMaxY != mMaxY))
   {
      mMinX = dMinX;
      mMinY = dMinY;
      mMaxX = dMaxX;
      mMaxY = dMaxY;

      emit regionChanged(mMinX, mMinY, mMaxX, mMaxY);
      notify(SIGNAL_NAME(RegionObject, RegionChanged), boost::any(
         boost::tuple<double,double,double,double>(mMinX, mMinY, mMaxX, mMaxY)));
   }
}

void RegionObjectImp::setColor(const QColor& clrRegion)
{
   if (clrRegion.isValid() == false)
   {
      return;
   }

   QColor currentColor = getColor();
   if (clrRegion != currentColor)
   {
      ColorType newColor(clrRegion.red(), clrRegion.green(), clrRegion.blue());

      mColors.clear();
      mColors.push_back(newColor);
      emit colorsChanged(mColors);
      notify(SIGNAL_NAME(RegionObject, ColorsChanged), boost::any(mColors));
   }
}

void RegionObjectImp::setColors(const vector<QColor>& colors)
{
   vector<ColorType> newColors;
   for (unsigned int i = 0; i < colors.size(); ++i)
   {
      QColor color = colors.at(i);
      if (color.isValid() == true)
      {
         newColors.push_back(ColorType(color.red(), color.green(), color.blue()));
      }
   }

   setColors(newColors);
}

void RegionObjectImp::setColors(const vector<ColorType>& colors)
{
   if (colors != mColors)
   {
      mColors = colors;
      emit colorsChanged(mColors);
      notify(SIGNAL_NAME(RegionObject, ColorsChanged), boost::any(mColors));
   }
}

void RegionObjectImp::setColors(const ColorMap& colorMap)
{
   const vector<ColorType>& newColors = colorMap.getTable();
   setColors(newColors);
}

void RegionObjectImp::setTransparency(int iTransparency)
{
   if (iTransparency < 0)
   {
      iTransparency = 0;
   }

   if (iTransparency > 255)
   {
      iTransparency = 255;
   }

   if (iTransparency != mTransparency)
   {
      mTransparency = iTransparency;
      emit transparencyChanged(mTransparency);
      notify(SIGNAL_NAME(RegionObject, TransparencyChanged), boost::any(mTransparency));
   }
}

void RegionObjectImp::setDrawBorder(bool bBorder)
{
   if (bBorder != mBorder)
   {
      mBorder = bBorder;
      emit borderToggled(mBorder);
      notify(SIGNAL_NAME(RegionObject, BorderToggled), boost::any(mBorder));
   }
}

bool RegionObjectImp::getRegion(double& minX, double& minY, double& maxX, double& maxY) const
{
   if (isValid() == false)
   {
      return false;
   }

   minX = mMinX;
   minY = mMinY;
   maxX = mMaxX;
   maxY = mMaxY;

   return true;
}

bool RegionObjectImp::toXml(XMLWriter* pXml) const
{
   pXml->pushAddPoint(pXml->addElement(getObjectType().c_str()));

   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   stringstream buf;
   buf << mMinX << " " << mMinY << " " << mMaxX << " " << mMaxY;
   pXml->addAttr("extents", buf.str());
   pXml->addAttr("transparency", mTransparency);
   pXml->addAttr("border", mBorder);

   if (mColors.size() > 0)
   {
      vector<ColorType>::const_iterator it;
      for (it=mColors.begin(); it!=mColors.end(); ++it)
      {
         ColorType color = *it;
         pXml->pushAddPoint(pXml->addElement("Color"));
         pXml->addAttr("alpha", color.mAlpha);
         pXml->addAttr("red", color.mRed);
         pXml->addAttr("green", color.mGreen);
         pXml->addAttr("blue", color.mBlue);
         pXml->popAddPoint();
      }
   }

   pXml->popAddPoint();
   return true;
}

bool RegionObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement *pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   double minX, minY, maxX, maxY;
   XmlReader::StrToQuadCoord(pElem->getAttribute(X("extents")), minX, minY, maxX, maxY);
   setRegion(minX, minY, maxX, maxY);

   setTransparency(StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("transparency")))));

   setDrawBorder(StringUtilities::fromXmlString<bool>(
      A(pElem->getAttribute(X("border")))));

   vector<ColorType> colors;
   for (DOMNode* pChld = pDocument->getFirstChild();
        pChld != NULL;
        pChld = pChld->getNextSibling())
   {
      string name = A(pChld->getNodeName());
      if (name == "Color")
      {
         pElem = static_cast<DOMElement*>(pChld);
         ColorType ct;
         ct.mAlpha = StringUtilities::fromXmlString<int>(
                     A(pElem->getAttribute(X("alpha"))));
         ct.mRed = StringUtilities::fromXmlString<int>(
                     A(pElem->getAttribute(X("red"))));
         ct.mGreen = StringUtilities::fromXmlString<int>(
                     A(pElem->getAttribute(X("green"))));
         ct.mBlue = StringUtilities::fromXmlString<int>(
                     A(pElem->getAttribute(X("blue"))));
         colors.push_back(ct);
      }
   }

   if (colors.size() > 0)
   {
      setColors(colors);
   }

   return true;
}
