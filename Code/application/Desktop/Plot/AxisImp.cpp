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
#include <QtGui/QPixmap>
#include <qwt_math.h>
#include <qwt_scale_map.h>

#include "AxisImp.h"
#include "StringUtilities.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

const int TITLE_MARGIN = 5;

AxisImp::AxisImp(AxisPosition position, QWidget* pParent) :
   QWidget(pParent),
   mMaxMajorTicks(10),
   mMaxMinorTicks(4),
   mScaleType(SCALE_LINEAR),
   mTitle(QString()),
   mTitleFont(QApplication::font()),
   mTitleColor(Qt::black)
{
   setAttribute(Qt::WA_PaintOutsidePaintEvent);
   mLinearScale.setAttribute(QwtScaleEngine::Floating);
   mLogScale.setAttribute(QwtScaleEngine::Floating);

   switch (position)
   {
      case AXIS_LEFT:
         mScaleDraw.setAlignment(QwtScaleDraw::LeftScale);
         setMinimumSize(75, 100);
         break;

      case AXIS_BOTTOM:
         mScaleDraw.setAlignment(QwtScaleDraw::BottomScale);
         setMinimumSize(100, 25);
         break;

      case AXIS_TOP:
         mScaleDraw.setAlignment(QwtScaleDraw::TopScale);
         setMinimumSize(100, 25);
         break;

      case AXIS_RIGHT:
         mScaleDraw.setAlignment(QwtScaleDraw::RightScale);
         setMinimumSize(75, 100);
         break;

      default:
         mScaleDraw.setAlignment(QwtScaleDraw::BottomScale);
         setMinimumSize(100, 100);
         break;
   }

   mScaleDraw.move(0, 0);
   mScaleDraw.setLength(10);
}

AxisImp::~AxisImp()
{
}

AxisPosition AxisImp::getPosition() const
{
   AxisPosition position;

   QwtScaleDraw::Alignment scaleAlignment = mScaleDraw.alignment();
   switch (scaleAlignment)
   {
      case QwtScaleDraw::LeftScale:
         position = AXIS_LEFT;
         break;

      case QwtScaleDraw::BottomScale:
         position = AXIS_BOTTOM;
         break;

      case QwtScaleDraw::TopScale:
         position = AXIS_TOP;
         break;

      case QwtScaleDraw::RightScale:
         position = AXIS_RIGHT;
         break;

      default:
         position = AXIS_BOTTOM;
         break;
   }

   return position;
}

QString AxisImp::getTitle() const
{
   return mTitle;
}

QFont AxisImp::getTitleFont() const
{
   return mTitleFont.toQFont();
}

QColor AxisImp::getTitleColor() const
{
   return mTitleColor;
}

ScaleType AxisImp::getScaleType() const
{
   return mScaleType;
}

void AxisImp::setValueRange(double dMin, double dMax)
{
   if ((dMin == getMinimumValue()) && (dMax == getMaximumValue()))
   {
      return;
   }

   // Calculate a new scale
   QwtScaleDiv newScaleDiv;
   double dStepSize = 0.0;

   if (mScaleType == SCALE_LOG)
   {
      mLogScale.autoScale(mMaxMajorTicks, dMin, dMax, dStepSize);
      newScaleDiv = mLogScale.divideScale(dMin, dMax, mMaxMajorTicks, mMaxMinorTicks);
   }
   else
   {
      mLinearScale.autoScale(mMaxMajorTicks, dMin, dMax, dStepSize);
      newScaleDiv = mLinearScale.divideScale(dMin, dMax, mMaxMajorTicks, mMaxMinorTicks);
   }

   mScaleDraw.setScaleDiv(newScaleDiv);
   emit rangeChanged(dMin, dMax);

   // Force a repaint
   repaint();
}

double AxisImp::getMinimumValue() const
{
   QwtScaleDiv scaleDiv = mScaleDraw.scaleDiv();
   return scaleDiv.lBound();
}

