/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QStyle>

#include "CustomColorButton.h"
#include "ColorMenu.h"

CustomColorButton::CustomColorButton(QWidget* parent) :
   QToolButton(parent)
{
   initialize();
}

CustomColorButton::CustomColorButton(const QString& strText, QWidget* parent) :
   QToolButton(parent)
{
   initialize();
   setText(strText);
}

CustomColorButton::CustomColorButton(const QString& strText, const QColor& color, QWidget* parent) :
   QToolButton(parent)
{
   initialize();
   setText(strText);
   setColor(color);
}

CustomColorButton::~CustomColorButton()
{
}

QColor CustomColorButton::getColor() const
{
   return mColor;
}

ColorType CustomColorButton::getColorType() const
{
   return QCOLOR_TO_COLORTYPE(mColor);
}

bool CustomColorButton::isPopupGridUsed() const
{
   return (menu() != NULL);
}

void CustomColorButton::setColor(const QColor& clrNew)
{
   if (clrNew == mColor)
   {
      return;
   }

   mColor = clrNew;
   if (mColor.isValid() == true)
   {
      QBrush brush(clrNew);
      QColor borderColor = QColor(127, 157, 185);
      QPen pen(borderColor);
      QPixmap pix = QPixmap(16, 16);
      QRectF rect(0, 0, 15, 15);
      QPainter p;

      p.begin(&pix);
      p.fillRect(rect, brush);
      p.setPen(pen);
      p.drawRect(rect);
      p.end();

      setIcon(QIcon(pix));
   }
   else
   {
      setIcon(QIcon(":/icons/Forbidden"));
   }

   emit colorChanged(clrNew);
}

void CustomColorButton::setColor(const ColorType& clrNew)
{
   QColor newColor = COLORTYPE_TO_QCOLOR(clrNew);
   setColor(newColor);
}

void CustomColorButton::usePopupGrid(bool bGrid)
{
   if (bGrid == true)
   {
      setMenu(mpMenu);
      setPopupMode(QToolButton::MenuButtonPopup);
   }
   else
   {
      setMenu(NULL);
      setPopupMode(QToolButton::DelayedPopup);
   }
}

void CustomColorButton::initialize()
{
   // Create the popup color menu
   mpMenu = new ColorMenu(this);

   // Initialization
   setColor(Qt::black);
   usePopupGrid(false);
   setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
   setAutoRaise(false);

   // Connections
   connect(mpMenu, SIGNAL(aboutToShow()), this, SLOT(initializeColorMenu()));
   connect(mpMenu, SIGNAL(colorSelected(const QColor&)), this, SLOT(setColor(const QColor&)));
   connect(this, SIGNAL(clicked()), mpMenu, SLOT(setCustomColor()));
}

void CustomColorButton::initializeColorMenu()
{
   mpMenu->setSelectedColor(mColor);
}
