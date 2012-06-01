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

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

SampleGeorefGui::SampleGeorefGui(QWidget* pParent) :
   QWidget(pParent),
   mpDescriptor(NULL)
{
   QLabel* pXLabel = new QLabel("X Size:", this);
   QLabel* pYLabel = new QLabel("Y Size:", this);

   mpXSpin = new QSpinBox(this);
   mpXSpin->setMinimum(0);
   mpXSpin->setMaximum(90);
   mpXSpin->setValue(10);

   mpYSpin = new QSpinBox(this);
   mpYSpin->setMinimum(0);
   mpYSpin->setMaximum(180);
   mpYSpin->setValue(5);

   mpExtrapolateCheck = new QCheckBox("Extrapolate", this);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pXLabel, 0, 0);
   pGrid->addWidget(mpXSpin, 0, 1);
   pGrid->addWidget(pYLabel, 1, 0);
   pGrid->addWidget(mpYSpin, 1, 1);
   pGrid->addWidget(mpExtrapolateCheck, 2, 1);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpXSpin, SIGNAL(valueChanged(int)), this, SLOT(setXSize(int))));
   VERIFYNR(connect(mpYSpin, SIGNAL(valueChanged(int)), this, SLOT(setYSize(int))));
   VERIFYNR(connect(mpExtrapolateCheck, SIGNAL(toggled(bool)), this, SLOT(setExtrapolate(bool))));

   mpDescriptor.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &SampleGeorefGui::georeferenceDescriptorModified));
}

SampleGeorefGui::~SampleGeorefGui()
{}

void SampleGeorefGui::setGeoreferenceDescriptor(GeoreferenceDescriptor* pDescriptor)
{
   if (pDescriptor != mpDescriptor.get())
   {
      mpDescriptor.reset(pDescriptor);

      // Update the widgets based on the georeference parameters
      updateFromGeoreferenceDescriptor();
   }
}

GeoreferenceDescriptor* SampleGeorefGui::getGeoreferenceDescriptor()
{
   return mpDescriptor.get();
}

const GeoreferenceDescriptor* SampleGeorefGui::getGeoreferenceDescriptor() const
{
   return mpDescriptor.get();
}

void SampleGeorefGui::georeferenceDescriptorModified(Subject& subject, const std::string& signal,
                                                     const boost::any& value)
{
   updateFromGeoreferenceDescriptor();
}

void SampleGeorefGui::updateFromGeoreferenceDescriptor()
{
   if (mpDescriptor.get() != NULL)
   {
      // Need to block signals when updating the widgets because DynamicObject::setAttributeByPath() does
      // not check if the attribute value is modified before setting the value and notifying the signal

      int xSize = dv_cast<int>(mpDescriptor->getAttributeByPath("SampleGeoref/XSize"), 10);
      mpXSpin->blockSignals(true);
      mpXSpin->setValue(xSize);
      mpXSpin->blockSignals(false);

      int ySize = dv_cast<int>(mpDescriptor->getAttributeByPath("SampleGeoref/YSize"), 5);
      mpYSpin->blockSignals(true);
      mpYSpin->setValue(ySize);
      mpYSpin->blockSignals(false);

      bool extrapolate = dv_cast<bool>(mpDescriptor->getAttributeByPath("SampleGeoref/Extrapolate"), false);
      mpExtrapolateCheck->blockSignals(true);
      mpExtrapolateCheck->setChecked(extrapolate);
      mpExtrapolateCheck->blockSignals(false);
   }
}

void SampleGeorefGui::setXSize(int xSize)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("SampleGeoref/XSize", xSize);
   }
}

void SampleGeorefGui::setYSize(int ySize)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("SampleGeoref/YSize", ySize);
   }
}

void SampleGeorefGui::setExtrapolate(bool extrapolate)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("SampleGeoref/Extrapolate", extrapolate);
   }
}
