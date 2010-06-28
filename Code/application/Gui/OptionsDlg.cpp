/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QGridLayout>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>

#include "OptionsDlg.h"

#include "ConfigurationSettings.h"
#include "AppVerify.h"
#include "Option.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"

#include <string>
#include <vector>
using namespace std;

OptionsDlg::OptionsDlg(QWidget* pParent) :
   QDialog(pParent)
{
   mpSplitter = new QSplitter();

   mpOptionSelection = new QTreeWidget(mpSplitter);
   mpOptionSelection->setColumnCount(1);
   mpOptionSelection->header()->hide();
   mpOptionSelection->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpOptionSelection->setSelectionMode(QAbstractItemView::SingleSelection);
   mpOptionSelection->setMinimumWidth(160);

   mpOptionStack = new QStackedWidget(mpSplitter);

   mpSplitter->insertWidget(0, mpOptionSelection);
   mpSplitter->insertWidget(1, mpOptionStack);
   mpSplitter->setStretchFactor(0, 0);
   mpSplitter->setStretchFactor(1, 5);

   // Horizontal line
   QFrame* pLine = new QFrame(this);
   pLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);

   // Buttons
   QPushButton* pOkButton = new QPushButton("&OK", this);
   QPushButton* pCancelButton = new QPushButton("&Cancel", this);

   // Layout
   QHBoxLayout* pButtonLayout = new QHBoxLayout();
   pButtonLayout->setMargin(0);
   pButtonLayout->setSpacing(5);
   pButtonLayout->addStretch();
   pButtonLayout->addWidget(pOkButton);
   pButtonLayout->addWidget(pCancelButton);

   QVBoxLayout* pVBox = new QVBoxLayout(this);
   pVBox->setMargin(10);
   pVBox->setSpacing(10);
   pVBox->addWidget(mpSplitter, 10);
   pVBox->addWidget(pLine);
   pVBox->addLayout(pButtonLayout);

   // Initialization
   setWindowTitle("Options");
   setModal(true);

   populateDialogWithOptions();

   // Connections
   VERIFYNR(connect(mpOptionSelection, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(optionSelected(QTreeWidgetItem*))));
   VERIFYNR(connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));

   restoreState();
}

void OptionsDlg::populateDialogWithOptions()
{
   QTreeWidgetItem* pRootItem = new QTreeWidgetItem(0);
   Service<PlugInManagerServices> pPlugMgr;
   vector<PlugInDescriptor*> optionPlugIns = pPlugMgr->getPlugInDescriptors(PlugInManagerServices::OptionType());
   for (vector<PlugInDescriptor*>::iterator optionIter = optionPlugIns.begin();
        optionIter != optionPlugIns.end(); ++optionIter)
   {
      PlugInDescriptor* pDescriptor = *optionIter;
      if (pDescriptor == NULL)
      {
         continue;
      }
      string plugInName = pDescriptor->getName();
      PlugInResource pPlugInRes(plugInName);
      Option* pOptionPlugIn = dynamic_cast<Option*>(pPlugInRes.get());
      if (pOptionPlugIn != NULL)
      {
         QWidget* pWidget = pOptionPlugIn->getWidget();
         string optionName = pOptionPlugIn->getOptionName();
         if ((pWidget != NULL) && (!optionName.empty()))
         {
            mOptionPlugIns.push_back(pOptionPlugIn);               
            pPlugInRes.release();
            mpOptionStack->addWidget(pWidget);
            //In the optionName, '/' is used to separate the tree hierarchy, but
            //'//' should be used to display a '/'
            //So first replace '//' with a highly unlikely sequence, then split on '/'
            //then replace the highly unlikely sequence with '/'.
            QString strOptionName = QString::fromStdString(optionName);
            strOptionName.replace("//", "@@!!@@");
            QStringList optionParts = strOptionName.split("/");
            QTreeWidgetItem* pCurItem = pRootItem;
            for (QStringList::iterator optionPartIter = optionParts.begin();
                 optionPartIter != optionParts.end(); ++optionPartIter)
            {
               QString optionPartName = *optionPartIter;
               optionPartName.replace("@@!!@@", "/");
               //attempt to find item if already present
               QTreeWidgetItem* pFoundItem = NULL;
               for (int curChild = 0; curChild < pCurItem->childCount(); ++curChild)
               {
                  QTreeWidgetItem* pCurChild = pCurItem->child(curChild);
                  if (pCurChild->text(0) == optionPartName)
                  {
                     pFoundItem = pCurChild;
                     break;
                  }
               }
               if (pFoundItem == NULL)
               {
                  QTreeWidgetItem* pNewItem = new QTreeWidgetItem(static_cast<QTreeWidgetItem*>(NULL),
                     QStringList(optionPartName));
                  pCurItem->addChild(pNewItem);
                  pCurItem = pNewItem;
               }
               else
               {
                  pCurItem = pFoundItem;
               }
            }

            mOptionWidgets.insert(pair<QTreeWidgetItem* const, QWidget*>(pCurItem, pWidget));
         }
      }
   }

   fixNodes(pRootItem);

   QList<QTreeWidgetItem*> pRootChildren = pRootItem->takeChildren();
   mpOptionSelection->addTopLevelItems(pRootChildren);
   mpOptionSelection->sortItems(0, Qt::AscendingOrder);
   delete pRootItem;
}

