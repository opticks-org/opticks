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
   QLabel(pParent)
{}

ElidedLabel::ElidedLabel(const QString& text, QWidget* pParent) :
   QLabel(text, pParent)
{}

ElidedLabel::~ElidedLabel()
{}

void ElidedLabel::paintEvent(QPaintEvent* pEvent)
{
   QFontMetrics metrics = fontMetrics();
   if (metrics.width(text()) > contentsRect().width())
   {
      QString elidedText = metrics.elidedText(text(), Qt::ElideLeft, contentsRect().width());

      QPainter p(this);
      p.drawText(contentsRect(), elidedText);
   }
   else
   {
      QLabel::paintEvent(pEvent);
   }
}
