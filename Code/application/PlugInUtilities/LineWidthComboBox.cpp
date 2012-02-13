/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "LineWidthComboBox.h"

#include <QtGui/QBitmap>
#include <QtGui/QPainter>

LineWidthComboBox::LineWidthComboBox(QWidget* pParent) :
   QComboBox(pParent)
{
   setEditable(false);

   QSize lineWidthSize(75, 16);
   setIconSize(lineWidthSize);

   for (int i = 0; i < 6; ++i)
   {
      QPixmap pix = QPixmap(lineWidthSize);
      pix.fill(Qt::white);

      QPainter p(&pix);
      p.setPen(QPen(Qt::black, i + 1));
      p.drawLine(pix.rect().left(), pix.height() / 2, pix.rect().right(), pix.height() / 2);
      p.end();

      pix.setMask(pix.createMaskFromColor(Qt::white));
      addItem(QIcon(pix), QString());
   }

   VERIFYNR(connect(this, SIGNAL(activated(int)), this, SLOT(translateActivated(int))));
}

LineWidthComboBox::~LineWidthComboBox()
{}

void LineWidthComboBox::setCurrentValue(unsigned int value)
{
   if (value >= 1 && value <= 7)
   {
      setCurrentIndex(value - 1);
   }
   else
   {
      setCurrentIndex(-1);
   }
}

unsigned int LineWidthComboBox::getCurrentValue() const
{
   return currentIndex() + 1;
}

void LineWidthComboBox::translateActivated(int newIndex)
{
   emit valueChanged(newIndex + 1);
}
