/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMenu>
#include <QtGui/QPainter>

#include "DataVariantEditor.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServices.h"
#include "WizardGraphicsItem.h"
#include "WizardItemPalette.h"

#include <map>
#include <string>
#include <vector>
using namespace std;

WizardItemPalette::WizardItemPalette(QWidget* pParent) :
   QToolBox(pParent)
{
   WizardItemList* pAllPalette = new WizardItemList(this);
   addItem(pAllPalette, "All");

   int displayMode = 1;
   if (WizardItemPalette::hasSettingDisplayMode() == true)
   {
      displayMode = WizardItemPalette::getSettingDisplayMode();
   }

   if (displayMode == 0)
   {
      pAllPalette->enableListMode();
   }
   else if (displayMode == 1)
   {
      pAllPalette->enableIconMode();
   }

   // Add plug-ins
   Service<PlugInManagerServices> pManager;
   multimap<string, pair<string, QIcon> > plugInItems;

   vector<PlugInDescriptor*> plugIns = pManager->getPlugInDescriptors();
   for (vector<PlugInDescriptor*>::iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = *iter;
      if ((pDescriptor != NULL) && (pDescriptor->hasWizardSupport() == true))
      {
         string name = pDescriptor->getName();
         string type = pDescriptor->getType();
         QIcon plugInIcon = pDescriptor->getIcon();

         if ((name.empty() == false) && (type.empty() == false))
         {
            // Sort the types alphabetically by adding them to a map
            plugInItems.insert(pair<string, pair<string, QIcon> >(type, pair<string, QIcon>(name, plugInIcon)));
         }
      }
   }

   for (multimap<string, pair<string, QIcon> >::iterator iter = plugInItems.begin(); iter != plugInItems.end(); ++iter)
   {
      QString type = QString::fromStdString(iter->first);
      QString name = QString::fromStdString(iter->second.first);

      QIcon plugInIcon = iter->second.second;
      if (plugInIcon.isNull() == true)
      {
         QPixmap tmpPixmap(":/icons/WizardItem");
         QPixmap iconPixmap(tmpPixmap.size());
         QColor itemColor = WizardGraphicsItem::getItemBackgroundColor(type);
         iconPixmap.fill(itemColor);

         QPainter painter(&iconPixmap);
         painter.drawPixmap(0, 0, tmpPixmap);
         painter.end();

         plugInIcon = QIcon(iconPixmap);
      }

      // Add the plug-in item to the palette containing all items
      QListWidgetItem* pAllItem = new QListWidgetItem(name, pAllPalette);
      pAllItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
      pAllItem->setIcon(plugInIcon);
      pAllItem->setData(Qt::UserRole, type);
      pAllItem->setToolTip(name);

      // Add the plug-in item to the plug-in type palette
      WizardItemList* pPalette = NULL;
      for (int i = 0; i < count(); ++i)
      {
         if (itemText(i) == type)
         {
            pPalette = dynamic_cast<WizardItemList*>(widget(i));
            break;
         }
      }

      if (pPalette == NULL)
      {
         pPalette = new WizardItemList(this);
         addItem(pPalette, type);

         if (displayMode == 0)
         {
            pPalette->enableListMode();
         }
         else if (displayMode == 1)
         {
            pPalette->enableIconMode();
         }
      }

      if (pPalette != NULL)
      {
         QListWidgetItem* pItem = new QListWidgetItem(name, pPalette);
         pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
         pItem->setIcon(plugInIcon);
         pItem->setData(Qt::UserRole, type);
         pItem->setToolTip(name);
      }
   }

   // Add value items
   QPixmap tmpPixmap(":/icons/WizardItem");
   QPixmap iconPixmap(tmpPixmap.size());
   QColor itemColor = WizardGraphicsItem::getItemBackgroundColor("Value");
   iconPixmap.fill(itemColor);

   QPainter painter(&iconPixmap);
   painter.drawPixmap(0, 0, tmpPixmap);
   painter.end();

   QIcon itemIcon(iconPixmap);

   const vector<DataVariantEditorDelegate>& delegates = DataVariantEditor::getDelegates();
   for (vector<DataVariantEditorDelegate>::const_iterator iter = delegates.begin(); iter != delegates.end(); ++iter)
   {
      QString typeName = QString::fromStdString(iter->getTypeName());

      // Add the value item to the palette containing all items
      QListWidgetItem* pAllItem = new QListWidgetItem(typeName, pAllPalette);
      pAllItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
      pAllItem->setIcon(itemIcon);
      pAllItem->setData(Qt::UserRole, "Value");
      pAllItem->setToolTip(typeName);

      // Add the value item to the plug-in type palette
      WizardItemList* pPalette = NULL;
      for (int i = 0; i < count(); ++i)
      {
         if (itemText(i) == "Value")
         {
            pPalette = dynamic_cast<WizardItemList*>(widget(i));
            break;
         }
      }

      if (pPalette == NULL)
      {
         pPalette = new WizardItemList(this);
         addItem(pPalette, "Value");

         if (displayMode == 0)
         {
            pPalette->enableListMode();
         }
         else if (displayMode == 1)
         {
            pPalette->enableIconMode();
         }
      }

      if (pPalette != NULL)
      {
         QListWidgetItem* pItem = new QListWidgetItem(typeName, pPalette);
         pItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
         pItem->setIcon(itemIcon);
         pItem->setData(Qt::UserRole, "Value");
         pItem->setToolTip(typeName);
      }
   }

   pAllPalette->sortItems();
}

