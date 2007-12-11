/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QIcon>
#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "DesktopServices.h"
#include "SessionItem.h"
#include "SessionItemImp.h"
#include "SessionItemModel.h"

using namespace std;

//////////////////////
// SessionItemModel //
//////////////////////

SessionItemModel::SessionItemModel(QObject* pParent) :
   QAbstractItemModel(pParent),
   mpRootWrapper(new SessionItemWrapper(this, NULL))
{
}

SessionItemModel::~SessionItemModel()
{
   delete mpRootWrapper;
}

QModelIndex SessionItemModel::index(SessionItem* pItem) const
{
   SessionItemWrapper* pWrapper = getWrapper(pItem);
   if (pWrapper != NULL)
   {
      // Create a model index based on the index of the item with its siblings
      SessionItemWrapper* pParentWrapper = pWrapper->getParent();
      if (pParentWrapper != NULL)
      {
         const vector<SessionItemWrapper*>& wrappers = pParentWrapper->getChildren();
         for (vector<SessionItemWrapper*>::size_type i = 0; i < wrappers.size(); ++i)
         {
            SessionItemWrapper* pCurrentWrapper = wrappers[i];
            if (pCurrentWrapper == pWrapper)
            {
               return createIndex(static_cast<int>(i), 0, pCurrentWrapper);
            }
         }
      }
   }

   return QModelIndex();
}

QModelIndex SessionItemModel::index(int row, int column, const QModelIndex& parent) const
{
   SessionItemWrapper* pParent = NULL;
   if (parent.isValid() == true)
   {
      pParent = reinterpret_cast<SessionItemWrapper*>(parent.internalPointer());
   }
   else
   {
      pParent = mpRootWrapper;
   }

   if (pParent != NULL)
   {
      const vector<SessionItemWrapper*>& children = pParent->getChildren();
      if (static_cast<int>(children.size()) > row)
      {
         SessionItemWrapper* pChild = children[row];
         if (pChild != NULL)
         {
            return createIndex(row, column, pChild);
         }
      }
   }

   return QModelIndex();
}

QModelIndex SessionItemModel::parent(const QModelIndex& index) const
{
   if (index.isValid() == true)
   {
      SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
      if (pWrapper != NULL)
      {
         // Get the parent data
         SessionItemWrapper* pParent = pWrapper->getParent();
         if ((pParent != NULL) && (pParent != mpRootWrapper))
         {
            // Get the parent row by iterating over the grandparent
            SessionItemWrapper* pGrandparent = pParent->getParent();
            if (pGrandparent != NULL)
            {
               int row = -1;

               const vector<SessionItemWrapper*>& children = pGrandparent->getChildren();
               for (vector<SessionItemWrapper*>::size_type i = 0; i < children.size(); ++i)
               {
                  SessionItemWrapper* pCurrentWrapper = children[i];
                  if (pCurrentWrapper == pParent)
                  {
                     row = i;
                     break;
                  }
               }

               return createIndex(row, 0, pParent);
            }
         }
      }
   }

   return QModelIndex();
}

int SessionItemModel::rowCount(const QModelIndex& parent) const
{
   SessionItemWrapper* pParent = NULL;
   if (parent.isValid() == true)
   {
      pParent = reinterpret_cast<SessionItemWrapper*>(parent.internalPointer());
   }
   else
   {
      pParent = mpRootWrapper;
   }

   if (pParent != NULL)
   {
      return pParent->getChildren().size();
   }

   return 0;
}

int SessionItemModel::columnCount(const QModelIndex& parent) const
{
   return 1;
}

Qt::ItemFlags SessionItemModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags flag = QAbstractItemModel::flags(index);
   if (index.isValid())
   {
      SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
      if (pWrapper != NULL)
      {
         SessionItemImp* pItem = dynamic_cast<SessionItemImp*>(pWrapper->getSessionItem());
         if (pItem != NULL && pItem->canRename())
         {
            flag |= Qt::ItemIsEditable;
         }
      }
   }
   return flag;
}

