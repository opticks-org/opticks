/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "GcpGui.h"
#include "GeoreferenceUtilities.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "TypeConverter.h"

#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>

#include <vector>

GcpGui::GcpGui(int maxOrder, QWidget* pParent) :
   QWidget(pParent),
   mMaxOrder(maxOrder),
   mpDescriptor(NULL),
   mpRasterElement(NULL)
{
   // Gcp list combo
   mpGcpListLabel = new QLabel("GCP List:", this);
   mpGcpListCombo = new QComboBox(this);
   mpGcpListCombo->setEditable(false);

   // Polynomial order
   QLabel* pOrderLabel = new QLabel("Polynomial Order:", this);
   mpOrderLabel = new QLabel(this);
   mpOrderSpin = new QSpinBox(this);
   mpOrderSpin->setMinimum(1);
   mpOrderSpin->setMaximum(mMaxOrder);
   mpOrderSpin->setSingleStep(1);
   mpOrderSpin->setFixedWidth(50);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(0);
   pGrid->setSpacing(10);
   pGrid->addWidget(mpGcpListLabel, 0, 0);
   pGrid->addWidget(mpGcpListCombo, 0, 1);
   pGrid->addWidget(pOrderLabel, 1, 0);
   pGrid->addWidget(mpOrderSpin, 1, 1, Qt::AlignLeft);
   pGrid->addWidget(mpOrderLabel, 2, 0, 1, 2);
   pGrid->setRowStretch(3, 10);
   pGrid->setColumnStretch(1, 10);

   // Connections
   VERIFYNR(connect(mpGcpListCombo, SIGNAL(currentIndexChanged(const QString&)), this,
      SLOT(setGcpList(const QString&))));
   VERIFYNR(connect(mpOrderSpin, SIGNAL(valueChanged(int)), this, SLOT(setOrder(int))));

   mpDescriptor.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &GcpGui::georeferenceDescriptorModified));
}

GcpGui::~GcpGui()
{}

void GcpGui::setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, const std::list<GcpPoint>& gcps)
{
   // Do not update the widget if the data has not changed.  If the same Georeference descriptor is passed
   // in, update the widget if the GCP list combo is not visible, because it means that GcpList children
   // of a raster element are currently being displayed instead of a specific list of GCPs.
   if ((pDescriptor == mpDescriptor.get()) && (mpGcpListCombo->isVisibleTo(this) == false))
   {
      return;
   }

   mpDescriptor.reset(pDescriptor);
   mGcps = gcps;
   mpRasterElement = NULL;

   mpGcpListLabel->hide();
   mpGcpListCombo->clear();
   mpGcpListCombo->hide();

   // Update the widgets based on the georeference parameters
   updateFromGeoreferenceDescriptor();
}

void GcpGui::setGeoreferenceData(GeoreferenceDescriptor* pDescriptor, const RasterElement* pRaster)
{
   // Do not update the widget if the data has not changed.  If the same Georeference descriptor is passed
   // in, update the widget if the GCP list combo is visible, because it means that a specific list of
   // GCPs is currently displayed instead of GcpList children of a raster element.
   if ((pDescriptor == mpDescriptor.get()) && (mpGcpListCombo->isVisibleTo(this) == true))
   {
      return;
   }

   mpDescriptor.reset(pDescriptor);
   mGcps.clear();
   mpRasterElement = pRaster;

   mpGcpListLabel->show();
   mpGcpListCombo->show();
   mpGcpListCombo->clear();
   mpGcpListCombo->blockSignals(true);

   std::vector<std::string> gcpLists = Service<ModelServices>()->getElementNames(mpRasterElement,
      TypeConverter::toString<GcpList>());
   for (std::vector<std::string>::const_iterator iter = gcpLists.begin(); iter != gcpLists.end(); ++iter)
   {
      std::string gcpList = *iter;
      if (gcpList.empty() == false)
      {
         mpGcpListCombo->addItem(QString::fromStdString(gcpList));
      }
   }

   mpGcpListCombo->setCurrentIndex(-1);
   mpGcpListCombo->blockSignals(false);

   // Update the widgets based on the georeference parameters
   updateFromGeoreferenceDescriptor();
}

GeoreferenceDescriptor* GcpGui::getGeoreferenceDescriptor()
{
   return mpDescriptor.get();
}

const GeoreferenceDescriptor* GcpGui::getGeoreferenceDescriptor() const
{
   return mpDescriptor.get();
}

void GcpGui::georeferenceDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value)
{
   updateFromGeoreferenceDescriptor();
}

