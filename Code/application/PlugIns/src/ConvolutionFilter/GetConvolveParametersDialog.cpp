/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "GetConvolveParametersDialog.h"
#include "Layer.h"
#include "LayerList.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"

#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>

GetConvolveParametersDialog::GetConvolveParametersDialog(SpatialDataView* pView,
                                                         RasterElement* pElement,
                                                         QWidget* pParent)
   : QDialog(pParent),
   mpView(pView)
{
   setWindowTitle("Convolution Parameters");

   mpAoiSelect = new QComboBox(this);
   mpAoiSelect->setEditable(false);
   mpAoiSelect->addItem("<no AOI>");
   std::vector<Layer*> layers;
   if (pView != NULL)
   {
      pView->getLayerList()->getLayers(AOI_LAYER, layers);
   }
   if (!layers.empty())
   {
      for (std::vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
      {
         Layer* pLayer = *layer;
         if (pLayer != NULL)
         {
            mpAoiSelect->addItem(QString::fromStdString(pLayer->getName()));
         }
      }
   }

   mpBandSelect = new QListWidget(this);
   mpBandSelect->setResizeMode(QListView::Adjust);
   mpBandSelect->setFlow(QListWidget::TopToBottom);
   mpBandSelect->setUniformItemSizes(true);
   mpBandSelect->setSelectionMode(QListWidget::ExtendedSelection);
   if (pElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
      VERIFYNRV(pDescriptor != NULL);
      const std::vector<DimensionDescriptor>& bands = pDescriptor->getBands();
      for (std::vector<DimensionDescriptor>::const_iterator bandIter = bands.begin();
           bandIter != bands.end();
           ++bandIter)
      {
         mpBandSelect->addItem(QString::fromStdString(RasterUtilities::getBandName(pDescriptor, *bandIter)));
      }
   }
   mpBandSelect->setWrapping(true);
   if (mpBandSelect->count() > 0)
   {
      mpBandSelect->item(0)->setSelected(true);
   }
   QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     Qt::Horizontal, this);

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->addWidget(new QLabel("Select spatial subset:", this), 1, 0);
   pLayout->addWidget(mpAoiSelect, 1, 1);
   pLayout->addWidget(new QLabel("Select band subset:", this), 2, 0);
   pLayout->addWidget(mpBandSelect, 3, 0, 1, 2);
   pLayout->setRowStretch(3, 1);
   pLayout->setColumnStretch(1, 1);
   pLayout->addWidget(pButtons, 4, 0, 1, 2);

   VERIFYNR(connect(pButtons, SIGNAL(accepted()), this, SLOT(accept())));
   VERIFYNR(connect(pButtons, SIGNAL(rejected()), this, SLOT(reject())));
}

GetConvolveParametersDialog::~GetConvolveParametersDialog()
{}

void GetConvolveParametersDialog::setSelectedAoi(const AoiElement* pElement)
{
   if (mpAoiSelect->count() < 0)
   {
      return;
   }
   if (pElement == NULL)
   {
      mpAoiSelect->setCurrentIndex(0);
   }
   std::vector<Layer*> layers;
   if (mpView != NULL)
   {
      mpView->getLayerList()->getLayers(AOI_LAYER, layers);
   }
   for (unsigned int i = 0; i < layers.size(); ++i)
   {
      Layer* pLayer = layers[i];
      if (pLayer == NULL)
      {
         continue;
      }
      AoiElement* pCurElement = dynamic_cast<AoiElement*>(pLayer->getDataElement());
      if (pCurElement == pElement && (i + 1 < static_cast<unsigned int>(mpAoiSelect->count())))
      {
         mpAoiSelect->setCurrentIndex(i + 1);
      }
   }

}

AoiElement* GetConvolveParametersDialog::getSelectedAoi() const
{
   int curIndex = mpAoiSelect->currentIndex();
   if (curIndex <= 0)
   {
      // no AOI selected
      return NULL;
   }
   std::vector<Layer*> layers;
   if (mpView != NULL)
   {
      mpView->getLayerList()->getLayers(AOI_LAYER, layers);
   }
   //curIndex - 1 is done since <no AOI> is always added.
   if (static_cast<unsigned int>(curIndex - 1) >= layers.size())
   {
      return NULL;
   }
   AoiLayer* pLayer = dynamic_cast<AoiLayer*>(layers[curIndex - 1]);
   if (pLayer != NULL)
   {
      return dynamic_cast<AoiElement*>(pLayer->getDataElement());
   }
   return NULL;
}

void GetConvolveParametersDialog::setBandSelectionIndices(const std::vector<unsigned int>& indices)
{
   if (mpBandSelect->count() < 0)
   {
      return;
   }
   mpBandSelect->clearSelection();
   for (std::vector<unsigned int>::const_iterator iter = indices.begin();
        iter != indices.end(); ++iter)
   {
      if (*iter < static_cast<unsigned int>(mpBandSelect->count()))
      {
         QListWidgetItem* pItem = mpBandSelect->item(*iter);
         if (pItem != NULL)
         {
            pItem->setSelected(true);
         }
      }
   }
}

std::vector<unsigned int> GetConvolveParametersDialog::getBandSelectionIndices() const
{
   std::vector<unsigned int> bands;
   bands.reserve(mpBandSelect->count());
   for (int i = 0; i < mpBandSelect->count(); ++i)
   {
      QListWidgetItem* pItem = mpBandSelect->item(i);
      if (pItem != NULL && pItem->isSelected())
      {
         bands.push_back(i);
      }
   }
   return bands;
}