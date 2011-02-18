/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "AoiMergeDlg.h"
#include "DesktopServicesImp.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <vector>
using namespace std;

AoiMergeDlg::AoiMergeDlg(QWidget* parent) :
   QDialog(parent)
{
   // Merge list
   QLabel* pMergeLabel = new QLabel("Please select two or more\nAOIs to merge:", this);
   mpMergeList = new QListWidget(this);
   mpMergeList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpMergeList->setFixedWidth(150);

   // Method
   QGroupBox* pMethodGroup = new QGroupBox("Merge Method", this);
   mpCombineRadio = new QRadioButton("Combination (union)", pMethodGroup);
   QRadioButton* pCommonRadio = new QRadioButton("Common pixels (intersection)", pMethodGroup);

   QVBoxLayout* pMethodLayout = new QVBoxLayout(pMethodGroup);
   pMethodLayout->setMargin(10);
   pMethodLayout->setSpacing(5);
   pMethodLayout->addWidget(mpCombineRadio);
   pMethodLayout->addWidget(pCommonRadio);

   // Output
   QLabel* pOutputLabel = new QLabel("Output AOI Name:", this);
   mpOutputCombo = new QComboBox(this);
   mpOutputCombo->setEditable(true);
   mpOutputCombo->setAutoCompletion(false);
   mpOutputCombo->setInsertPolicy(QComboBox::NoInsert);

   QHBoxLayout* pOutputLayout = new QHBoxLayout();
   pOutputLayout->setMargin(0);
   pOutputLayout->setSpacing(5);
   pOutputLayout->addWidget(pOutputLabel);
   pOutputLayout->addWidget(mpOutputCombo, 10);

   QVBoxLayout* pVLayout = new QVBoxLayout();
   pVLayout->setMargin(0);
   pVLayout->setSpacing(5);
   pVLayout->addWidget(pMethodGroup);
   pVLayout->addStretch(10);
   pVLayout->addLayout(pOutputLayout);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // OK and Cancel buttons
   QPushButton* pOk = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);
   connect(pOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));

   QHBoxLayout* pHLayout = new QHBoxLayout();
   pHLayout->setMargin(0);
   pHLayout->setSpacing(5);
   pHLayout->addStretch(10);
   pHLayout->addWidget(pOk);
   pHLayout->addWidget(pCancel);

   // Layout
   QGridLayout* pGrid = new QGridLayout(this);
   pGrid->setMargin(10);
   pGrid->setSpacing(10);
   pGrid->addWidget(pMergeLabel, 0, 0);
   pGrid->addWidget(mpMergeList, 1, 0);
   pGrid->addLayout(pVLayout, 0, 1, 2, 1);
   pGrid->addWidget(pLine, 2, 0, 1, 2);
   pGrid->addLayout(pHLayout, 3, 0, 1, 2);
   pGrid->setRowStretch(1, 10);

   // Initialization
   setWindowTitle("AOI Merge");
   setModal(true);

   ModelServicesImp* pModel = NULL;
   pModel = ModelServicesImp::instance();
   if (pModel != NULL)
   {
      DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
      if (pDesktop != NULL)
      {
         SpatialDataView* pView = NULL;

         SpatialDataWindow* pSpatialDataWindow =
            dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
         if (pSpatialDataWindow != NULL)
         {
            pView = pSpatialDataWindow->getSpatialDataView();
         }
         else
         {
            ProductWindow* pProductWindow = dynamic_cast<ProductWindow*>(pDesktop->getCurrentWorkspaceWindow());
            if (pProductWindow != NULL)
            {
               ProductView* pProductView = pProductWindow->getProductView();
               if (pProductView != NULL)
               {
                  pView = dynamic_cast<SpatialDataView*> (pProductView->getActiveEditView());
               }
            }
         }

         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();

               QStringList strlAoiNames;

               vector<string> aoiNames = pModel->getElementNames(pRasterElement, "AoiElement");
               for (unsigned int i = 0; i < aoiNames.size(); ++i)
               {
                  string aoiName = aoiNames[i];
                  if (aoiName.empty() == false)
                  {
                     strlAoiNames.append(QString::fromStdString(aoiName));
                  }
               }

               mpMergeList->addItems(strlAoiNames);
               mpOutputCombo->addItems(strlAoiNames);
            }
         }
      }
   }

   mpCombineRadio->setChecked(true);
   mpOutputCombo->clearEditText();
}

AoiMergeDlg::~AoiMergeDlg()
{
}

QStringList AoiMergeDlg::getMergeAoiNames()
{
   QStringList strlAoiNames;

   QList<QListWidgetItem*> selectedItems = mpMergeList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      QListWidgetItem* pItem = selectedItems[i];
      if (pItem != NULL)
      {
         QString strName = pItem->text();
         if (strName.isEmpty() == false)
         {
            strlAoiNames.append(strName);
         }
      }
   }

   return strlAoiNames;
}

QString AoiMergeDlg::getOutputAoiName()
{
   QString strName = mpOutputCombo->currentText();
   return strName;
}

bool AoiMergeDlg::combinePixels()
{
   bool bCombine = mpCombineRadio->isChecked();
   return bCombine;
}

QSize AoiMergeDlg::sizeHint() const
{
   return QSize(400, 250);
}

void AoiMergeDlg::accept()
{
   QStringList strlAoiNames = getMergeAoiNames();
   
   int iCount = 0;
   iCount = strlAoiNames.count();
   if (iCount < 2)
   {
      QMessageBox::warning(this, "Merge AOI", "Please select at least two AOIs to merge!");
      return;
   }

   QString strOutputName = getOutputAoiName();
   if (strOutputName.isEmpty() == true)
   {
      QMessageBox::warning(this, "Merge AOI", "Please enter a name for the output AOI!");
      return;
   }

   if (!combinePixels())
   {
      if (QMessageBox::warning(this, "Merge AOI", 
         "Using common points (intersection) will cause the output AOI to be a bitmask.  "
         "This may cause loss of information.", QMessageBox::Ok, QMessageBox::Cancel) 
         == QMessageBox::Cancel)
      {
         return;
      }
   }

   QDialog::accept();
}
