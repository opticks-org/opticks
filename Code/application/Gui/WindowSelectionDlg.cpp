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

#include "WindowSelectionDlg.h"
#include "DesktopServicesImp.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "WorkspaceWindow.h"
#include "WorkspaceWindowImp.h"

#include <vector>
using namespace std;

WindowSelectionDlg::WindowSelectionDlg(QWidget* parent) :
QDialog(parent)
{
   // Available windows list
   QLabel* pAvailableLabel = new QLabel("Available Windows:", this);
   mpTileList = new QListWidget(this);
   mpTileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpTileList->setFixedWidth(250);

   // Selected windows
   QLabel* pSelectLabel = new QLabel("Windows to Tile:", this);
   mpSelectedList = new QListWidget(this);
   mpSelectedList->setSelectionMode(QAbstractItemView::ExtendedSelection);
   mpSelectedList->setFixedWidth(250);

   // Selection buttons
   QPushButton* pAdd = new QPushButton(" >> ", this);
   QPushButton* pRemove = new QPushButton(" << ", this);
   QPushButton* pClear = new QPushButton("Clear", this);
   connect(pAdd, SIGNAL(clicked()), this, SLOT(addSelection()));
   connect(pRemove, SIGNAL(clicked()), this, SLOT(removeSelection()));
   connect(pClear, SIGNAL(clicked()), this, SLOT(clearSelections()));

   QVBoxLayout* pSelectionLayout = new QVBoxLayout();
   pSelectionLayout->setMargin(10);
   pSelectionLayout->setSpacing(5);
   pSelectionLayout->addWidget(pAdd);
   pSelectionLayout->addWidget(pRemove);
   pSelectionLayout->addWidget(pClear);

   // Tiling Option
   QGroupBox* pOptionGroup = new QGroupBox("Tiling Option", this);
   mpTileRadio = new QRadioButton("Tile", pOptionGroup);
   mpTileRadio->setFixedWidth(40);
   connect(mpTileRadio, SIGNAL(toggled(bool)), this, SLOT(tileChanged(bool)));
   mpMaxFirst = new QCheckBox("Maximize First", pOptionGroup);
   mpTileHRadio = new QRadioButton("Tile Horizontally", pOptionGroup);
   mpTileVRadio = new QRadioButton("Tile Vertically", pOptionGroup);

   QFrame* pVLine = new QFrame(this);
   pVLine->setFrameStyle(QFrame::VLine | QFrame::Sunken);

   QHBoxLayout* pOptionLayout = new QHBoxLayout(pOptionGroup);
   pOptionLayout->setMargin(10);
   pOptionLayout->setSpacing(5);
   pOptionLayout->addWidget(mpTileRadio);
   pOptionLayout->setStretchFactor(mpTileRadio, 1);
   pOptionLayout->addWidget(mpMaxFirst);
   pOptionLayout->addWidget(pVLine);
   pOptionLayout->addWidget(mpTileHRadio);
   pOptionLayout->addWidget(mpTileVRadio);

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
   pGrid->setSpacing(5);
   pGrid->addWidget(pAvailableLabel, 0, 0);
   pGrid->addWidget(mpTileList, 1, 0);
   pGrid->addLayout(pSelectionLayout, 1, 1);
   pGrid->addWidget(pSelectLabel, 0, 2);
   pGrid->addWidget(mpSelectedList, 1, 2);
   pGrid->addWidget(pOptionGroup, 2, 0, 1, 3);
   pGrid->addWidget(pLine, 3, 0, 1, 3);
   pGrid->addLayout(pHLayout, 4, 0, 1, 3);
   pGrid->setRowStretch(1, 10);

   // Initialization
   setWindowTitle("Select Windows to Tile");
   setModal(true);
   mpTileRadio->setChecked(true);
   mpMaxFirst->setChecked(true);

   ModelServicesImp* pModel = NULL;
   pModel = ModelServicesImp::instance();
   if (pModel != NULL)
   {
      DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
      if (pDesktop != NULL)
      {
         std::vector<Window*> windows;
         pDesktop->getWindows(windows);
         std::vector<Window*>::const_iterator it;
         WindowType winType;
         for (it = windows.begin(); it != windows.end(); ++it)
         {
            Window* pWin = *it;
            if (pWin != NULL)
            {
               winType = pWin->getWindowType();
               if (winType == SPATIAL_DATA_WINDOW || winType == PRODUCT_WINDOW)
               {
                  string name = pWin->getName();
                  if (name.empty() == false)
                  {
                     QString label(winType == SPATIAL_DATA_WINDOW ? "SDW>" : "PW>");
                     label += name.c_str();
                     QListWidgetItem* pItem = new QListWidgetItem(label, mpTileList);
                     pItem->setData(Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(pWin)));
                  }
               }
            }
         }
      }
   }

}

WindowSelectionDlg::~WindowSelectionDlg()
{
}

std::vector<WorkspaceWindow*> WindowSelectionDlg::getSelectedWindows() const
{
   std::vector<WorkspaceWindow*> selectedWindows;
   int numWins = mpSelectedList->count();
   for (int i = 0; i < numWins; ++i)
   {
      QListWidgetItem* pItem = mpSelectedList->item(i);
      if (pItem != NULL)
      {
         QVariant var = pItem->data(Qt::UserRole);
         void* pVoid = var.value<void*>();
         if (pVoid != NULL)
         {
            WorkspaceWindow* pWin = reinterpret_cast<WorkspaceWindow*>(pVoid);
            if (pWin != NULL)
            {
               selectedWindows.push_back(pWin);
            }
         }
      }
   }

   return selectedWindows;
}

QSize WindowSelectionDlg::sizeHint() const
{
   return QSize(400, 250);
}

void WindowSelectionDlg::accept()
{
   int iCount = mpSelectedList->count();
   if (iCount < 1)
   {
      QMessageBox::warning(this, "Tile Selected Windows", "Please select at least one window to tile!");
      return;
   }

   QDialog::accept();
}

TilingType WindowSelectionDlg::getTilingType() const
{
   if (mpTileHRadio->isChecked())
   {
      return TILE_HORIZONTAL;
   }
   if (mpTileVRadio->isChecked())
   {
      return TILE_VERTICAL;
   }

   return TILE_GRID;
}

void WindowSelectionDlg::clearSelections()
{
   mpSelectedList->clear();
}

void WindowSelectionDlg::addSelection()
{
   QList<QListWidgetItem*> selectedItems = mpTileList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      QListWidgetItem* pItem = selectedItems.at(i);
      QList<QListWidgetItem*> matches = mpSelectedList->findItems(pItem->text(),
                                                          Qt::MatchFixedString);
      if (matches.count() == 0)
      {
         QListWidgetItem* pNewItem = new QListWidgetItem(pItem->text(), mpSelectedList);
         if (pNewItem != NULL)
         {
            pNewItem->setData(Qt::UserRole, pItem->data(Qt::UserRole));
         }
      }
   }
}

void WindowSelectionDlg::removeSelection()
{
   QList<QListWidgetItem*> selectedItems = mpSelectedList->selectedItems();
   for (int i = 0; i < selectedItems.count(); ++i)
   {
      QListWidgetItem* pItem = selectedItems.at(i);
      int row = mpSelectedList->row(pItem);
      mpSelectedList->takeItem(row);
      delete pItem;
   }
}

void WindowSelectionDlg::tileChanged(bool enable)
{
   mpMaxFirst->setEnabled(enable);
}

bool WindowSelectionDlg::maximizeFirstColumn()
{
   return mpMaxFirst->isChecked();
}
