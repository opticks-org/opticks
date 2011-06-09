/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GraphicMeasurementWidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

#include "AppVerify.h"
#include "TypesFile.h"

GraphicMeasurementWidget::GraphicMeasurementWidget(QWidget *pParent) : QWidget(pParent)
{
   QLabel* pDistanceLabel = new QLabel("Distance Precision:", this);
   mpDistancePrecisionSpin = new QSpinBox(this);
   mpDistancePrecisionSpin->setRange(0, 10);

   QLabel* pBearingLabel = new QLabel("Bearing Precision:", this);
   mpBearingPrecisionSpin = new QSpinBox(this);
   mpBearingPrecisionSpin->setRange(0, 10);

   QLabel* pEndPointsLabel = new QLabel("End Points Precision:", this);
   mpEndPointsPrecisionSpin = new QSpinBox(this);
   mpEndPointsPrecisionSpin->setRange(0, 10);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);

   pGrid->addWidget(pBearingLabel, 0, 0);
   pGrid->addWidget(mpBearingPrecisionSpin, 0, 1);
   pGrid->addWidget(pDistanceLabel, 1, 0);
   pGrid->addWidget(mpDistancePrecisionSpin, 1, 1);
   pGrid->addWidget(pEndPointsLabel, 2, 0);
   pGrid->addWidget(mpEndPointsPrecisionSpin, 2, 1);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(2, 10);

   VERIFYNR(connect(mpDistancePrecisionSpin, SIGNAL(valueChanged(int)), this, 
      SIGNAL(distancePrecisionChanged(int))));
   VERIFYNR(connect(mpBearingPrecisionSpin, SIGNAL(valueChanged(int)), this, 
      SIGNAL(bearingPrecisionChanged(int))));
   VERIFYNR(connect(mpEndPointsPrecisionSpin, SIGNAL(valueChanged(int)), this, 
      SIGNAL(endPointsPrecisionChanged(int))));
}

GraphicMeasurementWidget::~GraphicMeasurementWidget()
{
}

void GraphicMeasurementWidget::setDistancePrecision(int precision)
{
   if (precision != getDistancePrecision())
   {
      mpDistancePrecisionSpin->setValue(precision);
   }
}

int GraphicMeasurementWidget::getDistancePrecision() const
{
   return mpDistancePrecisionSpin->value();
}

void GraphicMeasurementWidget::setBearingPrecision(int precision)
{
   if (precision != getBearingPrecision())
   {
      mpBearingPrecisionSpin->setValue(precision);
   }
}

int GraphicMeasurementWidget::getBearingPrecision() const
{
   return mpBearingPrecisionSpin->value();
}

void GraphicMeasurementWidget::setEndPointsPrecision(int precision)
{
   if (precision != getEndPointsPrecision())
   {
      mpEndPointsPrecisionSpin->setValue(precision);
   }
}

int GraphicMeasurementWidget::getEndPointsPrecision() const
{
   return mpEndPointsPrecisionSpin->value();
}

