/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

#include "ElidedLabel.h"

ElidedLabel::ElidedLabel(QWidget* pParent) :
   QLabel(pParent),
   mElideMode(Qt::ElideLeft)
{}

ElidedLabel::ElidedLabel(const QString& text, QWidget* pParent) :
   QLabel(text, pParent),
   mElideMode(Qt::ElideLeft)
{}

ElidedLabel::~ElidedLabel()
{}

void ElidedLabel::setElideMode(Qt::TextElideMode mode)
{
   if (mode != mElideMode)
   {
      // Update the elide mode
      mElideMode = mode;

      // Update the elided text
      QString currentText = text();
      if (currentText.isEmpty() == false)
      {
         int textMargin = margin();
         QRect textRect = contentsRect();
         textRect.adjust(textMargin, textMargin, textMargin, textMargin);
         mElidedText = fontMetrics().elidedText(currentText, mElideMode, textRect.width());
      }

      // Redraw the button with the updated text
      repaint();
   }
}

Qt::TextElideMode ElidedLabel::getElideMode() const
{
   return mElideMode;
}

void ElidedLabel::paintEvent(QPaintEvent* pEvent)
{
   QString currentText = text();
   if (currentText.isEmpty() == false)    // Drawing text
   {
      bool updateElidedText = false;
      int textMargin = margin();

      QRect textRect = contentsRect();
      textRect.adjust(textMargin, textMargin, textMargin, textMargin);
      if (textRect != mPreviousRect)
      {
         mPreviousRect = textRect;
         updateElidedText = true;
      }

      if (currentText != mPreviousText)
      {
         mPreviousText = currentText;
         updateElidedText = true;
      }

      if (updateElidedText == true)
      {
         mElidedText = fontMetrics().elidedText(currentText, mElideMode, textRect.width());
      }

      // Draw elided text
      if (currentText != mElidedText)
      {
         QPainter p(this);
         p.drawText(textRect, mElidedText);
         return;
      }
   }

   // Draw a picture, pixmap, movie, or non-elided text, which may have an associated alignment
   QLabel::paintEvent(pEvent);
}
