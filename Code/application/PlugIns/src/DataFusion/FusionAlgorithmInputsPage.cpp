/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "ColorType.h"
#include "DesktopServices.h"
#include "FusionAlgorithmInputsPage.h"
#include "GraphicGroup.h"
#include "LayerList.h"
#include "LocationType.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "Slot.h"
#include "SpatialDataView.h"

using namespace std;

const string FusionAlgorithmInputsPage::FUSION_ROI_NAME = "Fusion Region";

FusionAlgorithmInputsPage::FusionAlgorithmInputsPage(QWidget* pParent) :
   FusionPage(pParent),
   mpAoiLayer(NULL)
{
   // ROI Information
   QGroupBox* pGroup = new QGroupBox("ROI Bounding Box", this);

   QLabel* pMinX = new QLabel("Minimum X:", pGroup);
   mpCurrentMinX = new QLabel(pGroup);

   QLabel* pMinY = new QLabel("Minimum Y:", pGroup);
   mpCurrentMinY = new QLabel(pGroup);

   QLabel* pMaxX = new QLabel("Maximum X:", pGroup);
   mpCurrentMaxX = new QLabel(pGroup);

   QLabel* pMaxY = new QLabel("Maximum Y:", pGroup);
   mpCurrentMaxY = new QLabel(pGroup);

   QGridLayout* pGroupLayout = new QGridLayout(pGroup);
   pGroupLayout->setMargin(10);
   pGroupLayout->setSpacing(5);
   pGroupLayout->addWidget(pMinX, 0, 0);
   pGroupLayout->addWidget(mpCurrentMinX, 0, 1);
   pGroupLayout->addWidget(pMinY, 0, 2);
   pGroupLayout->addWidget(mpCurrentMinY, 0, 3);
   pGroupLayout->addWidget(pMaxX, 1, 0);
   pGroupLayout->addWidget(mpCurrentMaxX, 1, 1);
   pGroupLayout->addWidget(pMaxY, 1, 2);
   pGroupLayout->addWidget(mpCurrentMaxY, 1, 3);
   pGroupLayout->setColumnStretch(1, 10);
   pGroupLayout->setColumnStretch(3, 10);

   // Available Views
   mpProductGroup = new QGroupBox("Available Views", this);

   mpFlickerOption = new QCheckBox("Flicker / Blend", mpProductGroup);
   mpRunOverlayOption = new QCheckBox("Run Flicker Tools when done", mpProductGroup);
   mpRunOverlayOption->setEnabled(false);
   mpSbsOption = new QCheckBox("Side-by-Side", mpProductGroup);

   QGridLayout* pViewLayout = new QGridLayout(mpProductGroup);
   pViewLayout->setMargin(10);
   pViewLayout->setSpacing(5);
   pViewLayout->addWidget(mpFlickerOption, 0, 0);
   pViewLayout->addWidget(mpRunOverlayOption, 0, 1, Qt::AlignRight);
   pViewLayout->addWidget(mpSbsOption, 1, 0);
   pViewLayout->setColumnStretch(1, 10);

   // color map copying
   QGroupBox* pColorMapGroup = new QGroupBox("Color Maps to Copy", this);
   QVBoxLayout* pColorLayout = new QVBoxLayout(pColorMapGroup);

   mpPrimaryCheck = new QCheckBox("Primary Chip", pColorMapGroup);
   mpSecondaryCheck = new QCheckBox("Secondary Chip", pColorMapGroup);

   pColorLayout->setMargin(10);
   pColorLayout->setSpacing(5);
   pColorLayout->addWidget(mpPrimaryCheck);
   pColorLayout->addWidget(mpSecondaryCheck);

   QString tip = "Copies the color map from the primary or secondary image to the respective image\n"
      "chip after fusion. All histogram stretches are preserved in the chip's color\nmap.";
   pColorMapGroup->setToolTip(tip);

   // Optimization check box
   mpOnDiskButton = new QCheckBox("Optimize for memory", this);

   // Execute button
   mpExecuteButton = new QPushButton("Execute", this);

   // Layout
   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addWidget(pGroup, 0, 0, 1, 2);
   pLayout->addWidget(mpProductGroup, 1, 0, 1, 2);
   pLayout->addWidget(pColorMapGroup, 2, 0, 1, 2);
   pLayout->addWidget(mpOnDiskButton, 3, 0, Qt::AlignLeft);
   pLayout->addWidget(mpExecuteButton, 3, 1, Qt::AlignRight);
   pLayout->setRowStretch(4, 10);

   // Connections
   connect(this, SIGNAL(modified()), this, SLOT(enableFusion()));
   connect(mpOnDiskButton, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   connect(mpExecuteButton, SIGNAL(clicked()), this, SIGNAL(executeAlgorithm()));
   connect(mpFlickerOption, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   connect(mpRunOverlayOption, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   connect(mpSbsOption, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   connect(mpPrimaryCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
   connect(mpSecondaryCheck, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
}

FusionAlgorithmInputsPage::~FusionAlgorithmInputsPage()
{
   setAoiLayer(NULL);
   setViews(NULL, NULL);
}

bool FusionAlgorithmInputsPage::copyColormap(const SpatialDataView& view)
{
   if (&view == getPrimaryView())
   {
      return mpPrimaryCheck->isChecked();
   }
   else if (&view == getSecondaryView())
   {
      return mpSecondaryCheck->isChecked();
   }
   return false;
}

void FusionAlgorithmInputsPage::attached(Subject& subject, const string& signal, const Slot& slot)
{
   aoiLayerAttached(subject, signal, boost::any());
}

void FusionAlgorithmInputsPage::detached(Subject& subject, const string& signal, const Slot& slot)
{
   aoiLayerDetached(subject, signal, boost::any());
}

void FusionAlgorithmInputsPage::aoiLayerAttached(Subject& subject, const string& signal, const boost::any& v)
{
   AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(&subject);
   if (pAoiLayer != NULL && mpAoiLayer == pAoiLayer)
   {
      DataElement* pElement = pAoiLayer->getDataElement();
      if (pElement != NULL)
      {
         pElement->attach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionAlgorithmInputsPage::aoiModified));
      }
   }
}

void FusionAlgorithmInputsPage::aoiLayerDetached(Subject& subject, const string& signal, const boost::any& v)
{
   AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(&subject);
   if (pAoiLayer != NULL && mpAoiLayer == pAoiLayer)
   {
      DataElement* pElement = pAoiLayer->getDataElement();
      if (pElement != NULL)
      {
         pElement->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionAlgorithmInputsPage::aoiModified));
      }
   }
}

void FusionAlgorithmInputsPage::aoiLayerDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   AoiLayer* pAoiLayer = dynamic_cast<AoiLayer*>(&subject);
   if (pAoiLayer != NULL && mpAoiLayer == pAoiLayer)
   {
      DataElement* pElement = pAoiLayer->getDataElement();
      if (pElement != NULL)
      {
         pElement->detach(SIGNAL_NAME(Subject, Modified), Slot(this, &FusionAlgorithmInputsPage::aoiModified));
      }

      mpAoiLayer = NULL;

      SpatialDataView* pView = getPrimaryView();
      if (pView != NULL)
      {
         AoiLayer* pNewLayer = static_cast<AoiLayer*>(pAoiLayer->copy(false));
         VERIFYNRV(pNewLayer != NULL);

         AoiElement* pNewLayerAoi = dynamic_cast<AoiElement*>(pNewLayer->getDataElement());
         VERIFYNRV(pNewLayerAoi != NULL);

         pView->addLayer(pNewLayer);
         setAoiLayer(pNewLayer);

         pNewLayerAoi->clearPoints();
      }
   }
}

void FusionAlgorithmInputsPage::aoiModified(Subject& subject, const string& signal, const boost::any& v)
{
   AoiElement* pAoi = dynamic_cast<AoiElement*>(&subject);
   if (pAoi != NULL)
   {
      LocationType ll = pAoi->getGroup()->getLlCorner();
      LocationType ur = pAoi->getGroup()->getUrCorner();
      int x1 = std::min(ll.mX, ur.mX);
      int x2 = std::max(ll.mX, ur.mX);
      int y1 = std::min(ll.mY, ur.mY);
      int y2 = std::max(ll.mY, ur.mY);
      if (!pAoi->getGroup()->getObjects().empty())
      {
         // if the AOI has elements, reset the bounding box to start from 1 instead of 0
         x1++;
         y1++;
         x2++;
         y2++;
      }
      mpCurrentMinX->setText(QString::number(x1));
      mpCurrentMinY->setText(QString::number(y1));
      mpCurrentMaxX->setText(QString::number(x2));
      mpCurrentMaxY->setText(QString::number(y2));
      emit modified();
   }
}

bool FusionAlgorithmInputsPage::isValid() const
{
   SpatialDataView* pPrimary = getPrimaryView();
   SpatialDataView* pSecondary = getSecondaryView();
   bool bValidRoi = false;
   if (mpAoiLayer != NULL)
   {
      AoiElement* pAoi = static_cast<AoiElement*>(mpAoiLayer->getDataElement());
      if (pAoi != NULL)
      {
         bValidRoi = !pAoi->getGroup()->getObjects().empty();
      }
   }
   return pPrimary != NULL && pSecondary != NULL && pPrimary != pSecondary && bValidRoi && (flicker() || sbs());
}

void FusionAlgorithmInputsPage::setViews(SpatialDataView* pPrimary, SpatialDataView* pSecondary)
{
   FusionPage::setViews(pPrimary, pSecondary);
   createFusionAoiLayer(pPrimary);
}

bool FusionAlgorithmInputsPage::sbs() const
{
   return mpSbsOption != NULL && mpSbsOption->isChecked();
}

bool FusionAlgorithmInputsPage::flicker() const
{
   return mpFlickerOption != NULL && mpFlickerOption->isChecked();
}

bool FusionAlgorithmInputsPage::openOverlayTools() const
{
   return mpRunOverlayOption != NULL && mpRunOverlayOption->isChecked();
}

bool FusionAlgorithmInputsPage::getRoiBoundingBox(int& x1, int& y1, int& x2, int& y2,
                                                  int rasterBbX1, int rasterBbY1,
                                                  int rasterBbX2, int rasterBbY2) const
{
   bool success = false;
   if (mpAoiLayer != NULL)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(mpAoiLayer->getDataElement());
      VERIFY(pAoi != NULL); // the AOI's DataElement should always be an AOI
      const BitMask* pMask = pAoi->getSelectedPoints();
      VERIFY(pMask != NULL); // an AOI should always have a valid BitMask
      BitMaskIterator maskIter(pMask, rasterBbX1, rasterBbY1, rasterBbX2, rasterBbY2);
      maskIter.getBoundingBox(x1, y1, x2, y2);
      success = true;
   }
   return success;
}