void OptionsDlg::fixNodes(QTreeWidgetItem* pCurItem)
{
   for (int childIndex = 0; childIndex < pCurItem->childCount(); ++childIndex)
   {
      QTreeWidgetItem* pCurChild = pCurItem->child(childIndex);
      if (pCurChild->childCount() != 0) //non-leaf
      {
         pCurChild->setFlags(Qt::ItemIsEnabled);
         map<QTreeWidgetItem*, QWidget*>::iterator iter = mOptionWidgets.find(pCurChild);
         if (iter != mOptionWidgets.end())
         {
            //we have a custom widget associated with a non-leaf node, fix this by
            //forcibly making it a leaf node
            QTreeWidgetItem* pNewNode = new QTreeWidgetItem(static_cast<QTreeWidgetItem*>(NULL),
               QStringList(pCurChild->text(0)));
            pCurChild->addChild(pNewNode);
            mOptionWidgets.insert(pair<QTreeWidgetItem* const, QWidget*>(pNewNode, iter->second));
            mOptionWidgets.erase(iter);
         }
      }
      fixNodes(pCurChild);
   }
}

void OptionsDlg::saveState()
{
   Service<ConfigurationSettings> pSettings;
   pSettings->setSessionSetting("OptionsDlg/geometry/posX", pos().x());
   pSettings->setSessionSetting("OptionsDlg/geometry/posY", pos().y());
   pSettings->setSessionSetting("OptionsDlg/geometry/height", size().height());
   pSettings->setSessionSetting("OptionsDlg/geometry/width", size().width());
   QByteArray splitterConfiguration = mpSplitter->saveState().toBase64();
   string configData;
   QString strConfiguration(splitterConfiguration);
   if (strConfiguration.isEmpty() == false)
   {
      configData = strConfiguration.toStdString();
   }
   pSettings->setSessionSetting("OptionsDlg/geometry/splitterState", configData);
   pSettings->setSessionSetting("OptionsDlg/selectedOption", getCurrentOptionPath());
}

void OptionsDlg::restoreState()
{
   string defaultOptionPage = "Session/General";
   Service<ConfigurationSettings> pSettings;

   DataVariant posVariant = pSettings->getSetting("OptionsDlg/geometry/posX");
   if (!posVariant.isValid() || posVariant.getTypeName() != "int")
   {
      //first time OptionsDlg has been shown during this run of the application
      setCurrentOptionByPath(defaultOptionPage);
      return;
   }
   int posX = dv_cast<int>(posVariant);
   int posY = dv_cast<int>(pSettings->getSetting("OptionsDlg/geometry/posY"));
   int height = dv_cast<int>(pSettings->getSetting("OptionsDlg/geometry/height"));
   int width = dv_cast<int>(pSettings->getSetting("OptionsDlg/geometry/width"));

   resize(width, height);
   move(posX, posY);

   string configData = dv_cast<string>(pSettings->getSetting("OptionsDlg/geometry/splitterState"));
   if (!configData.empty())
   {
      QByteArray splitterConfiguration(configData.c_str(), configData.size());
      mpSplitter->restoreState(QByteArray::fromBase64(splitterConfiguration));
   }

   string selectedOption = dv_cast<string>(pSettings->getSetting("OptionsDlg/selectedOption"));
   if (!setCurrentOptionByPath(selectedOption))
   {
      //couldn't reset back to last option page, ie. plug-in was unloaded, so try the default page
      setCurrentOptionByPath(defaultOptionPage);
   }
}

