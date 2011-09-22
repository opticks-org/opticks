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

#include "AppVerify.h"
#include "GraphicTriangleWidget.h"

GraphicTriangleWidget::GraphicTriangleWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Apex
   QLabel* pApexLabel = new QLabel("Apex:", this);
   mpApexSpin = new QSpinBox(this);
   mpApexSpin->setRange(0, 100);
   mpApexSpin->setSuffix("%");

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pApexLabel, 0, 0);
   pGrid->addWidget(mpApexSpin, 0, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpApexSpin, SIGNAL(valueChanged(int)), this, SIGNAL(apexChanged(int))));
}

GraphicTriangleWidget::~GraphicTriangleWidget()
{
}

int GraphicTriangleWidget::getApex() const
{
   return mpApexSpin->value();
}

void GraphicTriangleWidget::setApex(int apex)
{
   if (apex != getApex())
   {
      mpApexSpin->setValue(apex);
   }
}
