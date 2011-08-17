/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include "ArcRegionComboBox.h"
#include "AppVerify.h"
#include "GraphicArcWidget.h"

GraphicArcWidget::GraphicArcWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Start angle
   QLabel* pStartLabel = new QLabel("Start Angle:", this);
   mpStartSpin = new QDoubleSpinBox(this);
   mpStartSpin->setRange(-360.0, 360.0);
   mpStartSpin->setDecimals(1);
   mpStartSpin->setSuffix(" Degrees");

   // Stop angle
   QLabel* pStopLabel = new QLabel("Stop Angle:", this);
   mpStopSpin = new QDoubleSpinBox(this);
   mpStopSpin->setRange(-360.0, 360.0);
   mpStopSpin->setDecimals(1);
   mpStopSpin->setSuffix(" Degrees");

   // Region
   QLabel* pRegionLabel = new QLabel("Region:", this);
   mpRegionCombo = new ArcRegionComboBox(this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pStartLabel, 0, 0);
   pGrid->addWidget(mpStartSpin, 0, 1);
   pGrid->addWidget(pStopLabel, 1, 0);
   pGrid->addWidget(mpStopSpin, 1, 1);
   pGrid->addWidget(pRegionLabel, 2, 0);
   pGrid->addWidget(mpRegionCombo, 2, 1);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpStartSpin, SIGNAL(valueChanged(double)), this, SIGNAL(startAngleChanged(double))));
   VERIFYNR(connect(mpStopSpin, SIGNAL(valueChanged(double)), this, SIGNAL(stopAngleChanged(double))));
   VERIFYNR(connect(mpRegionCombo, SIGNAL(valueChanged(ArcRegion)), this, SIGNAL(regionChanged(ArcRegion))));
}

GraphicArcWidget::~GraphicArcWidget()
{
}

double GraphicArcWidget::getStartAngle() const
{
   return mpStartSpin->value();
}

double GraphicArcWidget::getStopAngle() const
{
   return mpStopSpin->value();
}

ArcRegion GraphicArcWidget::getRegion() const
{
   return mpRegionCombo->getCurrentValue();
}

void GraphicArcWidget::setStartAngle(double angle)
{
   if (angle != getStartAngle())
   {
      mpStartSpin->setValue(angle);
   }
}

void GraphicArcWidget::setStopAngle(double angle)
{
   if (angle != getStopAngle())
   {
      mpStopSpin->setValue(angle);
   }
}

void GraphicArcWidget::setRegion(ArcRegion region)
{
   if (region != getRegion())
   {
      mpRegionCombo->setCurrentValue(region);
   }
}