bool SessionItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if (!index.isValid() || role != Qt::EditRole)
   {
      return false;
   }
   QString newItemName = value.toString();
   if (newItemName.isEmpty())
   {
      return false;
   }
   QString oldName = data(index, role).toString();
   if (newItemName == oldName)
   {
      return true;
   }

   SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
   if (pWrapper == NULL)
   {
      return false;
   }
   SessionItemImp* pItem = dynamic_cast<SessionItemImp*>(pWrapper->getSessionItem());
   if (pItem == NULL || !pItem->canRename())
   {
      return false;
   }
   string errorMessage;
   if(!pItem->rename(newItemName.toStdString(), errorMessage))
   {
      QString msg = QString("Unable to rename \"%1\" to \"%2\"").arg(oldName).arg(newItemName);
      if(!errorMessage.empty())
      {
         msg += "\n" + QString::fromStdString(errorMessage);
      }
      QMessageBox::warning(Service<DesktopServices>()->getMainWidget(), "Rename item", msg);
      return false;
   }
   emit dataChanged(index, index);
   return true;
}

QVariant SessionItemModel::data(const QModelIndex& index, int role) const
{
   if (index.isValid() == false)
   {
      return QVariant();
   }

   SessionItemWrapper* pWrapper = reinterpret_cast<SessionItemWrapper*>(index.internalPointer());
   if (pWrapper != NULL)
   {
      SessionItem* pItem = pWrapper->getSessionItem();
      if (role == Qt::DecorationRole)
      {
         if (pItem != NULL)
         {
            const QIcon& itemIcon = pItem->getIcon();
            return QVariant(itemIcon);
         }
      }
      else if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
         QString strItemName = pWrapper->getDisplayName();
         if ((strItemName.isEmpty() == true) && (pItem != NULL))
         {
            string itemName = pItem->getDisplayName();
            if (itemName.empty() == true)
            {
               itemName = pItem->getName();
            }

            strItemName = QString::fromStdString(itemName);
         }

         if (strItemName.isEmpty() == false)
         {
            return QVariant(strItemName);
         }
      }
      else if (role == Qt::FontRole)
      {
         QFont itemFont = pWrapper->getDisplayFont();
         return QVariant(itemFont);
      }
      else if (role == Qt::ForegroundRole)
      {
         QColor itemColor = pWrapper->getDisplayColor();
         return QVariant(itemColor);
      }
      else if (role == Qt::ToolTipRole)
      {
         if (pItem != NULL)
         {
            const string& displayText = pItem->getDisplayText();
            if (displayText.empty() == false)
            {
               return QVariant(QString::fromStdString(displayText));
            }
         }
      }
      else if (role == Qt::UserRole)
      {
         return QVariant::fromValue(pItem);
      }
   }

   return QVariant();
}

void SessionItemModel::updateData(SessionItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   QModelIndex itemIndex = index(pItem);
   if (itemIndex.isValid() == true)
   {
      emit dataChanged(itemIndex, itemIndex);
   }
}

SessionItemModel::SessionItemWrapper* SessionItemModel::getWrapper(SessionItem* pItem) const
{
   map<SessionItem*, SessionItemWrapper*>::const_iterator iter = mSessionItems.find(pItem);
   if (iter != mSessionItems.end())
   {
      return iter->second;
   }

   return NULL;
}

SessionItemModel::SessionItemWrapper* SessionItemModel::getRootWrapper() const
{
   return mpRootWrapper;
}