double AxisImp::getMaximumValue() const
{
   QwtScaleDiv scaleDiv = mScaleDraw.scaleDiv();
   return scaleDiv.hBound();
}

vector<double> AxisImp::getMajorTickLocations() const
{
   vector<double> majorTicks;
   QwtScaleDiv scaleDiv = mScaleDraw.scaleDiv();

   QwtValueList majorTickList = scaleDiv.ticks(QwtScaleDiv::MajorTick);
   for (int i = 0; i < majorTickList.size(); ++i)
   {
      double dValue = majorTickList[i];
      majorTicks.push_back(dValue);
   }

   return majorTicks;
}

vector<double> AxisImp::getMinorTickLocations() const
{
   vector<double> minorTicks;
   QwtScaleDiv scaleDiv = mScaleDraw.scaleDiv();

   QwtValueList minorTickList = scaleDiv.ticks(QwtScaleDiv::MinorTick) + scaleDiv.ticks(QwtScaleDiv::MediumTick);
   for (int i = 0; i < minorTickList.size(); ++i)
   {
      double dValue = minorTickList[i];
      minorTicks.push_back(dValue);
   }

   return minorTicks;
}

void AxisImp::setMaxNumMajorTicks(int numTicks)
{
   if (numTicks != mMaxMajorTicks)
   {
      mMaxMajorTicks = numTicks;
      updateScale();
      emit maxNumMajorTicksChanged(mMaxMajorTicks);
      update();
   }
}

void AxisImp::setMaxNumMinorTicks(int numTicks)
{
   if (numTicks != mMaxMinorTicks)
   {
      mMaxMinorTicks = numTicks;
      updateScale();
      emit maxNumMinorTicksChanged(mMaxMinorTicks);
      update();
   }
}

int AxisImp::getMaxNumMajorTicks() const
{
   return mMaxMajorTicks;
}

int AxisImp::getMaxNumMinorTicks() const
{
   return mMaxMinorTicks;
}

void AxisImp::setLabelFormat(const QString& strFormat)
{
   mScaleDraw.setLabelFormat(strFormat);
   update();
}

QString AxisImp::getLabelFormat() const
{
   return mScaleDraw.getLabelFormat();
}

QSize AxisImp::sizeHint() const
{
   QFont labelFont = font();
   int iScaleWidth = 0;
   int iScaleHeight = 0;
   int iTitleHeight = 0;
   int iTitleWidth = 0;

   if (mTitle.isEmpty() == false)
   {
      QFontMetrics fm(mTitleFont.getQFont());
      iTitleHeight = fm.height() + TITLE_MARGIN + 2;
      iTitleWidth = fm.width(mTitle) + TITLE_MARGIN + 2;
   }

   QSize szScale;
   switch (mScaleDraw.alignment())
   {
      case QwtScaleDraw::LeftScale:
      case QwtScaleDraw::RightScale:
      {
         iScaleWidth = mScaleDraw.extent(QPen(Qt::black), labelFont);
         iScaleHeight = mScaleDraw.minLength(QPen(Qt::black), labelFont);

         int iStartWidth = 0;
         int iEndWidth = 0;
         mScaleDraw.getBorderDistHint(labelFont, iStartWidth, iEndWidth);

         szScale.setWidth(iScaleWidth + iTitleHeight + (2 * (iEndWidth - iStartWidth)));
         szScale.setHeight(qwtMax(iTitleWidth, iScaleHeight));
         break;
      }

      case QwtScaleDraw::TopScale:
      case QwtScaleDraw::BottomScale:
      {
         iScaleWidth = mScaleDraw.minLength(QPen(Qt::black), labelFont);
         iScaleHeight = mScaleDraw.extent(QPen(Qt::black), labelFont);

         szScale.setWidth(qwtMax(iTitleWidth, iScaleWidth));
         szScale.setHeight(iScaleHeight + iTitleHeight);
         break;
      }

      default:
         break;
   }

   return szScale;
}

void AxisImp::setTitle(const QString& title)
{
   if (title != mTitle)
   {
      mTitle = title;
      emit titleChanged(mTitle);
      updateSize();
      update();
   }
}

