/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GridlinesImp.h"
#include "PlotViewImp.h"
#include "xmlreader.h"

#include <string>

using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;

GridlinesImp::GridlinesImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary),
   mMinorGridlines(false),
   mColor(Qt::black),
   mLineWidth(1),
   mLineStyle(SOLID_LINE),
   mMaxMajorLines(10),
   mMaxMinorLines(4)
{
   if (pPlot != NULL)
   {
      connect(pPlot, SIGNAL(displayAreaChanged()), this, SLOT(updateLocations()));
   }
}

GridlinesImp::~GridlinesImp()
{
}

GridlinesImp& GridlinesImp::operator= (const GridlinesImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      mMinorGridlines = object.mMinorGridlines;
      mColor = object.mColor;
      mLineWidth = object.mLineWidth;
      mLineStyle = object.mLineStyle;
      mMaxMajorLines = object.mMaxMajorLines;
      mMaxMinorLines = object.mMaxMinorLines;

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

bool GridlinesImp::areMinorGridlinesEnabled() const
{
   return mMinorGridlines;
}

QColor GridlinesImp::getColor() const
{
   return mColor;
}

int GridlinesImp::getLineWidth() const
{
   return mLineWidth;
}

LineStyle GridlinesImp::getLineStyle() const
{
   return mLineStyle;
}

int GridlinesImp::getMaxNumMajorLines() const
{
   return mMaxMajorLines;
}

int GridlinesImp::getMaxNumMinorLines() const
{
   return mMaxMinorLines;
}

void GridlinesImp::setVisible(bool bVisible)
{
   PlotObjectImp::setVisible(bVisible);
   if (bVisible == true)
   {
      updateLocations();
   }
}

void GridlinesImp::enableMinorGridlines(bool bEnable)
{
   if (bEnable != mMinorGridlines)
   {
      mMinorGridlines = bEnable;
      emit minorGridlinesEnabled(mMinorGridlines);
      PlotViewImp* pPlot = getPlot();
      if (pPlot != NULL)
      {
         pPlot->refresh();
      }
      updateLocations();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void GridlinesImp::setColor(const QColor& clrLine)
{
   if ((clrLine == mColor) || (clrLine.isValid() == false))
   {
      return;
   }

   mColor = clrLine;
   emit colorChanged(mColor);
   PlotViewImp* pPlot = getPlot();
   if (pPlot != NULL)
   {
      pPlot->refresh();
   }
   notify(SIGNAL_NAME(Subject, Modified));
}

void GridlinesImp::setLineWidth(int iWidth)
{
   if ((iWidth == mLineWidth) || (iWidth < 0))
   {
      return;
   }

   mLineWidth = iWidth;
   emit lineWidthChanged(mLineWidth);
   PlotViewImp* pPlot = getPlot();
   if (pPlot != NULL)
   {
      pPlot->refresh();
   }
   notify(SIGNAL_NAME(Subject, Modified));
}

void GridlinesImp::setLineStyle(LineStyle eStyle)
{
   if ((eStyle == mLineStyle) || (!eStyle.isValid()))
   {
      return;
   }

   mLineStyle = eStyle;
   emit lineStyleChanged(mLineStyle);
   PlotViewImp* pPlot = getPlot();
   if (pPlot != NULL)
   {
      pPlot->refresh();
   }
   notify(SIGNAL_NAME(Subject, Modified));
}

void GridlinesImp::setMaxNumMajorLines(int numLines)
{
   if (numLines != mMaxMajorLines)
   {
      mMaxMajorLines = numLines;
      updateLocations();
      emit maxNumMajorLinesChanged(mMaxMajorLines);
      PlotViewImp* pPlot = getPlot();
      if (pPlot != NULL)
      {
         pPlot->refresh();
      }
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void GridlinesImp::setMaxNumMinorLines(int numLines)
{
   if (numLines != mMaxMinorLines)
   {
      mMaxMinorLines = numLines;
      updateLocations();
      emit maxNumMinorLinesChanged(mMaxMinorLines);
      PlotViewImp* pPlot = getPlot();
      if (pPlot != NULL)
      {
         pPlot->refresh();
      }
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

bool GridlinesImp::toXml(XMLWriter* pXml) const
{
   std::string strType = StringUtilities::toXmlString(getType());

   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("minorGridlines", mMinorGridlines);
   pXml->addAttr("color", mColor.name().toStdString());
   pXml->addAttr("lineWidth", mLineWidth);
   pXml->addAttr("lineStyle", mLineStyle);
   pXml->addAttr("maxMajorLines", mMaxMajorLines);
   pXml->addAttr("maxMinorLines", mMaxMinorLines);

   return true;
}

bool GridlinesImp::fromXml(DOMNode* pDocument, unsigned int version)
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

   mMinorGridlines = StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("minorGridlines"))));

   QColor color(A(pElem->getAttribute(X("color"))));
   setColor(color);

   setLineWidth(StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("lineWidth")))));

   setLineStyle(StringUtilities::fromXmlString<LineStyle>(
      A(pElem->getAttribute(X("lineStyle")))));
      
   setMaxNumMajorLines(StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("maxMajorLines")))));

   setMaxNumMinorLines(StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("maxMinorLines")))));

   return true;
}
