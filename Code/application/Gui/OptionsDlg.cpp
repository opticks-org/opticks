/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ConfigurationSettings.h"
#include "Option.h"
#include "OptionsDlg.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>

#include <string>
using namespace std;

OptionsDlg::OptionsDlg(QWidget* pParent) :
   QDialog(pParent)
{
   mpSplitter = new QSplitter();

   mpOptionSelection = new QTreeWidget(mpSplitter);
   mpOptionSelection->setColumnCount(1);
   mpOptionSelection->setHeaderHidden(true);
   mpOptionSelection->setSelectionBehavior(QAbstractItemView::SelectItems);
   mpOptionSelection->setSelectionMode(QAbstractItemView::SingleSelection);
   mpOptionSelection->setMinimumWidth(200);

   mpOptionStack = new QStackedWidget(mpSplitter);

   mpNoOptionsWidget = new QLabel("No options are available for this item.  Please select one of its subitems.",
      mpOptionStack);
   mpNoOptionsWidget->setAlignment(Qt::AlignCenter);
   mpOptionStack->addWidget(mpNoOptionsWidget);

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
   resize(minimumSizeHint());

   // Connections
   VERIFYNR(connect(mpOptionSelection, SIGNAL(itemSelectionChanged()), this, SLOT(updateOptionWidget())));
   VERIFYNR(connect(pOkButton, SIGNAL(clicked()), this, SLOT(accept())));
   VERIFYNR(connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject())));

   restoreState();
}

void OptionsDlg::activatePage(const QString& pageName)
{
   if (pageName.isEmpty() == true)
   {
      return;
   }

   QTreeWidgetItemIterator iter(mpOptionSelection);
   while (*iter != NULL)
   {
      QTreeWidgetItem* pItem = *iter;
      if ((pItem != NULL) && (pItem->text(0) == pageName))
      {
         mpOptionSelection->setCurrentItem(pItem);
         break;
      }

      ++iter;
   }
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
                  pNewItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                  mOptionWidgets[pNewItem] = mpNoOptionsWidget;

                  pCurItem->addChild(pNewItem);
                  pCurItem = pNewItem;
               }
               else
               {
                  pCurItem = pFoundItem;
               }
            }

            pCurItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mOptionWidgets[pCurItem] = pWidget;
         }
      }
   }

   QList<QTreeWidgetItem*> pRootChildren = pRootItem->takeChildren();
   mpOptionSelection->addTopLevelItems(pRootChildren);
   mpOptionSelection->sortItems(0, Qt::AscendingOrder);
   delete pRootItem;
}

void OptionsDlg::saveState()
{
   Service<ConfigurationSettings> pSettings;
   pSettings->setTemporarySetting("OptionsDlg/geometry/posX", pos().x());
   pSettings->setTemporarySetting("OptionsDlg/geometry/posY", pos().y());
   pSettings->setTemporarySetting("OptionsDlg/geometry/height", size().height());
   pSettings->setTemporarySetting("OptionsDlg/geometry/width", size().width());
   QByteArray splitterConfiguration = mpSplitter->saveState().toBase64();
   string configData;
   QString strConfiguration(splitterConfiguration);
   if (strConfiguration.isEmpty() == false)
   {
      configData = strConfiguration.toStdString();
   }
   pSettings->setTemporarySetting("OptionsDlg/geometry/splitterState", configData);
   pSettings->setTemporarySetting("OptionsDlg/selectedOption", getCurrentOptionPath());
}

void OptionsDlg::restoreState()
{
   string defaultOptionPage = "General";
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
      pFoundItem->setExpanded(true);
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
   Service<PlugInManagerServices> pPlugMgr;
   for (vector<Option*>::iterator iter = mOptionPlugIns.begin(); iter != mOptionPlugIns.end(); ++iter)
   {
      pPlugMgr->destroyPlugIn(dynamic_cast<PlugIn*>(*iter));
   }
}

void OptionsDlg::updateOptionWidget()
{
   QList<QTreeWidgetItem*> selectedItems = mpOptionSelection->selectedItems();
   VERIFYNRV(selectedItems.count() == 1);

   QTreeWidgetItem* pItem = selectedItems.front();
   VERIFYNRV(pItem != NULL);

   map<QTreeWidgetItem*, QWidget*>::iterator iter = mOptionWidgets.find(pItem);
   if (iter != mOptionWidgets.end())
   {
      mpOptionStack->setCurrentWidget(iter->second);
   }
}

void OptionsDlg::accept()
{
   for (vector<Option*>::iterator iter = mOptionPlugIns.begin(); iter != mOptionPlugIns.end(); ++iter)
   {
      Option* pOption = *iter;
      if (pOption != NULL)
      {
         pOption->applyChanges();
      }
   }

   QDialog::accept();
}
