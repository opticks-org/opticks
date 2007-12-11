/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "SampleGeorefGui.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>

#include <limits>

SampleGeorefGui::SampleGeorefGui(void)
{
   QLabel *pXLabel = new QLabel("X Size:", this);
   QLabel *pYLabel = new QLabel("Y Size:", this);

   mpXSpin = new QSpinBox(this);
   mpXSpin->setMinimum(0);
   mpXSpin->setMaximum(90);
   mpXSpin->setValue(10);
   mpYSpin = new QSpinBox(this);
   mpYSpin->setMinimum(0);
   mpYSpin->setMaximum(180);
   mpYSpin->setValue(5);

   mpAnimatedCheck = new QCheckBox("Animated", this);
   
   QRadioButton *pTranslateButton = new QRadioButton("Translate", this);
   mpRotateButton = new QRadioButton("Rotate", this);
   pTranslateButton->setChecked(true);
   pTranslateButton->setEnabled(false);
   mpRotateButton->setEnabled(false);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pXLabel, 0, 0);
   pGrid->addWidget(pYLabel, 1, 0);
   pGrid->addWidget(mpXSpin, 0, 1);
   pGrid->addWidget(mpYSpin, 1, 1);
   pGrid->addWidget(mpAnimatedCheck, 2, 1);
   pGrid->addWidget(pTranslateButton, 3, 1);
   pGrid->addWidget(mpRotateButton, 4, 1);

   pGrid->setRowStretch(5, 10);
   pGrid->setColumnStretch(2, 10);

   VERIFYNR(connect(mpAnimatedCheck, SIGNAL(toggled(bool)), pTranslateButton, SLOT(setEnabled(bool))));
   VERIFYNR(connect(mpAnimatedCheck, SIGNAL(toggled(bool)), mpRotateButton, SLOT(setEnabled(bool))));
}

SampleGeorefGui::~SampleGeorefGui(void)
{
}

int SampleGeorefGui::getXSize() const
{
   return mpXSpin->value();
}

int SampleGeorefGui::getYSize() const
{
   return mpYSpin->value();
}

bool SampleGeorefGui::getAnimated() const
{
   return mpAnimatedCheck->isChecked();
}

bool SampleGeorefGui::getRotate() const
{
   return mpRotateButton->isChecked();
}