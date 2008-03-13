/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "OptionsShortcuts.h"

#include "ApplicationWindow.h"
#include "CustomTreeWidget.h"
#include "DynamicObject.h"
#include "LabeledSection.h"
#include "ObjectResource.h"
#include "PlugInManagerServicesImp.h"

#include <QtGui/QApplication>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

#include <string>

using namespace std;

OptionsShortcuts::OptionsShortcuts() :
   QWidget(NULL)
{
   // Shortcuts
   QStringList columnNames;
   columnNames.append("Description");
   columnNames.append("Shortcut");

   mpShortcutTree = new CustomTreeWidget(this);
   mpShortcutTree->setColumnCount(2);
   mpShortcutTree->setHeaderLabels(columnNames);
   mpShortcutTree->setRootIsDecorated(true);
   mpShortcutTree->setSelectionMode(QAbstractItemView::SingleSelection);
   mpShortcutTree->setSortingEnabled(true);
   mpShortcutTree->setGridlinesShown(Qt::Horizontal | Qt::Vertical, true);
   QHeaderView* pHeader = mpShortcutTree->header();
   if (pHeader != NULL)
   {
      pHeader->setStretchLastSection(false);
      pHeader->resizeSection(0, 150);
      pHeader->resizeSection(1, 100);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->setSortIndicatorShown(true);
   }
   LabeledSection* pShortcutSection = new LabeledSection(mpShortcutTree, "Current Shortcuts", this);
   
   // Dialog layout
   QVBoxLayout* pLayout = new QVBoxLayout(this);
   pLayout->setMargin(0);
   pLayout->setSpacing(10);
   pLayout->addWidget(pShortcutSection);

   // Initialize From Settings
   shortcutsIntoGui(ApplicationWindow::getSettingShortcuts(), NULL);
   mpShortcutTree->sortItems(0, Qt::AscendingOrder);
}
   
void OptionsShortcuts::applyChanges()
{  
   mpShortcutTree->closeActiveCellWidget(true);

   FactoryResource<DynamicObject> pShortcutRes;

   for (int count = 0; count < mpShortcutTree->topLevelItemCount(); count++)
   {
      QTreeWidgetItem* pItem = mpShortcutTree->topLevelItem(count);
      if (pItem->text(0).toStdString() == "Plug-Ins")
      {
         for (int plugInCount = 0; plugInCount < pItem->childCount(); plugInCount++)
         {
            shortcutsFromGui(pItem->child(plugInCount), pShortcutRes.get());
         }
      }
      else
      {
         shortcutsFromGui(pItem, pShortcutRes.get());
      }
   }
   ApplicationWindow::setSettingShortcuts(pShortcutRes.get());

   //Now update all found QAction's to have the new shortcut value
   const DynamicObject* pShortcuts = ApplicationWindow::getSettingShortcuts();
   if (pShortcuts != NULL)
   {
      // Update the actions with the current shortcut values
      QWidgetList topWidgets = QApplication::topLevelWidgets();
      QWidgetList::iterator widgetIter;
      for (widgetIter = topWidgets.begin(); widgetIter != topWidgets.end(); ++widgetIter)
      {
         QList<QAction*> actions = (*widgetIter)->findChildren<QAction*>();

         for (int i = 0; i < actions.count(); ++i)
         {
            QAction* pAction = actions[i];
            if (pAction != NULL)
            {
               QMap<QString, QVariant> actionData = pAction->data().toMap();

               QMap<QString, QVariant>::const_iterator iter = actionData.find("ShortcutContext");
               if (iter != actionData.end())
               {
                  string strContext = iter.value().toString().toStdString();
                  string strName = pAction->toolTip().toStdString();

                  const DataVariant& dv = pShortcuts->getAttributeByPath(strContext + "/" + strName);
                  if (dv.isValid() && dv.getTypeName() == "string")
                  {
                     QString keySequence = QString::fromStdString(dv_cast<string>(dv));
                     pAction->setShortcut(QKeySequence(keySequence));
                  }
               }
            }
         }
      }
   }
}

OptionsShortcuts::~OptionsShortcuts()
{
}

void OptionsShortcuts::shortcutsIntoGui(const DynamicObject* pCurObject, QTreeWidgetItem* pParent)
{
   Service<PlugInManagerServices> pManager;
   vector<string> attributes;
   vector<string>::iterator attrIter;
   pCurObject->getAttributeNames(attributes);

   QTreeWidgetItem* pPlugInItem = NULL;
   for (attrIter = attributes.begin(); attrIter != attributes.end(); ++attrIter)
   {
      string name = *attrIter;
      const DataVariant& dv = pCurObject->getAttribute(*attrIter);
      if (dv.isValid())
      {
         QTreeWidgetItem* pItem = NULL;
         if (pParent == NULL)
         {
            //see if this item is a plug-in, if so put under a "PlugIns" item in the tree widget
            if (pManager->getPlugInDescriptor(name) != NULL)
            {
               if (pPlugInItem == NULL)
               {
                  pPlugInItem = new QTreeWidgetItem(mpShortcutTree);
                  pPlugInItem->setText(0, "Plug-Ins");
                  pPlugInItem->setBackgroundColor(1, QColor(235, 235, 235));
               }
               pItem = new QTreeWidgetItem(pPlugInItem);
            }
            else
            {
               pItem = new QTreeWidgetItem(mpShortcutTree);
            }
         }
         else
         {
            pItem = new QTreeWidgetItem(pParent);
         }

         if (dv.getTypeName() == "DynamicObject")
         {
            const DynamicObject* pObject = dv_cast<DynamicObject>(&dv);
            pItem->setText(0, QString::fromStdString(name));
            pItem->setBackgroundColor(1, QColor(235, 235, 235));
            shortcutsIntoGui(pObject, pItem);
         }
         else if (dv.getTypeName() == "string")
         {
            QString shortcut = QString::fromStdString(dv_cast<string>(dv));
            mpShortcutTree->setCellWidgetType(pItem, 1, CustomTreeWidget::SHORTCUT_EDIT);
            pItem->setBackgroundColor(1, Qt::white);
            pItem->setText(0, QString::fromStdString(name));
            pItem->setText(1, shortcut);
         }
      }
   }
}

void OptionsShortcuts::shortcutsFromGui(QTreeWidgetItem* pCurObject, DynamicObject* pParent) const
{
   if (pCurObject->childCount() != 0)
   {
      FactoryResource<DynamicObject> pNewObject;
      string name = pCurObject->text(0).toStdString();
      pParent->setAttribute(name, *pNewObject);
      DynamicObject* pObject = dv_cast<DynamicObject>(&pParent->getAttribute(name));
      if (pObject != NULL)
      {
         for (int count = 0; count < pCurObject->childCount(); count++)
         {
            QTreeWidgetItem* pItem = pCurObject->child(count);
            shortcutsFromGui(pItem, pObject);
         }
      }
   }
   else
   {
      string name = pCurObject->text(0).toStdString();
      string shortcut = pCurObject->text(1).toStdString();
      pParent->setAttribute(name, shortcut);
   }
}