void AxisImp::setTitleFont(const QFont& titleFont)
{
   if (titleFont != mTitleFont.getQFont())
   {
      mTitleFont = titleFont;
      emit titleFontChanged(mTitleFont.getQFont());
      updateSize();
      update();
   }
}

void AxisImp::setTitleColor(const QColor& titleColor)
{
   if (titleColor.isValid() == false)
   {
      return;
   }

   if (titleColor != mTitleColor)
   {
      mTitleColor = titleColor;
      emit titleColorChanged(mTitleColor);
      update();
   }
}

void AxisImp::setScaleType(ScaleType scaleType)
{
   if (scaleType != mScaleType)
   {
      mScaleType = scaleType;
      updateScale();
      emit scaleTypeChanged(mScaleType);
      update();
   }
}

void AxisImp::paintEvent(QPaintEvent* e)
{
   // Draw to an off-screen pixmap
   QRect rcWidget = rect();
   QPixmap pix(rcWidget.size());
   pix.fill(this, 0, 0);

   QPainter p(&pix);

   // Size the scale
   int iScaleWidth = 0;
   int iScaleHeight = 0;

   switch (mScaleDraw.alignment())
   {
      case QwtScaleDraw::LeftScale:
         iScaleWidth = mScaleDraw.extent(p.pen(), p.font());
         iScaleHeight = mScaleDraw.minLength(p.pen(), p.font());
         mScaleDraw.move(rcWidget.right(), rcWidget.y());
         mScaleDraw.setLength(rcWidget.height());
         break;

      case QwtScaleDraw::RightScale:
         iScaleWidth = mScaleDraw.extent(p.pen(), p.font());
         iScaleHeight = mScaleDraw.minLength(p.pen(), p.font());
         mScaleDraw.move(rcWidget.x(), rcWidget.y());
         mScaleDraw.setLength(rcWidget.height());
         break;

      case QwtScaleDraw::TopScale:
         iScaleWidth = mScaleDraw.minLength(p.pen(), p.font());
         iScaleHeight = mScaleDraw.extent(p.pen(), p.font());
         mScaleDraw.move(rcWidget.x(), rcWidget.bottom());
         mScaleDraw.setLength(rcWidget.width());
         break;

      case QwtScaleDraw::BottomScale:
         iScaleWidth = mScaleDraw.minLength(p.pen(), p.font());
         iScaleHeight = mScaleDraw.extent(p.pen(), p.font());
         mScaleDraw.move(rcWidget.x(), rcWidget.y());
         mScaleDraw.setLength(rcWidget.width());
         break;

      default:
         break;
   }

   // Draw the scale
   mScaleDraw.draw(&p, palette());

   // Draw the title
   if (mTitle.isEmpty() == false)
   {
      p.setFont(mTitleFont.toQFont());
      p.setPen(mTitleColor);
      int iTitleHeight = p.fontMetrics().height();

      QRect rcText = rcWidget;
      switch (mScaleDraw.alignment())
      {
         case QwtScaleDraw::LeftScale:
            p.rotate(-90.0);
            p.drawText(-rcWidget.bottom(), rcWidget.left(), rcWidget.height(),
               rcWidget.width() - iScaleWidth - TITLE_MARGIN, Qt::AlignCenter, mTitle);
            break;

         case QwtScaleDraw::RightScale:
            p.rotate(90.0);
            p.drawText(rcWidget.top(), rcWidget.right(), rcWidget.height(),
               rcWidget.width() - iScaleWidth - TITLE_MARGIN, Qt::AlignCenter, mTitle);
            break;

         case QwtScaleDraw::TopScale:
            rcText.setBottom(rcWidget.bottom() - iScaleHeight - TITLE_MARGIN);
            p.drawText(rcText, Qt::AlignCenter, mTitle);
            break;

         case QwtScaleDraw::BottomScale:
            rcText.setTop(rcWidget.top() + iScaleHeight + TITLE_MARGIN);
            p.drawText(rcText, Qt::AlignCenter, mTitle);
            break;

         default:
            break;
      }
   }

   p.end();

   // Draw to the widget
   p.begin(this);
   p.drawPixmap(rcWidget.topLeft(), pix);
   p.end();
}

