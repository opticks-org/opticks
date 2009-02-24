/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "PlugInSelectDlg.h"

#include "PlugInDescriptor.h"
#include "AppVersion.h"
#include "PlugInManagerServices.h"
#include "StringUtilities.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

PlugInSelectDlg::PlugInSelectDlg(QWidget* parent) :
   QDialog(parent)
{
   // Plug-in list view
   QStringList columnNames;
   columnNames.append("Plug-Ins");
   columnNames.append("Type");

   mpPlugInList = new QTreeWidget(this);
   mpPlugInList->setColumnCount(columnNames.count());
   mpPlugInList->setHeaderLabels(columnNames);
   mpPlugInList->setSelectionMode(QAbstractItemView::SingleSelection);
   mpPlugInList->setRootIsDecorated(false);
   mpPlugInList->setSortingEnabled(true);

   QHeaderView* pHeader = mpPlugInList->header();
   if (pHeader != NULL)
   {
      pHeader->setSortIndicatorShown(true);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(0, 125);
   }

   // Buttons
   QPushButton* pOK = new QPushButton("&OK", this);
   QPushButton* pCancel = new QPushButton("&Cancel", this);

   // Layout
   QVBoxLayout* pButtonLayout = new QVBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addWidget(pOK);
   pButtonLayout->addWidget(pCancel);
   pButtonLayout->addStretch(10);

   QHBoxLayout* pLayout = new QHBoxLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(10);
   pLayout->addWidget(mpPlugInList, 10);
   pLayout->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Select Plug-In");
   setModal(true);

   // Connections
   connect(mpPlugInList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
      SLOT(acceptItem(QTreeWidgetItem*)));
   connect(pOK, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pCancel, SIGNAL(clicked()), this, SLOT(reject()));
   mShowWizardOnly = false;
}

PlugInSelectDlg::~PlugInSelectDlg()
{
}

void PlugInSelectDlg::setDisplayedPlugInType(const string& type, const std::string& subtype)
{
   vector<string> types;
   types.push_back(type);

   setDisplayedPlugInTypes(types, subtype);
}

void PlugInSelectDlg::setDisplayedPlugInTypes(const vector<string>& types, const std::string& subtype)
{
   mTypes = types;
   mSubtype = subtype;

   updatePlugInList();
}

void PlugInSelectDlg::setExcludedPlugInTypes(const std::vector<std::string>& types, const std::string& subtype)
{
   mExcludedTypes = types;
   mExcludedSubtype = subtype;

   updatePlugInList();
}

void PlugInSelectDlg::setShowPlugInsForWizardOnly(bool show)
{
   mShowWizardOnly = show;

   updatePlugInList();
}

string PlugInSelectDlg::getSelectedPlugInName() const
{
   string strPlugIn;

   QList<QTreeWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      QTreeWidgetItem* pItem = items.front();
      if (pItem != NULL)
      {
         strPlugIn = pItem->text(0).toStdString();
      }
   }

   return strPlugIn;
}

std::string PlugInSelectDlg::getSelectedPlugInType() const
{
   string type = "";

   QList<QTreeWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      QTreeWidgetItem* pItem = items.front();
      if (pItem != NULL)
      {
         QString strType = pItem->text(1);
         if (strType.isEmpty() == false)
         {
            type = strType.toStdString();
         }
      }
   }

   return type;
}

void PlugInSelectDlg::updatePlugInList()
{
   // Clear the list view
   mpPlugInList->clear();

   Service<PlugInManagerServices> pManager;

   if (mTypes.empty())
   {
      //insert blank type which will cause pManager->getPlugInDescriptors() to return all descriptors
      mTypes.push_back("");
   }

   vector<string>::iterator iter = mTypes.begin();
   while (iter != mTypes.end())
   {
      string plugInType = *iter;
      
      vector<PlugInDescriptor*> plugIns = pManager->getPlugInDescriptors(plugInType);
      
      for (unsigned int i = 0; i < plugIns.size(); i++)
      {
         PlugInDescriptor* pDescriptor = plugIns.at(i);
         if (pDescriptor == NULL)
         {
            continue;
         }
         string curType = pDescriptor->getType();
         string curSubType = pDescriptor->getSubtype();
         vector<string>::iterator foundExclude = std::find(mExcludedTypes.begin(), mExcludedTypes.end(), curType);
         if (foundExclude != mExcludedTypes.end())
         {
            continue;
         }
         if (!mExcludedSubtype.empty() && (curSubType == mExcludedSubtype))
         {
            continue;
         }
         if (!mSubtype.empty() && (curSubType != mSubtype))
         {
            continue;
         }
         if (mShowWizardOnly && pDescriptor->hasWizardSupport() == false)
         {
            continue;
         }
         string name = pDescriptor->getName();
         if (name.empty())
         {
            continue;
         }
         QString strName = QString::fromStdString(name);
         QString strType = QString::fromStdString(pDescriptor->getType());

         QTreeWidgetItem* pItem = new QTreeWidgetItem(mpPlugInList);
         if (pItem != NULL)
         {
            pItem->setText(0, strName);
            pItem->setText(1, strType);
         }
      }

      ++iter;
   }

   // Sort the plug-ins by name
   mpPlugInList->sortItems(0, Qt::AscendingOrder);
}

void PlugInSelectDlg::acceptItem(QTreeWidgetItem* pItem)
{
   if (pItem != NULL)
   {
      accept();
   }
}

void PlugInSelectDlg::accept()
{
   // Get the index of the selected item
   QTreeWidgetItem* pItem = NULL;

   QList<QTreeWidgetItem*> items = mpPlugInList->selectedItems();
   if (items.empty() == false)
   {
      pItem = items.front();
   }

   if (pItem == NULL)
   {
      QMessageBox::warning(this, APP_NAME, "Please select a plug-in.");
      return;
   }

   // Check for a valid plug-in name
   string strPlugIn = getSelectedPlugInName();
   if (strPlugIn.empty() == true)
   {
      QMessageBox::warning(this, APP_NAME, "The selected plug-in is invalid.");
      return;
   }

   // Check for a valid plug-in type
   string type = getSelectedPlugInType();
   if (type.empty())
   {
      QMessageBox::warning(this, APP_NAME, "The selected plug-in is invalid.");
      return;
   }

   QDialog::accept();
}
