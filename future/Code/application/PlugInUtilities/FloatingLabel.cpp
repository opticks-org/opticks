/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <QtGui/QPainter>

#include "FloatingLabel.h"

FloatingLabel::FloatingLabel(Qt::Orientation eOrientation, QWidget* parent) :
   QLabel(parent),
   meOrientation(eOrientation),
   miPosition(0.0),
   meJustification(AUTO_JUSTIFIED)
{
}

FloatingLabel::~FloatingLabel()
{
}

Qt::Orientation FloatingLabel::getOrientation() const
{
   return meOrientation;
}

void FloatingLabel::setTextPosition(int iPosition)
{
   int iLimit = getPositionLimit();
   if (iPosition > iLimit)
   {
      iPosition = iLimit;
   }

   if (iPosition < 0.0)
   {
      iPosition = 0.0;
   }

   if (iPosition != miPosition)
   {
      miPosition = iPosition;
      emit positionChanged(iPosition);
      repaint();
   }
}

int FloatingLabel::getTextPosition() const
{
   return miPosition;
}

void FloatingLabel::setTextJustification(FloatingLabel::TextJustification eJustification)
{
   if (eJustification != meJustification)
   {
      meJustification = eJustification;
      emit justificationChanged(eJustification);
      repaint();
   }
}

FloatingLabel::TextJustification FloatingLabel::getTextJustification() const
{
   return meJustification;
}

QSize FloatingLabel::sizeHint() const
{
   QSize szLabel = QLabel::sizeHint();
   if (meOrientation == Qt::Vertical)
   {
      int iTemp = szLabel.width();
      szLabel.setWidth(szLabel.height());
      szLabel.setHeight(iTemp);
   }

   return szLabel;
}

void FloatingLabel::paintEvent(QPaintEvent* pEvent)
{
   if (pEvent != NULL)
   {
      QPainter painter(this);

      int iTranslation = 0;
      int iLimit = getPositionLimit();

      QString strText = text();
      int iTextWidth = painter.fontMetrics().width(strText);

      switch (meJustification)
      {
         case AUTO_JUSTIFIED:
         {
            if (miPosition < (iTextWidth * 0.5))
            {
               iTranslation = 0;
            }
            else if (miPosition > (iLimit - (iTextWidth * 0.5)))
            {
               iTranslation = iLimit - iTextWidth;
            }
            else
            {
               iTranslation = miPosition - (iTextWidth * 0.5);
            }

            break;
         }

         case LEFT_JUSTIFIED:
            iTranslation = miPosition;
            break;

         case CENTER_JUSTIFIED:
            iTranslation = miPosition - (iTextWidth * 0.5);
            break;

         case RIGHT_JUSTIFIED:
            iTranslation = miPosition - iTextWidth;
            break;

         default:
            break;
      }

      QRect rcLabel = rect();
      switch (meOrientation)
      {
         case Qt::Horizontal:
            painter.translate(iTranslation, 0.0);
            painter.drawText(rcLabel.bottomLeft(), strText);
            break;

         case Qt::Vertical:
            painter.rotate(90.0);
            painter.drawText(rcLabel.top() + iTranslation, 0, strText);
            break;

         default:
            break;
      }

      painter.end();
   }
   else
   {
      QLabel::paintEvent(pEvent);
   }
}

int FloatingLabel::getPositionLimit() const
{
   int iLimit = 0;
   switch (meOrientation)
   {
      case Qt::Horizontal:
         iLimit = width();
         break;

      case Qt::Vertical:
         iLimit = height();
         break;
   }

   return iLimit;
}
