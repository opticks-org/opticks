/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>

#include "AppVersion.h"
#include "LinkDlg.h"
#include "DataElement.h"
#include "AppVerify.h"
#include "DesktopServices.h"
#include "LinkDatasetPage.h"
#include "LinkOptionsPage.h"
#include "Layer.h"
#include "LayerImp.h"
#include "LayerList.h"
#include "LayerListImp.h"
#include "ModelServicesImp.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "SpatialDataViewImp.h"
#include "SubjectAdapter.h"
#include "WorkspaceWindow.h"

#include <string>
using namespace std;

LinkDlg::LinkDlg(QWidget* parent) :
   QDialog(parent)
{
   // Step description
   mpDescriptionLabel = new QLabel(this);

   QFont descriptionFont = mpDescriptionLabel->font();
   descriptionFont.setBold(true);
   mpDescriptionLabel->setFont(descriptionFont);

   // Horizontal line
   QFrame* pDescriptionLine = new QFrame(this);
   pDescriptionLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Wizard pages
   mpStack = new QStackedWidget(this);

   mpSourcePage = new LinkDatasetPage(mpStack);
   mpOptionsPage = new LinkOptionsPage(mpStack);
   mpDestinationPage = new LinkDatasetPage(mpStack);

   mpStack->addWidget(mpSourcePage);
   mpStack->addWidget(mpOptionsPage);
   mpStack->addWidget(mpDestinationPage);

   // Horizontal line
   QFrame* pButtonLine = new QFrame(this);
   pButtonLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   mpBackButton = new QPushButton("< &Back", this);
   mpNextButton = new QPushButton("&Next >", this);
   mpFinishButton = new QPushButton("&Finish", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(mpBackButton);
   pButtonLayout->addWidget(mpNextButton);
   pButtonLayout->addWidget(mpFinishButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   pLayout->addWidget(mpDescriptionLabel);
   pLayout->addWidget(pDescriptionLine);
   pLayout->addWidget(mpStack, 10);
   pLayout->addWidget(pButtonLine);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Link/Unlink");
   setModal(true);
   resize(550, 350);
   showPage(mpSourcePage);

   // Connections
   connect(mpBackButton, SIGNAL(clicked()), this, SLOT(back()));
   connect(mpNextButton, SIGNAL(clicked()), this, SLOT(next()));
   connect(mpFinishButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

LinkDlg::~LinkDlg()
{
}

Layer* LinkDlg::duplicateLayer(Layer* pLayer, SpatialDataView* pView)
{
   if ((pLayer == NULL) || (pView == NULL))
   {
      return NULL;
   }

   QString strName = QString::fromStdString(pLayer->getName());
   QString strDataset;

   RasterElement* pRasterElement = NULL;

   DataElement* pElement = pLayer->getDataElement();
   if (pElement != NULL)
   {
      string elementName = pElement->getName();
      const string& elementType = pElement->getObjectType();

      if ((elementName.empty() == false) && (elementType.empty() == false))
      {
         LayerListImp* pLayerList = dynamic_cast<LayerListImp*>(pView->getLayerList());
         if (pLayerList != NULL)
         {
            pRasterElement = pLayerList->getPrimaryRasterElement();
         }

         ModelServicesImp* pModel = ModelServicesImp::instance();
         if (pModel != NULL)
         {
            QString strElementName = QString::fromStdString(elementName);

            // Check if the element already exists in the destination data set
            DataElement* pExistElement = pModel->getElement(elementName, elementType, pRasterElement);
            if (pExistElement != NULL)
            {
               // Force the user to rename the new element
               QMessageBox::warning(dynamic_cast<SpatialDataViewImp*>(pView), APP_NAME, "The '" + strElementName +
                  "' element name already exists.  You must select a new name.");

               if (pLayerList != NULL)
               {
                  LayerType eType = pLayer->getLayerType();
                  strElementName = pLayerList->getUniqueLayerName(strElementName, eType);
               }
            }

            if (strElementName.isEmpty() == true)
            {
               return NULL;
            }

            // Ensure the layer name matched the element name
            strName = strElementName;
         }
      }
   }

   Layer* pNewLayer = pLayer->copy(string(), true, pRasterElement);
   if (pNewLayer != NULL)
   {
      LayerImp* pNewLayerImp = dynamic_cast<LayerImp*>(pNewLayer);
      if (pNewLayerImp != NULL)
      {
         pNewLayerImp->setName(strName.toStdString());
      }

      bool bSuccess = pView->addLayer(pNewLayer);
      if (bSuccess == false)
      {
         delete pNewLayerImp;
         return NULL;
      }
   }

   return pNewLayer;
}

void LinkDlg::showPage(QWidget* pPage)
{
   if (pPage == NULL)
   {
      return;
   }

   if (pPage == mpSourcePage)
   {
      mpDescriptionLabel->setText("Select the Source Data Set (Step 1 of 3)");
   }
   else if (pPage == mpOptionsPage)
   {
      mpDescriptionLabel->setText("Select the Link Options (Step 2 of 3)");
   }
   else if (pPage == mpDestinationPage)
   {
      mpDescriptionLabel->setText("Select the Destination Data Set (Step 3 of 3)");
   }

   mpBackButton->setEnabled(pPage != mpSourcePage);
   mpNextButton->setVisible(pPage != mpDestinationPage);
   mpFinishButton->setVisible(pPage == mpDestinationPage);
   mpStack->setCurrentWidget(pPage);
}

void LinkDlg::back()
{
   QWidget* pPageToShow = NULL;

   QWidget* pPage = mpStack->currentWidget();
   if (pPage == mpOptionsPage)
   {
      pPageToShow = mpSourcePage;
   }
   else if (pPage == mpDestinationPage)
   {
      pPageToShow = mpOptionsPage;
   }

   showPage(pPageToShow);
}

void LinkDlg::next()
{
   QWidget* pPageToShow = NULL;

   QWidget* pPage = mpStack->currentWidget();
   if (pPage == mpSourcePage)
   {
      QString strDataset = mpSourcePage->getSelectedDatasetName();
      mpOptionsPage->setLinkObjects(strDataset);

      pPageToShow = mpOptionsPage;
   }
   else if (pPage == mpOptionsPage)
   {
      // Check for no selected links
      vector<View*> viewLinks = mpOptionsPage->getViewLinks();
      vector<Layer*> layerLinks = mpOptionsPage->getLayerLinks();

      if ((viewLinks.empty() == true) && (layerLinks.empty() == true))
      {
         QMessageBox::critical(this, windowTitle(), "No links are selected!");
         return;
      }

      pPageToShow = mpDestinationPage;
   }

   showPage(pPageToShow);
}

void LinkDlg::accept()
{
   QString strSourceDataset = mpSourcePage->getSelectedDatasetName();
   QString strDestinationDataset = mpDestinationPage->getSelectedDatasetName();

   if ((strSourceDataset == strDestinationDataset) && (strDestinationDataset.isEmpty() == false))
   {
      QMessageBox::critical(this, windowTitle(), "The source data set is the same as the "
         "destination data set!");
      return;
   }

   SpatialDataView* pDestinationView = NULL;
   vector<Window*> windows;

   Service<DesktopServices> pDesktop;
   pDesktop->getWindows(windows);

   vector<Window*>::iterator iter;
   for (iter = windows.begin(); iter != windows.end(); ++iter)
   {
      WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pWindow->getActiveView());
         if (pView != NULL)
         {
            QString strViewName = QString::fromStdString(pView->getName());
            if (strViewName == strDestinationDataset)
            {
               pDestinationView = pView;
               break;
            }
         }
      }
   }

   if (pDestinationView != NULL)
   {
      // Views
      vector<View*> viewLinks = mpOptionsPage->getViewLinks();

      unsigned int i = 0;
      for (i = 0; i < viewLinks.size(); i++)
      {
         SpatialDataView* pSourceView = dynamic_cast<SpatialDataView*>(viewLinks[i]);
         if (pSourceView != NULL)
         {
            LinkType type = mpOptionsPage->getLinkType();
            if (type == GEOCOORD_LINK)
            {
               SpatialDataViewImp* pSrcViewImp = dynamic_cast<SpatialDataViewImp*>(pSourceView);
               SpatialDataViewImp* pDestViewImp = dynamic_cast<SpatialDataViewImp*>(pDestinationView);
               if (pSrcViewImp == NULL || pDestViewImp == NULL)
               {
                  QDialog::reject();
                  VERIFYNRV(false);
               }
               LayerList* pSrcLayerList = pSrcViewImp->getLayerList();
               LayerList* pDestLayerList = pDestViewImp->getLayerList();
               if (pSrcLayerList == NULL || pDestLayerList == NULL)
               {
                  QDialog::reject();
                  VERIFYNRV(false);
               }

               RasterElement* pSrcRaster = pSrcLayerList->getPrimaryRasterElement();
               RasterElement* pDestRaster = pDestLayerList->getPrimaryRasterElement();
               if (pSrcRaster == NULL || !pSrcRaster->isGeoreferenced())
               {
                  QString strSourceName = QString::fromStdString(pSrcViewImp->getName());
                  QMessageBox::critical(this, windowTitle(), "The " + strSourceName +
                     " data set must have geocoords in order to link based on "
                     "latitude and longitude.");
                  QDialog::reject();
                  return;
               }
               if (pDestRaster == NULL || !pDestRaster->isGeoreferenced())
               {
                  QString strDestinationName = QString::fromStdString(pDestViewImp->getName());
                  QMessageBox::critical(this, windowTitle(), "The " + strDestinationName +
                     " data set must have geocoords in order to link based on "
                     "latitude and longitude.");
                  QDialog::reject();
                  return;
               }
            }
            pSourceView->linkView(pDestinationView, type);
            if (mpOptionsPage->isTwoWayLink() == true)
            {
               pDestinationView->linkView(pSourceView, type);
            }
         }
      }

      // Layers
      vector<Layer*> layerLinks = mpOptionsPage->getLayerLinks();
      for (i = 0; i < layerLinks.size(); i++)
      {
         Layer* pLayer = NULL;
         pLayer = layerLinks.at(i);
         if (pLayer != NULL)
         {
            bool bDuplicate = false;
            bDuplicate = mpOptionsPage->areLayersDuplicated();
            if (bDuplicate == true)
            {
               Layer* pNewLayer = NULL;
               pNewLayer = duplicateLayer(pLayer, pDestinationView);
               if (pNewLayer == NULL)
               {
                  QMessageBox::critical(this, windowTitle(), "The " +
                     QString::fromStdString(pLayer->getName()) + " layer cannot be duplicated!");
               }
            }
         }
      }
   }

   QDialog::accept();
}
