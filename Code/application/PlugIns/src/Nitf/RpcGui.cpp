/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RpcGui.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

RpcGui::RpcGui(QWidget* pParent) :
   QWidget(pParent),
   mpDescriptor(NULL)
{
   QLabel* pHeightLabel = new QLabel("Height:", this);

   mpHeightSpin = new QSpinBox(this);
   mpHeightSpin->setMinimum(-418);     // Lowest dry-land point on earth in meters
   mpHeightSpin->setMaximum(8848);     // Highest point on earth in meters
   mpHeightSpin->setValue(0);
   mpHeightSpin->setSuffix(" m MSL");
   mpHeightSpin->setSingleStep(10);
   mpHeightSpin->setFixedWidth(80);
   mpHeightSpin->setWhatsThis("This is the value in meters above mean sea level");
   mpHeightSpin->setToolTip("This is the value in meters above mean sea level");

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(5);
   pGrid->addWidget(pHeightLabel, 0, 0);
   pGrid->addWidget(mpHeightSpin, 0, 1);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(2, 10);

   // Connections
   VERIFYNR(connect(mpHeightSpin, SIGNAL(valueChanged(int)), this, SLOT(setHeight(int))));
   mpDescriptor.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &RpcGui::georeferenceDescriptorModified));
}

RpcGui::~RpcGui()
{}

void RpcGui::setGeoreferenceDescriptor(GeoreferenceDescriptor* pDescriptor)
{
   if (pDescriptor != mpDescriptor.get())
   {
      mpDescriptor.reset(pDescriptor);

      // Update the widgets based on the georeference parameters
      updateFromGeoreferenceDescriptor();
   }
}

GeoreferenceDescriptor* RpcGui::getGeoreferenceDescriptor()
{
   return mpDescriptor.get();
}

const GeoreferenceDescriptor* RpcGui::getGeoreferenceDescriptor() const
{
   return mpDescriptor.get();
}

void RpcGui::georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   updateFromGeoreferenceDescriptor();
}

void RpcGui::updateFromGeoreferenceDescriptor()
{
   if (mpDescriptor.get() != NULL)
   {
      // Need to block signals when updating the widgets because DynamicObject::setAttributeByPath() does
      // not check if the attribute value is modified before setting the value and notifying the signal

      int heightValue =
         static_cast<int>(dv_cast<double>(mpDescriptor->getAttributeByPath("RPC Georeference/Height"), 0.0));
      mpHeightSpin->blockSignals(true);
      mpHeightSpin->setValue(heightValue);
      mpHeightSpin->blockSignals(false);
   }
}

void RpcGui::setHeight(int heightValue)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("RPC Georeference/Height", static_cast<double>(heightValue));
   }
}