void SessionItemModel::activateItem(SessionItem* pItem)
{
   SessionItemWrapper* pWrapper = getWrapper(pItem);
   if (pWrapper == NULL)
   {
      return;
   }

   SessionItemWrapper* pParentWrapper = pWrapper->getParent();
   if (pParentWrapper == NULL)
   {
      return;
   }

   const vector<SessionItemWrapper*>& wrappers = pParentWrapper->getChildren();
   for (vector<SessionItemWrapper*>::size_type i = 0; i < wrappers.size(); ++i)
   {
      SessionItemWrapper* pCurrentWrapper = wrappers[i];
      if (pCurrentWrapper != NULL)
      {
         QFont currentFont = pCurrentWrapper->getDisplayFont();

         QFont itemFont = currentFont;
         itemFont.setBold(pCurrentWrapper == pWrapper);

         if (currentFont != itemFont)
         {
            pCurrentWrapper->setDisplayFont(itemFont);

            QModelIndex index = createIndex(static_cast<int>(i), 0, pCurrentWrapper);
            emit dataChanged(index, index);
         }
      }
   }
}

void SessionItemModel::clear()
{
   mpRootWrapper->clear();
   mSessionItems.clear();
}

////////////////////////
// SessionItemWrapper //
////////////////////////

SessionItemModel::SessionItemWrapper::SessionItemWrapper(SessionItemModel* pModel, SessionItem* pItem,
                                                         SessionItemWrapper* pParent) :
   mpModel(pModel),
   mpSessionItem(pItem),
   mDisplayColor(Qt::black),
   mpParent(pParent)
{
}

SessionItemModel::SessionItemWrapper::~SessionItemWrapper()
{
   clear();
}

SessionItem* SessionItemModel::SessionItemWrapper::getSessionItem() const
{
   return mpSessionItem;
}

void SessionItemModel::SessionItemWrapper::setDisplayName(const QString& itemName)
{
   mDisplayName = itemName;
}

QString SessionItemModel::SessionItemWrapper::getDisplayName() const
{
   return mDisplayName;
}

void SessionItemModel::SessionItemWrapper::setDisplayFont(const QFont& itemFont)
{
   mDisplayFont = itemFont;
}

QFont SessionItemModel::SessionItemWrapper::getDisplayFont() const
{
   return mDisplayFont;
}

void SessionItemModel::SessionItemWrapper::setDisplayColor(const QColor& itemColor)
{
   if (itemColor.isValid() == true)
   {
      mDisplayColor = itemColor;
   }
}

QColor SessionItemModel::SessionItemWrapper::getDisplayColor() const
{
   return mDisplayColor;
}

SessionItemModel::SessionItemWrapper* SessionItemModel::SessionItemWrapper::getParent() const
{
   return mpParent;
}

const vector<SessionItemModel::SessionItemWrapper*>& SessionItemModel::SessionItemWrapper::getChildren() const
{
   return mChildren;
}

void SessionItemModel::SessionItemWrapper::addChild(SessionItemWrapper* pWrapper)
{
   if ((pWrapper == NULL) || (mpModel == NULL))
   {
      return;
   }

   QModelIndex parent;
   if (mpParent != NULL)
   {
      const vector<SessionItemWrapper*>& siblings = mpParent->getChildren();
      for (vector<SessionItemWrapper*>::size_type i = 0; i < siblings.size(); ++i)
      {
         SessionItemWrapper* pCurrentWrapper = siblings[i];
         if (pCurrentWrapper == this)
         {
            parent = mpModel->createIndex(static_cast<int>(i), 0, this);
            break;
         }
      }
   }

   int row = static_cast<int>(mChildren.size());
   mpModel->beginInsertRows(parent, row, row);

   SessionItem* pItem = pWrapper->getSessionItem();
   if (pItem != NULL)
   {
      map<SessionItem*, SessionItemWrapper*>::iterator iter = mpModel->mSessionItems.find(pItem);
      if (iter == mpModel->mSessionItems.end())
      {
         mpModel->mSessionItems[pItem] = pWrapper;
      }

      VERIFYNR_MSG(iter == mpModel->mSessionItems.end(),
         "SessionItemModel does not currently support multiple indexes for the same session item!");
   }

   mChildren.push_back(pWrapper);
   mpModel->endInsertRows();
}

