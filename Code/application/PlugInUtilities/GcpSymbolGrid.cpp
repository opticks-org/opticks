/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GcpSymbolGrid.h"

#include "AppAssert.h"
#include "AppVerify.h"
#include "Icons.h"
#include "StringUtilities.h"

#include <QtGui/QBitmap>
#include <QtGui/QPainter>

#include <string>
#include <vector>

using namespace std;

GcpSymbolGrid::GcpSymbolGrid(QWidget* pParent)
: PixmapGrid(pParent)
{
   setNumRows(1);
   setNumColumns(3);

   Icons* pIcons = Icons::instance();
   REQUIRE(pIcons != NULL);
   setPixmap(0, 0, pIcons->mGcpX,
      QString::fromStdString(StringUtilities::toXmlString(GCP_X)),
      QString::fromStdString(StringUtilities::toDisplayString(GCP_X)));
   setPixmap(0, 1, pIcons->mGcpPlus,
      QString::fromStdString(StringUtilities::toXmlString(GCP_PLUS)),
      QString::fromStdString(StringUtilities::toDisplayString(GCP_PLUS)));
   setPixmap(0, 2, pIcons->mGcpBlank,
      QString::fromStdString(StringUtilities::toXmlString(GCP_NODRAW)),
      QString::fromStdString(StringUtilities::toDisplayString(GCP_NODRAW)));

   // Set the current symbol
   setSelectedPixmap(QString::fromStdString(StringUtilities::toXmlString(GCP_X)));

   VERIFYNR(connect(this, SIGNAL(pixmapSelected(const QString&)), this, SLOT(translateChange(const QString&))));
}

void GcpSymbolGrid::setCurrentValue(GcpSymbol value)
{
   QString strValue = QString::fromStdString(StringUtilities::toXmlString(value));
   setSelectedPixmap(strValue);
}

GcpSymbol GcpSymbolGrid::getCurrentValue() const
{
   GcpSymbol retValue;
   string curText = getSelectedPixmapIdentifier().toStdString();
   if (!curText.empty())
   {
      retValue = StringUtilities::fromXmlString<GcpSymbol>(curText);
   }
   return retValue;
}

void GcpSymbolGrid::translateChange(const QString& strText)
{
   GcpSymbol curType = StringUtilities::fromXmlString<GcpSymbol>(strText.toStdString());
   emit valueChanged(curType);
}

GcpSymbolButton::GcpSymbolButton(QWidget* pParent) : 
   PixmapGridButton(pParent)
{
   setSyncIcon(true);
   GcpSymbolGrid* pGrid = new GcpSymbolGrid(this);
   setPixmapGrid(pGrid);
   VERIFYNR(connect(this, SIGNAL(valueChanged(const QString&)), this, SLOT(translateChange())));
}

void GcpSymbolButton::setCurrentValue(GcpSymbol value)
{
   GcpSymbolGrid* pGrid = dynamic_cast<GcpSymbolGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      pGrid->setCurrentValue(value);
   }
}

GcpSymbol GcpSymbolButton::getCurrentValue() const
{
   GcpSymbol retValue;
   GcpSymbolGrid* pGrid = dynamic_cast<GcpSymbolGrid*>(getPixmapGrid());
   if (pGrid != NULL)
   {
      retValue = pGrid->getCurrentValue();
   }
   return retValue;
}

void GcpSymbolButton::translateChange()
{
   GcpSymbol symbol = getCurrentValue();
   emit valueChanged(symbol);
}
