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
   QPushButton(pParent),
   mElideMode(Qt::ElideLeft)
{}

ElidedButton::ElidedButton(const QString& text, QWidget* pParent) :
   QPushButton(text, pParent),
   mElideMode(Qt::ElideLeft)
{}

ElidedButton::~ElidedButton()
{}

void ElidedButton::setElideMode(Qt::TextElideMode mode)
{
   if (mode != mElideMode)
   {
      // Update the elide mode
      mElideMode = mode;

      // Update the elided text
      QRect textRect;

      QStyle* pStyle = style();
      if (pStyle != NULL)
      {
         QStyleOptionButton option;
         initStyleOption(&option);

         QRect elementRect = pStyle->subElementRect(QStyle::SE_PushButtonContents, &option, this);
         int menuButtonSize = pStyle->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
         textRect = elementRect.adjusted(0, 0, -menuButtonSize, 0);
      }

      mElidedText = fontMetrics().elidedText(text(), mElideMode, textRect.width());

      // Redraw the button with the updated text
      repaint();
   }
}

Qt::TextElideMode ElidedButton::getElideMode() const
{
   return mElideMode;
}

void ElidedButton::paintEvent(QPaintEvent* pEvent)
{
   QStyleOptionButton option;
   initStyleOption(&option);

   bool updateElidedText = false;
   QRect textRect;

   QStyle* pStyle = style();
   if (pStyle != NULL)
   {
      QRect elementRect = pStyle->subElementRect(QStyle::SE_PushButtonContents, &option, this);
      int menuButtonSize = pStyle->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
      textRect = elementRect.adjusted(0, 0, -menuButtonSize, 0);
   }

   if (textRect != mPreviousRect)
   {
      mPreviousRect = textRect;
      updateElidedText = true;
   }

   QString currentText = text();
   if (currentText != mPreviousText)
   {
      mPreviousText = currentText;
      updateElidedText = true;
   }

   if (updateElidedText == true)
   {
      mElidedText = fontMetrics().elidedText(currentText, mElideMode, textRect.width(), Qt::TextShowMnemonic);
   }

   option.text = mElidedText;

   QStylePainter p(this);
   p.drawControl(QStyle::CE_PushButton, option);
}