SessionItemModel::SessionItemWrapper* SessionItemModel::SessionItemWrapper::addChild(SessionItem* pItem)
{
   if ((pItem == NULL) || (mpModel == NULL))
   {
      return NULL;
   }

   if (getChild(pItem) != NULL)
   {
      return NULL;
   }

   SessionItemWrapper* pChildWrapper = new SessionItemWrapper(mpModel, pItem, this);
   if (pChildWrapper != NULL)
   {
      addChild(pChildWrapper);
   }

   return pChildWrapper;
}

SessionItemModel::SessionItemWrapper* SessionItemModel::SessionItemWrapper::getChild(SessionItem* pItem) const
{
   if (pItem == NULL)
   {
      return NULL;
   }

   for (vector<SessionItemWrapper*>::const_iterator iter = mChildren.begin(); iter != mChildren.end(); ++iter)
   {
      SessionItemWrapper* pWrapper = *iter;
      VERIFYRV(pWrapper != NULL, NULL);

      SessionItem* pCurrentItem = pWrapper->getSessionItem();
      if (pCurrentItem == pItem)
      {
         return pWrapper;
      }
   }

   return NULL;
}

void SessionItemModel::SessionItemWrapper::removeChild(SessionItemWrapper* pWrapper)
{
   if ((pWrapper == NULL) || (mpModel == NULL))
   {
      return;
   }

   int row = 0;
   vector<SessionItemWrapper*>::reverse_iterator iter;
   for (iter = mChildren.rbegin(), row = static_cast<int>(mChildren.size()) - 1; iter != mChildren.rend(); ++iter, -row)
   {
      SessionItemWrapper* pCurrentWrapper = *iter;
      if (pCurrentWrapper == pWrapper)
      {
         // Remove any child items
         vector<SessionItemWrapper*> children = pWrapper->getChildren();
         for (vector<SessionItemWrapper*>::size_type i = 0; i < children.size(); ++i)
         {
            SessionItemWrapper* pChildWrapper = children[i];
            if (pChildWrapper != NULL)
            {
               pWrapper->removeChild(pChildWrapper);
            }
         }

         // Get the parent index
         QModelIndex parent;
         if (mpParent != NULL)
         {
            const vector<SessionItemWrapper*>& siblings = mpParent->getChildren();
            for (vector<SessionItemWrapper*>::size_type i = 0; i < siblings.size(); ++i)
            {
               SessionItemWrapper* pSiblingWrapper = siblings[i];
               if (pSiblingWrapper == this)
               {
                  parent = mpModel->createIndex(static_cast<int>(i), 0, this);
                  break;
               }
            }
         }

         // Remove the item
         mpModel->beginRemoveRows(parent, row, row);

         SessionItem* pItem = pWrapper->getSessionItem();
         if (pItem != NULL)
         {
            map<SessionItem*, SessionItemWrapper*>::iterator mapIter = mpModel->mSessionItems.find(pItem);
            if (mapIter != mpModel->mSessionItems.end())
            {
               mpModel->mSessionItems.erase(pItem);
            }
         }

         vector<SessionItemWrapper*>::iterator fwdIter = iter.base();
         mChildren.erase(--fwdIter);
         mpModel->endRemoveRows();
         delete pWrapper;
         break;
      }
   }
}

void SessionItemModel::SessionItemWrapper::removeChild(SessionItem* pItem)
{
   if (pItem == NULL)
   {
      return;
   }

   SessionItemWrapper* pWrapper = getChild(pItem);
   if (pWrapper != NULL)
   {
      removeChild(pWrapper);
   }
}

void SessionItemModel::SessionItemWrapper::clear()
{
   // Delete the child wrappers
   while (mChildren.empty() == false)
   {
      SessionItemWrapper* pWrapper = mChildren.back();
      VERIFYNRV(pWrapper != NULL);

      removeChild(pWrapper);
   }
}