WizardItemPalette::~WizardItemPalette()
{}

void WizardItemPalette::contextMenuEvent(QContextMenuEvent* pEvent)
{
   QMenu contextMenu(this);

   QAction* pIconAction = contextMenu.addAction("Icon Mode");
   pIconAction->setCheckable(true);

   QAction* pListAction = contextMenu.addAction("List Mode");
   pListAction->setCheckable(true);

   WizardItemList* pCurrentList = dynamic_cast<WizardItemList*>(currentWidget());
   if (pCurrentList != NULL)
   {
      if (pCurrentList->viewMode() == QListView::IconMode)
      {
         pIconAction->setChecked(true);
      }
      else
      {
         pListAction->setChecked(true);
      }
   }

   QAction* pInvokedAction = contextMenu.exec(pEvent->globalPos());
   if (pInvokedAction != NULL)
   {
      for (int i = 0; i < count(); ++i)
      {
         WizardItemList* pList = dynamic_cast<WizardItemList*>(widget(i));
         if (pList != NULL)
         {
            if (pInvokedAction == pIconAction)
            {
               pList->enableIconMode();
            }
            else if (pInvokedAction == pListAction)
            {
               pList->enableListMode();
            }
         }
      }

      if (pInvokedAction == pIconAction)
      {
         WizardItemPalette::setSettingDisplayMode(1);
      }
      else if (pInvokedAction == pListAction)
      {
         WizardItemPalette::setSettingDisplayMode(0);
      }
   }
}

WizardItemPalette::WizardItemList::WizardItemList(QWidget* pParent) :
   QListWidget(pParent)
{
   enableIconMode();
   setSelectionMode(QAbstractItemView::ExtendedSelection);
   setSelectionBehavior(QAbstractItemView::SelectItems);
}

WizardItemPalette::WizardItemList::~WizardItemList()
{}

void WizardItemPalette::WizardItemList::enableIconMode()
{
   setViewMode(QListView::IconMode);
   setGridSize(QSize(85, 70));
   setMovement(QListView::Static);
   setResizeMode(QListView::Adjust);
   setWordWrap(true);
   setDragEnabled(true);
   setDragDropMode(QAbstractItemView::DragOnly);
}

void WizardItemPalette::WizardItemList::enableListMode()
{
   setViewMode(QListView::ListMode);
   setGridSize(QSize());
   setMovement(QListView::Static);
   setResizeMode(QListView::Fixed);
   setWordWrap(false);
   setDragEnabled(true);
   setDragDropMode(QAbstractItemView::DragOnly);
}

QSize WizardItemPalette::WizardItemList::sizeHint() const
{
   QSize widgetSize = QListWidget::sizeHint();
   widgetSize.setWidth(280);     // This width allows for three columns to appear by default in icon mode

   return widgetSize;
}

QStringList WizardItemPalette::WizardItemList::mimeTypes() const
{
   return QStringList("text/x-wizarditem");
}

QMimeData* WizardItemPalette::WizardItemList::mimeData(const QList<QListWidgetItem*> items) const
{
   QStringList itemNames;
   for (int i = 0; i < items.count(); ++i)
   {
      QListWidgetItem* pItem = items[i];
      if (pItem != NULL)
      {
         QString itemText = pItem->text();

         QString itemType = pItem->data(Qt::UserRole).toString();
         if (itemType.isEmpty() == false)
         {
            itemText.prepend(itemType + "/");
         }

         if (itemText.isEmpty() == false)
         {
            itemNames.append(itemText);
         }
      }
   }

   if (itemNames.empty() == true)
   {
      return NULL;
   }

   QString dataText = itemNames.join("@@!!@@");

   QMimeData* pData = new QMimeData();
   pData->setData("text/x-wizarditem", dataText.toAscii());

   return pData;
}