void AxisImp::updateSize()
{
   switch (mScaleDraw.alignment())
   {
      case QwtScaleDraw::LeftScale:
      case QwtScaleDraw::RightScale:
         setMinimumWidth(sizeHint().width());
         break;

      case QwtScaleDraw::TopScale:
      case QwtScaleDraw::BottomScale:
         setMinimumHeight(sizeHint().height());
         break;

      default:
         break;
   }
}

void AxisImp::updateScale()
{
   const QwtScaleDiv& scaleDiv = mScaleDraw.scaleDiv();
   QwtScaleDiv newScaleDiv;
   QwtScaleTransformation* pScaleTransformation = NULL;

   double dStart = scaleDiv.lBound();
   double dEnd = scaleDiv.hBound();
   double dStepSize = 0.0;

   if (mScaleType == SCALE_LOG)
   {
      mLogScale.autoScale(mMaxMajorTicks, dStart, dEnd, dStepSize);
      newScaleDiv = mLogScale.divideScale(dStart, dEnd, mMaxMajorTicks, mMaxMinorTicks);
      pScaleTransformation = mLogScale.transformation();
   }
   else
   {
      mLinearScale.autoScale(mMaxMajorTicks, dStart, dEnd, dStepSize);
      newScaleDiv = mLinearScale.divideScale(dStart, dEnd, mMaxMajorTicks, mMaxMinorTicks);
      pScaleTransformation = mLinearScale.transformation();
   }

   mScaleDraw.setScaleDiv(newScaleDiv);
   mScaleDraw.setTransformation(pScaleTransformation);
}

const FontImp& AxisImp::getTitleFontImp() const
{
   return mTitleFont;
}

bool AxisImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("title", mTitle.toStdString());
   pXml->pushAddPoint(pXml->addElement("titleFont"));
   if(!mTitleFont.toXml(pXml))
   {
      return false;
   }
   pXml->popAddPoint();
   pXml->addAttr("titleColor", QCOLOR_TO_COLORTYPE(mTitleColor));
   pXml->addAttr("scaleType", mScaleType);
   pXml->addAttr("maxMajorTicks", mMaxMajorTicks);
   pXml->addAttr("maxMinorTicks", mMaxMinorTicks);
   pXml->addAttr("scaleDrawLabelFormat", mScaleDraw.getLabelFormat().toStdString());
   return true;
}

bool AxisImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* pElmnt = dynamic_cast<DOMElement*>(pDocument);
   if (pElmnt == NULL)
   {
      return false;
   }

   mTitle = A(pElmnt->getAttribute(X("title")));
   for (DOMNode* pNode = pElmnt->getFirstChild(); pNode != NULL; pNode = pNode->getNextSibling())
   {
      if (XMLString::equals(pNode->getNodeName(), X("titleFont")))
      {
         DOMElement* pFontElement = static_cast<DOMElement*>(pNode);
         if (mTitleFont.fromXml(pFontElement, version) == false)
         {
            return false;
         }
      }
   }

   ColorType color = StringUtilities::fromXmlString<ColorType>(A(pElmnt->getAttribute(X("titleColor"))));
   mTitleColor = COLORTYPE_TO_QCOLOR(color);
   mScaleType = StringUtilities::fromXmlString<ScaleType>(A(pElmnt->getAttribute(X("scaleType"))));
   mMaxMajorTicks = StringUtilities::fromXmlString<int>(A(pElmnt->getAttribute(X("maxMajorTicks"))));
   mMaxMinorTicks = StringUtilities::fromXmlString<int>(A(pElmnt->getAttribute(X("maxMinorTicks"))));
   mScaleDraw.setLabelFormat(A(pElmnt->getAttribute(X("scaleDrawLabelFormat"))));

   updateScale();
   updateSize();

   return true;
}