bool FusionAlgorithmInputsPage::inMemory() const
{
   return mpOnDiskButton != NULL && mpOnDiskButton->isChecked() == false;
}

string FusionAlgorithmInputsPage::getPreferredPrimaryMouseMode() const
{
   return "LayerMode";
}

Layer* FusionAlgorithmInputsPage::getPreferredPrimaryActiveLayer() const
{
   return mpAoiLayer;
}

void FusionAlgorithmInputsPage::createFusionAoiLayer(SpatialDataView* pView)
{
   if (pView == NULL)
   {
      if (mpAoiLayer != NULL)
      {
         mpAoiLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionAlgorithmInputsPage::aoiLayerDeleted));
      }
   }
   else
   {
      Service<DesktopServices> pDesktop;
      if (pDesktop.get() != NULL)
      {
         VERIFYNRV(pView != NULL);
         LayerList* pLayerList = pView->getLayerList();
         VERIFYNRV(pLayerList != NULL);

         SpatialDataView* pAoiView = NULL;
         if (mpAoiLayer != NULL)
         {
            pAoiView = dynamic_cast<SpatialDataView*>(mpAoiLayer->getView());
         }

         // member layer does not exist or the member layer is on the wrong view
         if (mpAoiLayer == NULL || (pAoiView != NULL && pAoiView != pView))
         {
            AoiLayer* pFusionAoiLayer = NULL;
            RasterElement* pRaster = pLayerList->getPrimaryRasterElement();
            VERIFYNRV(pRaster != NULL);
            Service<ModelServices> pModel;
            VERIFYNRV(pModel.get() != NULL);
            AoiElement* pAoi = static_cast<AoiElement*>(pModel->getElement(FUSION_ROI_NAME, "AoiElement", pRaster));
            if (pAoi == NULL)
            {
               DataDescriptor* pDescriptor = pModel->createDataDescriptor(FUSION_ROI_NAME, "AoiElement", pRaster);
               if (pDescriptor != NULL)
               {
                  pAoi = static_cast<AoiElement*>(pModel->createElement(pDescriptor));
               }
            }
            if (pAoi != NULL)
            {
               pFusionAoiLayer = static_cast<AoiLayer*>(pLayerList->getLayer(AOI_LAYER, pAoi));
               if (pFusionAoiLayer == NULL)
               {
                  pFusionAoiLayer = static_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi));
               }
               setAoiLayer(pFusionAoiLayer);
            }
            VERIFYNRV(mpAoiLayer != NULL); // Something went really wrong if mpAoiLayer is NULL
         }
         // set tool to be RED
         mpAoiLayer->setColor(ColorType(255, 0, 0));
         // set tool to be border
         mpAoiLayer->setSymbol(BOX);
      }
   }
}