void GcpGui::updateFromGeoreferenceDescriptor()
{
   if (mpDescriptor.get() != NULL)
   {
      // Need to block signals when updating the widgets because DynamicObject::setAttributeByPath() does
      // not check if the attribute value is modified before setting the value and notifying the signal

      std::string gcpList = dv_cast<std::string>(mpDescriptor->getAttributeByPath("GCP Georeference/GcpListName"),
         std::string());
      if (gcpList.empty() == false)
      {
         int index = mpGcpListCombo->findText(QString::fromStdString(gcpList));
         mpGcpListCombo->blockSignals(true);
         mpGcpListCombo->setCurrentIndex(index);
         mpGcpListCombo->blockSignals(false);
      }
      else if (mpRasterElement != NULL)
      {
         // Select the first available GCP list, and do not block signals so that the GCP list name will be set
         // in the georeference descriptor
         mpGcpListCombo->setCurrentIndex(0);
      }

      int order = dv_cast<int>(mpDescriptor->getAttributeByPath("GCP Georeference/PolynomialOrder"), 1);
      mpOrderSpin->blockSignals(true);
      mpOrderSpin->setValue(order);
      mpOrderSpin->blockSignals(false);
   }

   updateOrderRange();
}

int GcpGui::getMaxOrder(int numGcps) const
{
   // numBasisValues = (order + 1) * (order + 2) / 2, solve for order
   // add fudge factor to prevent x.9999999 being truncated to x
   int maxOrder = static_cast<int>((-3.0 + sqrt(9.0 + 8.0 * (numGcps - 1))) / 2.0 + 0.00000001);
   if (maxOrder < 0)
   {
      maxOrder = 0;
   }

   if (maxOrder > mMaxOrder)
   {
      maxOrder = mMaxOrder;
   }

   return maxOrder;
}

void GcpGui::updateOrderRange()
{
   int maxOrder = 0;
   if (mpGcpListCombo->isVisibleTo(this) == true)
   {
      QString gcpList = mpGcpListCombo->currentText();
      if (gcpList.isEmpty() == false)
      {
         GcpList* pGcpList = dynamic_cast<GcpList*>(Service<ModelServices>()->getElement(gcpList.toStdString(),
            TypeConverter::toString<GcpList>(), mpRasterElement));
         if (pGcpList != NULL)
         {
            maxOrder = getMaxOrder(pGcpList->getSelectedPoints().size());
         }
      }
   }
   else if (mGcps.empty() == false)
   {
      maxOrder = getMaxOrder(static_cast<int>(mGcps.size()));
   }

   if (maxOrder <= 0)
   {
      mpOrderSpin->setRange(0, 0);
      mpOrderSpin->setEnabled(false);
      mpOrderSpin->setValue(0);
   }
   else
   {
      mpOrderSpin->setRange(1, maxOrder);
      mpOrderSpin->setEnabled(true);
      mpOrderSpin->setValue(maxOrder);
   }

   updateOrderLabel();
}

void GcpGui::updateOrderLabel()
{
   int gcpsRequired = 3;

   int order = mpOrderSpin->value();
   if (order > 0)
   {
      gcpsRequired = COEFFS_FOR_ORDER(order);
   }

   QString requiredText;
   if (mpGcpListCombo->isVisibleTo(this) == true)
   {
      QString gcpList = mpGcpListCombo->currentText();
      if (gcpList.isEmpty() == false)
      {
         GcpList* pGcpList = dynamic_cast<GcpList*>(Service<ModelServices>()->getElement(gcpList.toStdString(),
            TypeConverter::toString<GcpList>(), mpRasterElement));
         if (pGcpList != NULL)
         {
            int gcpsPresent = static_cast<int>(pGcpList->getSelectedPoints().size());
            requiredText = QString("%1 GCPs required, %2 present").arg(gcpsRequired).arg(gcpsPresent);
         }
      }

      if (requiredText.isEmpty() == true)
      {
         requiredText = QString("%1 GCPs required, invalid GCP List").arg(gcpsRequired);
      }
   }

   if ((requiredText.isEmpty() == true) && (mGcps.empty() == false))
   {
      requiredText = QString("%1 GCPs required, %2 present").arg(gcpsRequired).arg(mGcps.size());
   }

   mpOrderLabel->setText(requiredText);
}

void GcpGui::setGcpList(const QString& gcpList)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("GCP Georeference/GcpListName", gcpList.toStdString());
   }

   updateOrderRange();
}

void GcpGui::setOrder(int order)
{
   if (mpDescriptor.get() != NULL)
   {
      mpDescriptor->setAttributeByPath("GCP Georeference/PolynomialOrder", order);
   }

   updateOrderLabel();
}
