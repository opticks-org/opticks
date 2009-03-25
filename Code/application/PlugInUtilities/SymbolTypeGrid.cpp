/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SymbolTypeGrid.h"

#include "AppVerify.h"
#include "StringUtilities.h"

#include <QtGui/QBitmap>
#include <QtGui/QPainter>

#include <string>
#include <vector>

using namespace std;

SymbolTypeGrid::SymbolTypeGrid(QWidget* pParent)
: PixmapGrid(pParent)
{
   setBorderedSymbols(false);
   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void SymbolTypeGrid::setBorderedSymbols(bool showSymbols)
{
   //these two lines should clear any existing pixmaps
   setNumRows(0);
   setNumColumns(0);

   if (showSymbols)
   {
      setNumRows(4);
   }
   else
   {
      setNumRows(2);
   }
   setNumColumns(4);

   // Populate the icon grid
   setPixmap(0, 0, getSymbolPixmap(SOLID),
      QString::fromStdString(StringUtilities::toXmlString(SOLID)),
      "Filled Pixel");
   setPixmap(0, 1, getSymbolPixmap(X),
      QString::fromStdString(StringUtilities::toXmlString(X)),
      "X");
   setPixmap(0, 2, getSymbolPixmap(CROSS_HAIR),
      QString::fromStdString(StringUtilities::toXmlString(CROSS_HAIR)),
      "Crosshair");
   setPixmap(0, 3, getSymbolPixmap(ASTERISK),
      QString::fromStdString(StringUtilities::toXmlString(ASTERISK)),
      "Asterisk");
   setPixmap(1, 0, getSymbolPixmap(VERTICAL_LINE),
      QString::fromStdString(StringUtilities::toXmlString(VERTICAL_LINE)),
      "Vertical Line");
   setPixmap(1, 1, getSymbolPixmap(HORIZONTAL_LINE),
      QString::fromStdString(StringUtilities::toXmlString(HORIZONTAL_LINE)),
      "Horizontal Line");
   setPixmap(1, 2, getSymbolPixmap(FORWARD_SLASH),
      QString::fromStdString(StringUtilities::toXmlString(FORWARD_SLASH)),
      "Positive Slope");
   setPixmap(1, 3, getSymbolPixmap(BACK_SLASH),
      QString::fromStdString(StringUtilities::toXmlString(BACK_SLASH)),
      "Negative Slope");

   if (showSymbols)
   {
      setPixmap(2, 0, getSymbolPixmap(BOX), QString::fromStdString(StringUtilities::toXmlString(BOX)), "Border");
      setPixmap(2, 1, getSymbolPixmap(BOXED_X), QString::fromStdString(StringUtilities::toXmlString(BOXED_X)),
         "Bordered X");
      setPixmap(2, 2, getSymbolPixmap(BOXED_CROSS_HAIR),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_CROSS_HAIR)), "Bordered Crosshair");
      setPixmap(2, 3, getSymbolPixmap(BOXED_ASTERISK),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_ASTERISK)), "Bordered Asterisk");
      setPixmap(3, 0, getSymbolPixmap(BOXED_VERTICAL_LINE),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_VERTICAL_LINE)), "Bordered Vert. Line");
      setPixmap(3, 1, getSymbolPixmap(BOXED_HORIZONTAL_LINE),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_HORIZONTAL_LINE)), "Bordered Horiz. Line");
      setPixmap(3, 2, getSymbolPixmap(BOXED_FORWARD_SLASH),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_FORWARD_SLASH)), "Bordered Pos. Slope");
      setPixmap(3, 3, getSymbolPixmap(BOXED_BACK_SLASH),
         QString::fromStdString(StringUtilities::toXmlString(BOXED_BACK_SLASH)), "Bordered Neg. Slope");
   }
   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(SOLID)));
}

void SymbolTypeGrid::setCurrentValue(SymbolType value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

SymbolType SymbolTypeGrid::getCurrentValue() const
{
   SymbolType retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<SymbolType>(curText);
   }
   return retValue;
}

void SymbolTypeGrid::translateChange(const QString& strText)
{
   SymbolType curType = StringUtilities::fromXmlString<SymbolType>(strText.toStdString());
   emit valueChanged(curType);
}

QPixmap SymbolTypeGrid::getSymbolPixmap(SymbolType eSymbol)
{
   map<SymbolType,QPixmap>::iterator iter = mPixmaps.find(eSymbol);
   if (iter != mPixmaps.end())
   {
      return iter->second;
   }

   QPixmap pix(15, 15);
   pix.fill(Qt::white);

   QPainter p(&pix);
   p.setPen(QPen(Qt::black, 2));

   QRect rcSymbol = pix.rect();

   switch (eSymbol)
   {
      case SOLID: default:
         p.fillRect(rcSymbol, Qt::black);
         break;

      case X:
         p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
         p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
         break;

      case CROSS_HAIR:
         p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
            rcSymbol.center().y());
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         break;

      case ASTERISK:
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
            rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
         p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
            rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
         break;

      case VERTICAL_LINE:
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         break;

      case HORIZONTAL_LINE:
         p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
            rcSymbol.center().y());
         break;

      case FORWARD_SLASH:
         p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
         break;

      case BACK_SLASH:
         p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
         break;

      case BOX:
         p.drawRect(rcSymbol);
         break;

      case BOXED_X:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
         p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
         break;

      case BOXED_CROSS_HAIR:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
            rcSymbol.center().y());
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         break;

      case BOXED_ASTERISK:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         p.drawLine(rcSymbol.left(), rcSymbol.top() + (rcSymbol.height() / 4), rcSymbol.right(),
            rcSymbol.top() + ((rcSymbol.height() / 4) * 3));
         p.drawLine(rcSymbol.left(), rcSymbol.top() + ((rcSymbol.height() / 4) * 3),
            rcSymbol.right(), rcSymbol.top() + (rcSymbol.height() / 4));
         break;

      case BOXED_VERTICAL_LINE:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.center().x(), rcSymbol.top(), rcSymbol.center().x(),
            rcSymbol.bottom());
         break;

      case BOXED_HORIZONTAL_LINE:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.left(), rcSymbol.center().y(), rcSymbol.right(),
            rcSymbol.center().y());
         break;

      case BOXED_FORWARD_SLASH:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.topRight(), rcSymbol.bottomLeft());
         break;

      case BOXED_BACK_SLASH:
         p.drawRect(rcSymbol);
         p.drawLine(rcSymbol.topLeft(), rcSymbol.bottomRight());
         break;
   }

   p.end();
   pix.setMask(pix.createMaskFromColor(Qt::white));
   mPixmaps.insert(pair<SymbolType, QPixmap>(eSymbol, pix));
   return pix;
}

SymbolTypeButton::SymbolTypeButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   SymbolTypeGrid* pGrid = new SymbolTypeGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(this, SIGNAL(valueChanged(const QString&)), this, SLOT(translateChange())));
}

void SymbolTypeButton::setCurrentValue(SymbolType value)
{
   SymbolTypeGrid* pGrid = dynamic_cast<SymbolTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

SymbolType SymbolTypeButton::getCurrentValue() const
{
   SymbolType retValue;
   SymbolTypeGrid* pGrid = dynamic_cast<SymbolTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}

void SymbolTypeButton::setBorderedSymbols(bool show)
{
   SymbolTypeGrid* pGrid = dynamic_cast<SymbolTypeGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setBorderedSymbols(show);
   }
}

void SymbolTypeButton::translateChange()
{
   SymbolType symbol = getCurrentValue();
   emit valueChanged(symbol);
}
