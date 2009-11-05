/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicLineWidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QColor>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

#include "AppVerify.h"
#include "CustomColorButton.h"
#include "LineStyleComboBox.h"
#include "LineWidthComboBox.h"
#include "TypesFile.h"

GraphicLineWidget::GraphicLineWidget(QWidget *pParent) : QWidget(pParent)
{
   mpBorderCheck = new QCheckBox("Bordered", this);
   mpScaledCheck = new QCheckBox("Line widths scale with zoom", this);

   QLabel* pLineStyleLabel = new QLabel("Line Style:", this);
   mpLineStyleCombo = new LineStyleComboBox(this);
   mpLineStyleCombo->setFixedWidth(100);

   QLabel* pLineWidthLabel = new QLabel("Line Width:", this);
   mpLineWidthCombo = new LineWidthComboBox(this);

   QLabel* pLineColorLabel = new QLabel("Line Color:", this);
   mpLineColorButton = new CustomColorButton(this);
   mpLineColorButton->usePopupGrid(true);

   QGridLayout* pLineGrid = new QGridLayout(this);
   pLineGrid->setMargin(0);
   pLineGrid->setSpacing(5);

   pLineGrid->addWidget(mpBorderCheck, 0, 0, 1, 2);
   pLineGrid->addWidget(mpScaledCheck, 1, 0, 1, 2);
   pLineGrid->addWidget(pLineStyleLabel, 2, 0);
   pLineGrid->addWidget(mpLineStyleCombo, 2, 1, Qt::AlignLeft);
   pLineGrid->addWidget(pLineWidthLabel, 3, 0);
   pLineGrid->addWidget(mpLineWidthCombo, 3, 1, Qt::AlignLeft);
   pLineGrid->addWidget(pLineColorLabel, 4, 0);
   pLineGrid->addWidget(mpLineColorButton, 4, 1, Qt::AlignLeft);
   pLineGrid->setRowStretch(5, 10);
   pLineGrid->setColumnStretch(2, 10);

   VERIFYNR(connect(mpBorderCheck, SIGNAL(toggled(bool)), this, SIGNAL(stateChanged(bool))));
   VERIFYNR(connect(mpLineStyleCombo, SIGNAL(valueChanged(LineStyle)), this, SIGNAL(styleChanged(LineStyle))));
   VERIFYNR(connect(mpLineWidthCombo, SIGNAL(valueChanged(unsigned int)), this, SIGNAL(widthChanged(unsigned int))));
   VERIFYNR(connect(mpLineColorButton, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(colorChanged(const QColor&))));
   VERIFYNR(connect(mpScaledCheck, SIGNAL(toggled(bool)), this, SIGNAL(scaledChanged(bool))));
}

GraphicLineWidget::~GraphicLineWidget()
{
}

void GraphicLineWidget::setHideLineState(bool hide)
{
   mpBorderCheck->setHidden(hide);
}

bool GraphicLineWidget::getHideLineState() const
{
   return mpBorderCheck->isHidden();
}

void GraphicLineWidget::setLineState(bool lineState)
{
   if (lineState != getLineState())
   {
      mpBorderCheck->setChecked(lineState);
   }
}

bool GraphicLineWidget::getLineState() const
{
   return mpBorderCheck->isChecked();
}

void GraphicLineWidget::setLineStyle(LineStyle style)
{
   if (style != getLineStyle())
   {
      mpLineStyleCombo->setCurrentValue(style);
   }
}

LineStyle GraphicLineWidget::getLineStyle() const
{
   return mpLineStyleCombo->getCurrentValue();
}

void GraphicLineWidget::setLineWidth(unsigned int width)
{
   if (width != getLineWidth())
   {
      mpLineWidthCombo->setCurrentValue(width);
   }
}

unsigned int GraphicLineWidget::getLineWidth() const
{
   return mpLineWidthCombo->getCurrentValue();
}

void GraphicLineWidget::setLineColor(const ColorType &color)
{
   if (color != getLineColor())
   {
      mpLineColorButton->setColor(color);
   }
}

ColorType GraphicLineWidget::getLineColor() const
{
   return mpLineColorButton->getColorType();
}

void GraphicLineWidget::setLineScaled(bool scaled)
{
   if (scaled != getLineScaled())
   {
      mpScaledCheck->setChecked(scaled);
   }
}

bool GraphicLineWidget::getLineScaled() const
{
   return mpScaledCheck->isChecked();
}
