/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AddFeatureDlg.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <string>
using namespace std;

AddFeatureDlg::AddFeatureDlg(QWidget* parent) :
   QDialog(parent)
{
   // Element list
   QLabel* pElementLabel = new QLabel("Data Elements:", this);

   QStringList columnNames;
   columnNames.append("Name");
   columnNames.append("Type");

   mpElementTree = new QTreeWidget(this);
   mpElementTree->setColumnCount(columnNames.count());
   mpElementTree->setHeaderLabels(columnNames);
   mpElementTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpElementTree->setRootIsDecorated(false);
   mpElementTree->setSortingEnabled(true);

   QHeaderView* pHeader = mpElementTree->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
   }

   // Horizontal line
   QFrame* pHLine = new QFrame(this);
   pHLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Dialog buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pLayout = new QHBoxLayout();
   pLayout->setMargin(0);
   pLayout->setSpacing(5);
   pLayout->addStretch(10);
   pLayout->addWidget(pOkButton);
   pLayout->addWidget(pCancelButton);

   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pElementLabel, 0, 0);
   pGrid->addWidget(mpElementTree, 1, 0);
   pGrid->addWidget(pHLine, 2, 0);
   pGrid->addLayout(pLayout, 3, 0);
   pGrid->setRowStretch(1, 10);
   pGrid->setColumnStretch(0, 10);

   // Initialization
   setWindowTitle("Add Feature");
   setModal(true);
   resize(400, 225);

   // Get the current data set filename from desktop services
   RasterElement* pRasterElement = NULL;

   Service<DesktopServices> pDesktop;

   SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   if(pWindow != NULL)
   {
      SpatialDataView* pView = pWindow->getSpatialDataView();
      if(pView != NULL)
      {
         LayerList* pLayerList = pView->getLayerList();
         if (pLayerList != NULL)
         {
            pRasterElement = pLayerList->getPrimaryRasterElement();
         }
      }
   }

   // Populate the list with AOIs
   Service<ModelServices> pModel;

   vector<DataElement*> elements = pModel->getElements(pRasterElement, "AoiElement");
   for (unsigned int i = 0; i < elements.size(); i++)
   {
      DataElement* pElement = elements[i];
      if (pElement != NULL)
      {
         QString strName;

         string elementName = pElement->getName();
         if (elementName.empty() == false)
         {
            strName = QString::fromStdString(elementName);
         }

         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpElementTree);
         if (pItem != NULL)
         {
            pItem->setText(0, strName);
            pItem->setText(1, "AOI");
            mElements.insert(pItem, pElement);
         }
      }
   }

   // Connections
   connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

AddFeatureDlg::~AddFeatureDlg()
{
}

vector<DataElement*> AddFeatureDlg::getDataElements() const
{
   vector<DataElement*> elements;

   for (QMap<QTreeWidgetItem*, DataElement*>::const_iterator iter = mElements.begin();
                                                             iter != mElements.end();
                                                             ++iter)
   {
      QTreeWidgetItem* pItem = iter.key();
      if(pItem != NULL)
      {
         if(mpElementTree->isItemSelected(pItem))
         {
            DataElement* pElement = iter.value();
            if(pElement != NULL)
            {
               elements.push_back(pElement);
            }
         }
      }
   }

   return elements;
}

void AddFeatureDlg::accept()
{
   if (getDataElements().empty() == true)
   {
      QMessageBox::critical(this, windowTitle(), "Please select at least one element for a feature!");
      return;
   }

   QDialog::accept();
}
