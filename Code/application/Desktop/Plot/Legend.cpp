/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QHeaderView>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>

#include "Legend.h"
#include "PlotObject.h"
#include "PlotObjectImp.h"

Legend::Legend(QWidget* parent) :
   QTreeWidget(parent),
   mSecondaryObjects(false)
{
   setColumnCount(1);
   setRootIsDecorated(false);
   setSelectionMode(QAbstractItemView::NoSelection);
   setSortingEnabled(false);
   setFocusPolicy(Qt::NoFocus);

   QHeaderView* pHeader = header();
   if (pHeader != NULL)
   {
      pHeader->setMovable(false);
      pHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      pHeader->hide();
   }
}

Legend::~Legend()
{
}

void Legend::showSecondaryObjects(bool bShow)
{
   mSecondaryObjects = bShow;
}

bool Legend::areSecondaryObjectsShown() const
{
   return mSecondaryObjects;
}

QColor Legend::getBackgroundColor() const
{
   QPalette legendPalette = palette();
   return legendPalette.color(QPalette::Base);
}

void Legend::setBackgroundColor(const QColor& backgroundColor)
{
   if (backgroundColor.isValid() == true)
   {
      QPalette legendPalette = palette();
      legendPalette.setColor(QPalette::Base, backgroundColor);
      setPalette(legendPalette);
   }
}

bool Legend::insertItem(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   if (mSecondaryObjects == false)
   {
      if (pObject->isPrimary() == false)
      {
         return false;
      }
   }

   PlotObjectImp* pObjectImp = dynamic_cast<PlotObjectImp*>(pObject);
   if (pObjectImp == NULL)
   {
      return false;
   }

   QString strName = pObjectImp->getObjectName();

   QTreeWidgetItem* pItem = new QTreeWidgetItem(this);
   if (pItem != NULL)
   {
      bool bSelected = pObjectImp->isSelected();
      const QPixmap& pix = pObjectImp->getLegendPixmap(bSelected);

      pItem->setIcon(0, QIcon(pix));
      pItem->setText(0, strName);

      VERIFYNR(connect(pObjectImp, SIGNAL(renamed(const QString&)), this, SLOT(updateItemText(const QString&))));
      VERIFYNR(connect(pObjectImp, SIGNAL(legendPixmapChanged()), this, SLOT(updateItemPixmap())));

      mObjects.insert(pObject, pItem);
      return true;
   }

   return false;
}

bool Legend::setItemText(PlotObject* pObject, const QString& strName)
{
   if (pObject == NULL)
   {
      return false;
   }

   QMap<PlotObject*, QTreeWidgetItem*>::Iterator iter = mObjects.find(pObject);
   if (iter != mObjects.end())
   {
      QTreeWidgetItem* pItem = iter.value();
      if (pItem != NULL)
      {
         pItem->setText(0, strName);
         return true;
      }
   }

   return false;
}

bool Legend::setItemSelected(PlotObject* pObject, bool bSelected)
{
   if (pObject == NULL)
   {
      return false;
   }

   QMap<PlotObject*, QTreeWidgetItem*>::Iterator iter = mObjects.find(pObject);
   if (iter != mObjects.end())
   {
      QTreeWidgetItem* pItem = iter.value();
      if (pItem != NULL)
      {
         const QPixmap& newPixmap = (dynamic_cast<PlotObjectImp*>(pObject))->getLegendPixmap(bSelected);
         QIcon currentIcon = pItem->icon(0);
         QPixmap currentPixmap = currentIcon.pixmap(newPixmap.size());

         bool bSet = true;
         if (currentPixmap.isNull() == false)
         {
            QImage newImage = newPixmap.toImage();
            QImage currentImage = currentPixmap.toImage();
            if (newImage == currentImage)
            {
               bSet = false;
            }
         }

         if (bSet == true)
         {
            pItem->setIcon(0, QIcon(newPixmap));
            emit itemSelected(pObject, bSelected);
            return true;
         }
      }
   }

   return false;
}

bool Legend::removeItem(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   QMap<PlotObject*, QTreeWidgetItem*>::Iterator iter = mObjects.find(pObject);
   if (iter != mObjects.end())
   {
      PlotObjectImp* pObjectImp = dynamic_cast<PlotObjectImp*>(pObject);
      if (pObjectImp != NULL)
      {
         VERIFYNR(disconnect(pObjectImp, SIGNAL(renamed(const QString&)), this, SLOT(updateItemText(const QString&))));
         VERIFYNR(disconnect(pObjectImp, SIGNAL(legendPixmapChanged()), this, SLOT(updateItemPixmap())));
      }

      QTreeWidgetItem* pItem = iter.value();
      if (pItem != NULL)
      {
         delete pItem;
      }

      mObjects.remove(pObject);
      resizeColumnToContents(0);
      return true;
   }

   return false;
}

void Legend::clear()
{
   QTreeWidget::clear();
   mObjects.clear();
   resizeColumnToContents(0);
}

void Legend::selectObject(QTreeWidgetItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }
   QMap<PlotObject*, QTreeWidgetItem*>::Iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      QTreeWidgetItem* pCurrentItem = iter.value();
      if (pCurrentItem == pItem)
      {
         PlotObject* pObject = iter.key();
         if (pObject != NULL)
         {
            bool bSelected = pObject->isSelected();
            setItemSelected(pObject, !bSelected);
         }
         
         break;
      }
      
      ++iter;
   }
}

void Legend::updateItemText(const QString& strObjectName)
{
   QMap<PlotObject*, QTreeWidgetItem*>::Iterator iter;
   iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObjectImp* pObject = dynamic_cast<PlotObjectImp*>(iter.key());
      if (pObject != NULL)
      {
         QString strCurrentName = pObject->getObjectName();
         if (strCurrentName == strObjectName)
         {
            QTreeWidgetItem* pItem = iter.value();
            if (pItem != NULL)
            {
               pItem->setText(0, strObjectName);
               pItem->setToolTip(0,strObjectName);
               break;
            }
         }
      }

      ++iter;
   }
}

void Legend::updateItemPixmap()
{
   PlotObjectImp* pObjectImp = dynamic_cast<PlotObjectImp*>(sender());
   if (pObjectImp != NULL)
   {
      QMap<PlotObject*, QTreeWidgetItem*>::iterator iter = mObjects.find(dynamic_cast<PlotObject*>(pObjectImp));
      if (iter != mObjects.end())
      {
         QTreeWidgetItem* pItem = iter.value();
         if (pItem != NULL)
         {
            const QPixmap& pix = pObjectImp->getLegendPixmap(pObjectImp->isSelected());
            pItem->setIcon(0, QIcon(pix));
         }
      }
   }
}

void Legend::mousePressEvent(QMouseEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->button() == Qt::LeftButton)
      {
         QTreeWidgetItem* pSelectedItem = itemAt(pEvent->pos());
         if (pSelectedItem != NULL)
         {
            selectObject(pSelectedItem);
         }
      }
   }
   QTreeWidget::mousePressEvent(pEvent);
}
