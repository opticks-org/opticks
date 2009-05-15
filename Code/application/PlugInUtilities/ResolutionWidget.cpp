/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QIntValidator>
#include <QtGui/QLayout>

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "ResolutionWidget.h"

ResolutionWidget::ResolutionWidget(QWidget* pParent) : QWidget(pParent)
{
   // Resolution section
   mpUseViewResolution = new QCheckBox("Use View Resolution", this);
   mpWidthLabel = new QLabel("Width", this);
   mpResolutionX = new QLineEdit(this);
   mpResolutionX->setStatusTip("The number of pixels in the image width");
   mpResolutionX->setToolTip("Image width");

   mpHeightLabel = new QLabel("Height", this);
   mpResolutionY = new QLineEdit(this);
   mpResolutionY->setStatusTip("The number of pixels in the image height");
   mpResolutionY->setToolTip("Image height");
   QIntValidator* pValidator = new QIntValidator(this);
   pValidator->setBottom(2);
   mpResolutionX->setValidator(pValidator);
   mpResolutionY->setValidator(pValidator);


   mpAspectSizeGroup = new QGroupBox("Copy Method",this);
   mpFixedSizeRadio = new QRadioButton("Fixed Size", mpAspectSizeGroup);
   mpFixedSizeRadio->setStatusTip("Copy this size without respect to the image's aspect ratio");
   mpFixedSizeRadio->setToolTip("Copy this size");
   mpBoundingBoxRadio = new QRadioButton("Bounding Box", mpAspectSizeGroup);
   mpBoundingBoxRadio->setStatusTip("Fit the maximum image size into this bounding box");
   mpBoundingBoxRadio->setToolTip("Bounding Box size");

   QVBoxLayout* pMethodLayout = new QVBoxLayout(mpAspectSizeGroup);
   pMethodLayout->addWidget(mpFixedSizeRadio);
   pMethodLayout->addWidget(mpBoundingBoxRadio);

   VERIFYNR(connect(mpUseViewResolution, SIGNAL(toggled(bool)), this, 
      SLOT(setUseViewResolution(bool))));
   VERIFYNR(connect(mpResolutionX, SIGNAL(editingFinished()), this, 
      SLOT(checkResolutionX())));
   VERIFYNR(connect(mpResolutionY, SIGNAL(editingFinished()), this, 
      SLOT(checkResolutionY())));

   QGridLayout* pResolutionLayout = new QGridLayout(this);
   pResolutionLayout->setMargin(0);
   pResolutionLayout->setSpacing(10);
   pResolutionLayout->addWidget(mpUseViewResolution, 0, 0, 1, 3);
   pResolutionLayout->addWidget(mpWidthLabel, 1, 1);
   pResolutionLayout->addWidget(mpResolutionX, 1, 2);
   pResolutionLayout->addWidget(mpHeightLabel, 2, 1);
   pResolutionLayout->addWidget(mpResolutionY, 2, 2);
   pResolutionLayout->addWidget(mpAspectSizeGroup, 0, 3, 4, 1);
   pResolutionLayout->setColumnStretch(4, 10);
   pResolutionLayout->setColumnMinimumWidth(0, 10);
}

ResolutionWidget::~ResolutionWidget()
{
}


void ResolutionWidget::getResolution(unsigned int &width, unsigned int &height) const
{
   width = mpResolutionX->text().toUInt();
   height = mpResolutionY->text().toUInt();
}

void ResolutionWidget::setResolution(unsigned int width, unsigned int height)
{
   if (width == 0 && height == 0)
   {
      mpUseViewResolution->setChecked(true);
      return;
   }
   mpUseViewResolution->setChecked(false);
   const QValidator* pValidX = mpResolutionX->validator();
   const QValidator* pValidY = mpResolutionY->validator();
   int pos1 = 0;
   int pos2 = 0;
   if ((pValidX == NULL || pValidX->validate(QString::number(width), pos1) == QValidator::Acceptable) &&
      (pValidY == NULL || pValidY->validate(QString::number(height), pos2) == QValidator::Acceptable))
   {
      mpResolutionX->setText(QString::number(width));
      mpResolutionY->setText(QString::number(height));
      checkResolutionX(true);
      checkResolutionY(true);
      getResolution(mCurrentResolutionX, mCurrentResolutionY);
   }
}

void ResolutionWidget::setUseViewResolution(bool state)
{
   mpUseViewResolution->setChecked(state);
   mpResolutionX->setEnabled(!state);
   mpResolutionY->setEnabled(!state);
   mpFixedSizeRadio->setEnabled(!state);
   mpBoundingBoxRadio->setEnabled(!state);
}

void ResolutionWidget::checkResolutionX(bool ignoreAspectRatio)
{
   if (mpUseViewResolution->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionX->text().toUInt();
   if ((val % 2) != 0)
   {
      mpResolutionX->setText(QString::number(val + 1));
   }
}

void ResolutionWidget::checkResolutionY(bool ignoreAspectRatio)
{
   if (mpUseViewResolution->isChecked())
   {
      return;
   }
   unsigned int val = mpResolutionY->text().toUInt();
   if ((val % 2) != 0)
   {
      mpResolutionY->setText(QString::number(val + 1));
   }
}

bool ResolutionWidget::getUseViewResolution()
{
   return mpUseViewResolution->isChecked();
}

bool ResolutionWidget::getAspectRatioLock()
{
   return mpBoundingBoxRadio->isChecked();
}

void ResolutionWidget::setAspectRatioLock(bool state)
{
   mpBoundingBoxRadio->setChecked(state);
   mpFixedSizeRadio->setChecked(!state);
}