void FusionAlgorithmInputsPage::setAoiLayer(AoiLayer* pLayer)
{
   if (mpAoiLayer != pLayer)
   {
      // detach the old one if it exists
      if (mpAoiLayer != NULL)
      {
         mpAoiLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionAlgorithmInputsPage::aoiLayerDeleted));
      }
      mpAoiLayer = pLayer;
      if (mpAoiLayer != NULL)
      {
         mpAoiLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FusionAlgorithmInputsPage::aoiLayerDeleted));
      }
   }
}

void FusionAlgorithmInputsPage::showEvent(QShowEvent* pEvt)
{
   Service<DesktopServices> pDesktop;
   if (pDesktop.get() != NULL)
   {
      // enable rectangle mode
      pDesktop->setAoiSelectionTool(RECTANGLE_OBJECT, DRAW);
      enableFusion();
   }
   FusionPage::showEvent(pEvt);
}

void FusionAlgorithmInputsPage::enableFusion()
{
   VERIFYNR(mpFlickerOption != NULL && mpRunOverlayOption != NULL);
   if (mpFlickerOption->isChecked() == true)
   {
      mpRunOverlayOption->setEnabled(true);
   }
   else
   {
      mpRunOverlayOption->setChecked(false);
      mpRunOverlayOption->setEnabled(false);
   }

   bool bValid = isValid();
   mpExecuteButton->setEnabled(bValid);
}