bool OptionsDlg::setCurrentOptionByPath(std::string path)
{
   QTreeWidgetItem* pFoundItem = NULL;
   if (!path.empty())
   {
      QList<QTreeWidgetItem*> childrenItems;
      for (int count = 0; count < mpOptionSelection->topLevelItemCount(); ++count)
      {
         childrenItems.push_back(mpOptionSelection->topLevelItem(count));
      }
      QStringList optionParts = QString::fromStdString(path).split("/");
      for (QStringList::iterator iter = optionParts.begin();
           iter != optionParts.end(); ++iter)
      {
         QString optionName = *iter;
         optionName.replace("@@!!@@", "/");
         pFoundItem = NULL;
         for (QList<QTreeWidgetItem*>::iterator treeIter = childrenItems.begin();
              treeIter != childrenItems.end(); ++treeIter)
         {
            if ((*treeIter)->text(0) == optionName)
            {
               pFoundItem = *treeIter;
               break;
            }
         }
         if (pFoundItem != NULL)
         {
            childrenItems.clear();
            for (int count = 0; count < pFoundItem->childCount(); ++count)
            {
               childrenItems.push_back(pFoundItem->child(count));
            }
         }
      }
   }
   if (pFoundItem != NULL)
   {
      mpOptionSelection->setCurrentItem(pFoundItem);
      return true;
   }
   else
   {
      return false;
   }
}

std::string OptionsDlg::getCurrentOptionPath()
{
   string currentlySelectedOption;
   QWidget* pTopWidget = mpOptionStack->currentWidget();
   if (pTopWidget != NULL)
   {
      QTreeWidgetItem* pCurrentWidget = NULL;
      for (map<QTreeWidgetItem*, QWidget*>::iterator iter = mOptionWidgets.begin();
           iter != mOptionWidgets.end(); ++iter)
      {
         if (iter->second == pTopWidget)
         {
            pCurrentWidget = iter->first;
         }
      }
      QString curOption;
      while (pCurrentWidget != NULL)
      {
         QString curText = pCurrentWidget->text(0);
         curText.replace("/", "@@!!@@");
         curOption = curText + "/" + curOption;
         pCurrentWidget = pCurrentWidget->parent();
      }
      curOption.chop(1);
      currentlySelectedOption = curOption.toStdString();
   }
   return currentlySelectedOption;
}

OptionsDlg::~OptionsDlg()
{
   saveState();

   //NOTE: The QWidget* returned by Option::getWidget() is not being re-parented
   //This is intentional, this causes a crash if the implementer of the Option interface
   //does not destroy the widget that they created in their destructor.
   vector<Option*>::iterator iter;
   Service<PlugInManagerServices> pPlugMgr;
   for (iter = mOptionPlugIns.begin(); iter != mOptionPlugIns.end(); ++iter)
   {
      pPlugMgr->destroyPlugIn(dynamic_cast<PlugIn*>(*iter));
   }
}

void OptionsDlg::optionSelected(QTreeWidgetItem* pItem)
{
   map<QTreeWidgetItem*, QWidget*>::iterator iter = mOptionWidgets.find(pItem);
   if (iter != mOptionWidgets.end())
   {     
      mpOptionStack->setCurrentWidget(iter->second);
   }
}

void OptionsDlg::accept()
{
   vector<Option*>::iterator iter;
   for (iter = mOptionPlugIns.begin(); iter != mOptionPlugIns.end(); ++iter)
   {
      Option* pOption = *iter;
      pOption->applyChanges();
   }
   QDialog::accept();
}
