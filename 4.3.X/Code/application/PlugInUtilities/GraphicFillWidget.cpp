/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QColor>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>

#include "AppVerify.h"
#include "CustomColorButton.h"
#include "FillStyleComboBox.h"
#include "GraphicFillWidget.h"
#include "SymbolTypeGrid.h"

GraphicFillWidget::GraphicFillWidget(QWidget *pParent) : QWidget(pParent)
{
   QLabel* pFillStyleLabel = new QLabel("Fill Style:", this);
   mpFillStyleCombo = new FillStyleComboBox(this);
   mpFillStyleCombo->setFixedWidth(100);

   QLabel* pHatchLabel = new QLabel("Hatch:", this);
   mpHatchStyle = new SymbolTypeButton(this);
   mpHatchStyle->setBorderedSymbols(false);

   QLabel* pFillColorLabel = new QLabel("Fill Color:", this);
   mpFillColorButton = new CustomColorButton(this);
   mpFillColorButton->usePopupGrid(true);

   QGridLayout* pFillGrid = new QGridLayout(this);
   pFillGrid->setMargin(0);
   pFillGrid->setSpacing(5);
   pFillGrid->addWidget(pFillStyleLabel, 0, 0);
   pFillGrid->addWidget(mpFillStyleCombo, 0, 1, Qt::AlignLeft);
   pFillGrid->addWidget(pHatchLabel, 1, 0);
   pFillGrid->addWidget(mpHatchStyle, 1, 1, Qt::AlignLeft);
   pFillGrid->addWidget(pFillColorLabel, 2, 0);
   pFillGrid->addWidget(mpFillColorButton, 2, 1, Qt::AlignLeft);
   pFillGrid->setRowStretch(3, 10);
   pFillGrid->setColumnStretch(2, 10);

   VERIFYNR(connect(mpFillStyleCombo, SIGNAL(valueChanged(FillStyle)), this, SIGNAL(styleChanged(FillStyle))));
   VERIFYNR(connect(mpHatchStyle, SIGNAL(valueChanged(SymbolType)), this, SIGNAL(hatchChanged(SymbolType))));
   VERIFYNR(connect(mpFillColorButton, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(colorChanged(const QColor&))));
}

GraphicFillWidget::~GraphicFillWidget()
{
}

void GraphicFillWidget::setFillColor(const QColor& color)
{
   if (color != getFillColor())
   {
      mpFillColorButton->setColor(color);
   }
}

QColor GraphicFillWidget::getFillColor() const
{
   return mpFillColorButton->getColor();
}

void GraphicFillWidget::setFillStyle(FillStyle style)
{
   if (style != getFillStyle())
   {
      mpFillStyleCombo->setCurrentValue(style);
   }
}

FillStyle GraphicFillWidget::getFillStyle() const
{
   return mpFillStyleCombo->getCurrentValue();
}

void GraphicFillWidget::setHatchStyle(SymbolType hatch)
{
   if (hatch != getHatchStyle())
   {
      mpHatchStyle->setCurrentValue(hatch);
   }
}

SymbolType GraphicFillWidget::getHatchStyle() const
{
   return mpHatchStyle->getCurrentValue();
}
