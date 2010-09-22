/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFontMetrics>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QStylePainter>

#include "ElidedButton.h"

ElidedButton::ElidedButton(QWidget* pParent) :
   QPushButton(pParent)
{}

ElidedButton::ElidedButton(const QString& text, QWidget* pParent) :
   QPushButton(text, pParent)
{}

ElidedButton::~ElidedButton()
{}

void ElidedButton::paintEvent(QPaintEvent* pEvent)
{
   QStyleOptionButton option;
   initStyleOption(&option);

   int textWidth = contentsRect().width();

   QStyle* pStyle = style();
   if (pStyle != NULL)
   {
      QRect textRect = pStyle->subElementRect(QStyle::SE_PushButtonContents, &option, this);
      int menuButtonSize = pStyle->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
      textRect = textRect.adjusted(0, 0, -menuButtonSize, 0);
      textWidth = textRect.width();
   }

   QFontMetrics metrics = fontMetrics();
   if (metrics.width(text()) > textWidth)
   {
      QString elidedText = metrics.elidedText(text(), Qt::ElideLeft, textWidth);
      option.text = elidedText;

      QStylePainter p(this);
      p.drawControl(QStyle::CE_PushButton, option);
   }
   else
   {
      QPushButton::paintEvent(pEvent);
   }
}
