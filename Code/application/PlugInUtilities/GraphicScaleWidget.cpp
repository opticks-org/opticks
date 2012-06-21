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
#include "GraphicScaleWidget.h"

#include <limits>
using namespace std;

GraphicScaleWidget::GraphicScaleWidget(QWidget* pParent) :
   QWidget(pParent)
{
   // Scale
   QLabel* pScaleLabel = new QLabel("Scale:", this);
   mpScaleSpin = new QDoubleSpinBox(this);
   mpScaleSpin->setRange(-1 * numeric_limits<double>::max(), numeric_limits<double>::max());
   mpScaleSpin->setDecimals(1);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pScaleLabel, 0, 0);
   pGrid->addWidget(mpScaleSpin, 0, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpScaleSpin, SIGNAL(valueChanged(double)), this, SIGNAL(scaleChanged(double))));
}

GraphicScaleWidget::~GraphicScaleWidget()
{
}

double GraphicScaleWidget::getScale() const
{
   return mpScaleSpin->value();
}

void GraphicScaleWidget::setScale(double scale)
{
   if (scale != getScale())
   {
      mpScaleSpin->setValue(scale);
   }
}
