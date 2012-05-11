/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

#include "RpcGeoreference.h"
#include "RpcGui.h"
#include "RasterElement.h"

using namespace std;

RpcGui::RpcGui(RasterElement* pRasterElement, QWidget* pParent) :
   QWidget(pParent),
   mpRasterElement(pRasterElement)
{
   QLabel* pHeightLabel = new QLabel("Height:", this);

   mpHeightSpin = new QSpinBox(this);
   // Lowest dry-land point on earth in meters
   mpHeightSpin->setMinimum(-418);
   // Highest point on earth in meters
   mpHeightSpin->setMaximum(8848);
   mpHeightSpin->setValue(0);
   mpHeightSpin->setSuffix(" m MSL");
   mpHeightSpin->setSingleStep(10);
   mpHeightSpin->setFixedWidth(80);
   mpHeightSpin->setWhatsThis("This is the value in meters above mean sea level");
   mpHeightSpin->setToolTip("This is the value in meters above mean sea level");

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(6);
   pGrid->addWidget(pHeightLabel, 1, 0);
   pGrid->addWidget(mpHeightSpin, 1, 1);
   pGrid->setRowStretch(2, 10);
   pGrid->setColumnStretch(2, 10);
}

RpcGui::~RpcGui()
{}

int RpcGui::getHeightSize() const
{
   return mpHeightSpin->value();
}
