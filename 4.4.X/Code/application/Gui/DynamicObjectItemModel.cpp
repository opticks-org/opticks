/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DynamicObject.h"
#include "DynamicObjectItemModel.h"
#include "Slot.h"

#include <algorithm>
using namespace std;

DynamicObjectItemModel::DynamicObjectItemModel(QObject* pParent, DynamicObject* pDynamicObject) :
   QAbstractItemModel(pParent),
   mpDynamicObject(NULL),
   mpRootWrapper(new AttributeWrapper(this, "Root", NULL, NULL))
{
   // Initialization
   setDynamicObject(pDynamicObject);
}

DynamicObjectItemModel::~DynamicObjectItemModel()
{
   setDynamicObject(NULL);
}

QVariant DynamicObjectItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
   {
      if (section == 0)
      {
         return QVariant("Name");
      }
      else if (section == 1)
      {
         return QVariant("Type");
      }
      else if (section == 2)
      {
         return QVariant("Value");
      }
   }

   return QVariant();
}

QModelIndex DynamicObjectItemModel::index(DynamicObject* pDynamicObject) const
{
   AttributeWrapper* pWrapper = getWrapper(pDynamicObject);
   if (pWrapper != NULL)
   {
      // Create a model index based on the index of the item with its siblings
      AttributeWrapper* pParentWrapper = pWrapper->getParent();
      if (pParentWrapper != NULL)
      {
         const vector<AttributeWrapper*>& wrappers = pParentWrapper->getChildren();
         for (vector<AttributeWrapper*>::size_type i = 0; i < wrappers.size(); ++i)
         {
            AttributeWrapper* pCurrentWrapper = wrappers[i];
            if (pCurrentWrapper == pWrapper)
            {
               return createIndex(static_cast<int>(i), 0, pCurrentWrapper);
            }
         }
      }
   }

   return QModelIndex();
}

QModelIndex DynamicObjectItemModel::index(int row, int column, const QModelIndex& parent) const
{
   if ((row >= 0) && (row < rowCount(parent)) && (column >= 0) && (column < columnCount(parent)))
   {
      AttributeWrapper* pParentWrapper = mpRootWrapper;
      if (parent.isValid() == true)
      {
         pParentWrapper = reinterpret_cast<AttributeWrapper*>(parent.internalPointer());
      }

      if (pParentWrapper != NULL)
      {
         const vector<AttributeWrapper*>& children = pParentWrapper->getChildren();
         if (static_cast<int>(children.size()) > row)
         {
            AttributeWrapper* pChildWrapper = children[row];
            if (pChildWrapper != NULL)
            {
               return createIndex(row, column, pChildWrapper);
            }
         }
      }
   }

   return QModelIndex();
}

QModelIndex DynamicObjectItemModel::parent(const QModelIndex& index) const
{
   if (index.isValid() == true)
   {
      AttributeWrapper* pWrapper = reinterpret_cast<AttributeWrapper*>(index.internalPointer());
      if (pWrapper != NULL)
      {
         // Get the parent data
         AttributeWrapper* pParentWrapper = pWrapper->getParent();
         if ((pParentWrapper != NULL) && (pParentWrapper != mpRootWrapper))
         {
            // Get the parent row by iterating over the grandparent
            AttributeWrapper* pGrandparentWrapper = pParentWrapper->getParent();
            if (pGrandparentWrapper != NULL)
            {
               int row = -1;

               const vector<AttributeWrapper*>& children = pGrandparentWrapper->getChildren();
               for (vector<AttributeWrapper*>::size_type i = 0; i < children.size(); ++i)
               {
                  AttributeWrapper* pCurrentWrapper = children[i];
                  if (pCurrentWrapper == pParentWrapper)
                  {
                     row = i;
                     break;
                  }
               }

               return createIndex(row, index.column(), pParentWrapper);
            }
         }
      }
   }

   return QModelIndex();
}

int DynamicObjectItemModel::rowCount(const QModelIndex& parent) const
{
   AttributeWrapper* pParentWrapper = mpRootWrapper;
   if (parent.isValid() == true)
   {
      pParentWrapper = reinterpret_cast<AttributeWrapper*>(parent.internalPointer());
   }

   if (pParentWrapper != NULL)
   {
      return pParentWrapper->getChildren().size();
   }

   return 0;
}

int DynamicObjectItemModel::columnCount(const QModelIndex& parent) const
{
   return 3;
}

QVariant DynamicObjectItemModel::data(const QModelIndex& index, int role) const
{
   if (index.isValid() == false)
   {
      return QVariant();
   }

   AttributeWrapper* pWrapper = reinterpret_cast<AttributeWrapper*>(index.internalPointer());
   if (pWrapper != NULL)
   {
      const string& attributeName = pWrapper->getName();
      DataVariant* pValue = pWrapper->getValue();

      if (role == Qt::DisplayRole)
      {
         int column = index.column();
         if (column == 0)
         {
            // Name
            return QVariant(QString::fromStdString(attributeName));
         }
         else if ((pValue != NULL) && (pValue->isValid() == true))
         {
            string attributeType = pValue->getTypeName();
            if (column == 1)
            {
               // Type
               return QVariant(QString::fromStdString(attributeType));
            }
            else if (column == 2)
            {
               // Value
               if (attributeType == "DynamicObject")
               {
                  DynamicObject* pObject = dv_cast<DynamicObject>(pValue);
                  if (pObject != NULL)
                  {
                     QString valueText = QString::number(pObject->getNumAttributes()) + " children";
                     return QVariant(valueText);
                  }
               }
               else
               {
                  string attributeValue = pValue->toDisplayString();
                  return QVariant(QString::fromStdString(attributeValue));
               }
            }
         }
      }
      else if (role == Qt::UserRole)
      {
         return QVariant::fromValue(pValue);
      }
   }

   return QVariant();
}

void DynamicObjectItemModel::setDynamicObject(DynamicObject* pDynamicObject)
{
   if (pDynamicObject == mpDynamicObject)
   {
      return;
   }

   // Clear the attribute items and the dynamic object map
   clear();

   // Detach from the previous dynamic object
   if (mpDynamicObject != NULL)
   {
      // All other signal/slot connections were detached when the dynamic object map was cleared
      mpDynamicObject->detach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &DynamicObjectItemModel::dynamicObjectDeleted));
   }

   mpDynamicObject = pDynamicObject;

   if (mpDynamicObject != NULL)
   {
      // Add the attribute items
      vector<string> attributeNames;
      mpDynamicObject->getAttributeNames(attributeNames);
      for (vector<string>::size_type i = 0; i < attributeNames.size(); ++i)
      {
         string attributeName = attributeNames[i];
         if (attributeName.empty() == false)
         {
            DataVariant& attributeValue = mpDynamicObject->getAttribute(attributeName);
            if (attributeValue.isValid() == true)
            {
               addAttributeItem(mpRootWrapper, attributeName, &attributeValue);
            }
         }
      }

      // Add the dynamic object to the map
      map<DynamicObject*, AttributeWrapper*>::iterator iter = mDynamicObjects.find(pDynamicObject);
      if (iter == mDynamicObjects.end())
      {
         mDynamicObjects[pDynamicObject] = mpRootWrapper;
      }

      // Connections
      mpDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeAdded),
         Slot(this, &DynamicObjectItemModel::addAttribute));
      mpDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeModified),
         Slot(this, &DynamicObjectItemModel::modifyAttribute));
      mpDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeRemoved),
         Slot(this, &DynamicObjectItemModel::removeAttribute));
      mpDynamicObject->attach(SIGNAL_NAME(DynamicObject, Cleared),
         Slot(this, &DynamicObjectItemModel::removeAllAttributes));
      mpDynamicObject->attach(SIGNAL_NAME(Subject, Deleted),
         Slot(this, &DynamicObjectItemModel::dynamicObjectDeleted));
   }
}

void DynamicObjectItemModel::addAttribute(Subject& subject, const string& signal, const boost::any& value)
{
   DynamicObject* pDynamicObject = dynamic_cast<DynamicObject*>(&subject);
   if (pDynamicObject == NULL)
   {
      return;
   }

   pair<string, DataVariant*> attribute = boost::any_cast<pair<string, DataVariant*> >(value);

   string attributeName = attribute.first;
   DataVariant* pAttributeValue = attribute.second;
   if ((attributeName.empty() == true) || (pAttributeValue == NULL) || (pAttributeValue->isValid() == false))
   {
      return;
   }

   AttributeWrapper* pParentWrapper = getWrapper(pDynamicObject);
   if (pParentWrapper != NULL)
   {
      addAttributeItem(pParentWrapper, attributeName, pAttributeValue);
   }
}

void DynamicObjectItemModel::modifyAttribute(Subject& subject, const string& signal, const boost::any& value)
{
   DynamicObject* pDynamicObject = dynamic_cast<DynamicObject*>(&subject);
   if (pDynamicObject == NULL)
   {
      return;
   }

   pair<string, DataVariant*> attribute = boost::any_cast<pair<string, DataVariant*> >(value);

   string attributeName = attribute.first;
   DataVariant* pAttributeValue = attribute.second;
   if ((attributeName.empty() == true) || (pAttributeValue == NULL) || (pAttributeValue->isValid() == false))
   {
      return;
   }

   // Get the dynamic object wrapper
   AttributeWrapper* pDynamicObjectWrapper = getWrapper(pDynamicObject);
   if (pDynamicObjectWrapper == NULL)
   {
      return;
   }

   // Update the value in the wrapper and get its row
   int row = 0;

   const vector<AttributeWrapper*>& wrappers = pDynamicObjectWrapper->getChildren();
   for (vector<AttributeWrapper*>::size_type i = 0; i < wrappers.size(); ++i)
   {
      AttributeWrapper* pWrapper = wrappers[i];
      if (pWrapper != NULL)
      {
         const string& currentName = pWrapper->getName();
         if (currentName == attributeName)
         {
            pWrapper->setValue(pAttributeValue);
            row = static_cast<int>(i);
            break;
         }
      }
   }

   // Get the start and end indexes for the attribute row
   QModelIndex parent = index(pDynamicObject);
   QModelIndex startIndex = index(row, 1, parent);
   QModelIndex endIndex = index(row, 2, parent);

   // Update the row
   if ((startIndex.isValid() == true) && (endIndex.isValid() == true))
   {
      emit dataChanged(startIndex, endIndex);
   }
}

void DynamicObjectItemModel::removeAttribute(Subject& subject, const string& signal, const boost::any& value)
{
   DynamicObject* pDynamicObject = dynamic_cast<DynamicObject*>(&subject);
   if (pDynamicObject == NULL)
   {
      return;
   }

   pair<string, DataVariant*> attribute = boost::any_cast<pair<string, DataVariant*> >(value);

   string attributeName = attribute.first;
   DataVariant* pAttributeValue = attribute.second;
   if ((attributeName.empty() == true) || (pAttributeValue == NULL) || (pAttributeValue->isValid() == false))
   {
      return;
   }

   AttributeWrapper* pParentWrapper = getWrapper(pDynamicObject);
   if (pParentWrapper != NULL)
   {
      removeAttributeItem(pParentWrapper, attributeName, pAttributeValue);
   }
}

void DynamicObjectItemModel::removeAllAttributes(Subject& subject, const string& signal, const boost::any& value)
{
   DynamicObject* pDynamicObject = dynamic_cast<DynamicObject*>(&subject);
   if (pDynamicObject == NULL)
   {
      return;
   }

   AttributeWrapper* pParentWrapper = getWrapper(pDynamicObject);
   if (pParentWrapper != NULL)
   {
      vector<AttributeWrapper*> attributes = pParentWrapper->getChildren();
      for (vector<AttributeWrapper*>::size_type i = 0; i < attributes.size(); ++i)
      {
         AttributeWrapper* pWrapper = attributes[i];
         if (pWrapper != NULL)
         {
            const string& attributeName = pWrapper->getName();
            DataVariant* pAttributeValue = pWrapper->getValue();

            removeAttributeItem(pParentWrapper, attributeName, pAttributeValue);
         }
      }
   }
}

void DynamicObjectItemModel::dynamicObjectDeleted(Subject& subject, const string& signal, const boost::any& value)
{
   DynamicObject* pDynamicObject = dynamic_cast<DynamicObject*>(&subject);
   if ((pDynamicObject != NULL) && (pDynamicObject == mpDynamicObject))
   {
      setDynamicObject(NULL);
   }
}

DynamicObjectItemModel::AttributeWrapper*
DynamicObjectItemModel::addAttributeItem(AttributeWrapper* pParentWrapper, const string& name, DataVariant* pValue)
{
   if ((pParentWrapper == NULL) || (name.empty() == true) || (pValue == NULL))
   {
      return NULL;
   }

   // Add the attribute item
   AttributeWrapper* pWrapper = pParentWrapper->addChild(name, pValue);
   if ((pWrapper != NULL) && (pValue->isValid() == true))
   {
      // Initialize a DynamicObject item
      string attributeType = pValue->getTypeName();
      if (attributeType == "DynamicObject")
      {
         DynamicObject* pDynamicObject = dv_cast<DynamicObject>(pValue);
         if (pDynamicObject != NULL)
         {
            // Add the object to the map
            map<DynamicObject*, AttributeWrapper*>::iterator iter = mDynamicObjects.find(pDynamicObject);
            if (iter == mDynamicObjects.end())
            {
               mDynamicObjects[pDynamicObject] = pWrapper;
            }

            // Add items for the children
            vector<string> attributeNames;
            pDynamicObject->getAttributeNames(attributeNames);
            for (vector<string>::size_type i = 0; i < attributeNames.size(); ++i)
            {
               string attributeName = attributeNames[i];
               if (attributeName.empty() == false)
               {
                  DataVariant& attributeValue = pDynamicObject->getAttribute(attributeName);
                  if (attributeValue.isValid() == true)
                  {
                     addAttributeItem(pWrapper, attributeName, &attributeValue);
                  }
               }
            }

            // Connections
            pDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeAdded),
               Slot(this, &DynamicObjectItemModel::addAttribute));
            pDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeModified),
               Slot(this, &DynamicObjectItemModel::modifyAttribute));
            pDynamicObject->attach(SIGNAL_NAME(DynamicObject, AttributeRemoved),
               Slot(this, &DynamicObjectItemModel::removeAttribute));
            pDynamicObject->attach(SIGNAL_NAME(DynamicObject, Cleared),
               Slot(this, &DynamicObjectItemModel::removeAllAttributes));
         }
      }
   }

   return pWrapper;
}

void DynamicObjectItemModel::removeAttributeItem(AttributeWrapper* pParentWrapper, const string& name,
                                                 DataVariant* pValue)
{
   if ((pParentWrapper == NULL) || (name.empty() == true) || (pValue == NULL))
   {
      return;
   }

   // Get the dynamic object index
   QModelIndex parent;

   DataVariant* pParentValue = pParentWrapper->getValue();
   if ((pParentValue != NULL) && (pParentValue->isValid() == true))
   {
      if (pParentValue->getTypeName() == "DynamicObject")
      {
         DynamicObject* pDynamicObject = dv_cast<DynamicObject>(pParentValue);
         if (pDynamicObject != NULL)
         {
            parent = index(pDynamicObject);
         }
      }
   }

   // Get the attribute wrapper of the attribute to remove
   AttributeWrapper* pWrapper = pParentWrapper->getChild(name, pValue);
   if (pWrapper == NULL)
   {
      return;
   }

   // Cleanup for a DynamicObject
   if (pValue->isValid() == true)
   {
      string attributeType = pValue->getTypeName();
      if (attributeType == "DynamicObject")
      {
         DynamicObject* pDynamicObject = dv_cast<DynamicObject>(pValue);
         if (pDynamicObject != NULL)
         {
            // Detach from the object
            pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeAdded),
               Slot(this, &DynamicObjectItemModel::addAttribute));
            pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeModified),
               Slot(this, &DynamicObjectItemModel::modifyAttribute));
            pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeRemoved),
               Slot(this, &DynamicObjectItemModel::removeAttribute));
            pDynamicObject->detach(SIGNAL_NAME(DynamicObject, Cleared),
               Slot(this, &DynamicObjectItemModel::removeAllAttributes));

            // Remove any children items
            vector<AttributeWrapper*> children = pWrapper->getChildren();
            for (vector<AttributeWrapper*>::size_type i = 0; i < children.size(); ++i)
            {
               AttributeWrapper* pCurrentWrapper = children[i];
               if (pCurrentWrapper != NULL)
               {
                  const string& currentName = pCurrentWrapper->getName();
                  DataVariant* pCurrentValue = pCurrentWrapper->getValue();

                  if ((currentName.empty() == false) && (pCurrentValue != NULL) && (pCurrentValue->isValid() == true))
                  {
                     removeAttributeItem(pWrapper, currentName, pCurrentValue);
                  }
               }
            }

            // Remove the object from the map
            map<DynamicObject*, AttributeWrapper*>::iterator iter = mDynamicObjects.find(pDynamicObject);
            if (iter != mDynamicObjects.end())
            {
               mDynamicObjects.erase(iter);
            }
         }
      }
   }

   // Remove the attribute
   pParentWrapper->removeChild(pWrapper);
}

DynamicObjectItemModel::AttributeWrapper* DynamicObjectItemModel::getWrapper(DynamicObject* pDynamicObject) const
{
   map<DynamicObject*, AttributeWrapper*>::const_iterator iter = mDynamicObjects.find(pDynamicObject);
   if (iter != mDynamicObjects.end())
   {
      return iter->second;
   }

   return NULL;
}

void DynamicObjectItemModel::clear()
{
   // Delete all attribute items
   mpRootWrapper->clear();

   // Detach from DynamicObject items
   for (map<DynamicObject*, AttributeWrapper*>::const_iterator iter = mDynamicObjects.begin();
      iter != mDynamicObjects.end();
      ++iter)
   {
      DynamicObject* pDynamicObject = iter->first;
      if (pDynamicObject != NULL)
      {
         pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeAdded),
            Slot(this, &DynamicObjectItemModel::addAttribute));
         pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeModified),
            Slot(this, &DynamicObjectItemModel::modifyAttribute));
         pDynamicObject->detach(SIGNAL_NAME(DynamicObject, AttributeRemoved),
            Slot(this, &DynamicObjectItemModel::removeAttribute));
         pDynamicObject->detach(SIGNAL_NAME(DynamicObject, Cleared),
            Slot(this, &DynamicObjectItemModel::removeAllAttributes));
      }
   }

   // Clear the map
   mDynamicObjects.clear();
}

//////////////////////
// AttributeWrapper //
//////////////////////

DynamicObjectItemModel::AttributeWrapper::AttributeWrapper(DynamicObjectItemModel* pModel, const string& name,
                                                           DataVariant* pValue, AttributeWrapper* pParent) :
   mpModel(pModel),
   mName(name),
   mpValue(pValue),
   mpParent(pParent)
{}

DynamicObjectItemModel::AttributeWrapper::~AttributeWrapper()
{
   clear();
}

const string& DynamicObjectItemModel::AttributeWrapper::getName() const
{
   return mName;
}

void DynamicObjectItemModel::AttributeWrapper::setValue(DataVariant* pValue)
{
   mpValue = pValue;
}

DataVariant* DynamicObjectItemModel::AttributeWrapper::getValue() const
{
   return mpValue;
}

DynamicObjectItemModel::AttributeWrapper* DynamicObjectItemModel::AttributeWrapper::getParent() const
{
   return mpParent;
}

const vector<DynamicObjectItemModel::AttributeWrapper*>& DynamicObjectItemModel::AttributeWrapper::getChildren() const
{
   return mChildren;
}

void DynamicObjectItemModel::AttributeWrapper::addChild(AttributeWrapper* pWrapper)
{
   if ((pWrapper == NULL) || (mpModel == NULL))
   {
      return;
   }

   QModelIndex parent;
   if (mpParent != NULL)
   {
      const vector<AttributeWrapper*>& siblings = mpParent->getChildren();
      for (vector<AttributeWrapper*>::size_type i = 0; i < siblings.size(); ++i)
      {
         AttributeWrapper* pCurrentWrapper = siblings[i];
         if (pCurrentWrapper == this)
         {
            parent = mpModel->createIndex(static_cast<int>(i), 0, this);
            break;
         }
      }
   }

   int row = static_cast<int>(mChildren.size());
   mpModel->beginInsertRows(parent, row, row);
   mChildren.push_back(pWrapper);
   mpModel->endInsertRows();
}

DynamicObjectItemModel::AttributeWrapper* DynamicObjectItemModel::AttributeWrapper::addChild(const string& name,
                                                                                             DataVariant* pValue)
{
   if ((name.empty() == true) || (pValue == NULL) || (mpModel == NULL))
   {
      return NULL;
   }

   if (getChild(name, pValue) != NULL)
   {
      return NULL;
   }

   AttributeWrapper* pChildWrapper = new AttributeWrapper(mpModel, name, pValue, this);
   if (pChildWrapper != NULL)
   {
      addChild(pChildWrapper);
   }

   return pChildWrapper;
}

DynamicObjectItemModel::AttributeWrapper* DynamicObjectItemModel::AttributeWrapper::getChild(const string& name,
                                                                                             DataVariant* pValue) const
{
   if ((name.empty() == true) || (pValue == NULL))
   {
      return NULL;
   }

   for (vector<AttributeWrapper*>::const_iterator iter = mChildren.begin(); iter != mChildren.end(); ++iter)
   {
      AttributeWrapper* pWrapper = *iter;
      VERIFYRV(pWrapper != NULL, NULL);

      const string& currentName = pWrapper->getName();
      DataVariant* pCurrentValue = pWrapper->getValue();
      if ((currentName == name) && (pCurrentValue != NULL) &&
         ((pCurrentValue == pValue) || (*pCurrentValue == *pValue)))
      {
         return pWrapper;
      }
   }

   return NULL;
}

bool DynamicObjectItemModel::AttributeWrapper::removeChild(AttributeWrapper* pWrapper)
{
   if ((pWrapper == NULL) || (mpModel == NULL))
   {
      return false;
   }

   int row = 0;
   vector<AttributeWrapper*>::reverse_iterator iter;
   for (iter = mChildren.rbegin(), row = static_cast<int>(mChildren.size()) - 1;
        iter != mChildren.rend();
        ++iter, --row)
   {
      AttributeWrapper* pCurrentWrapper = *iter;
      if (pCurrentWrapper == pWrapper)
      {
         // Remove any child items
         vector<AttributeWrapper*> children = pWrapper->getChildren();
         for (vector<AttributeWrapper*>::size_type i = 0; i < children.size(); ++i)
         {
            AttributeWrapper* pChildWrapper = children[i];
            if (pChildWrapper != NULL)
            {
               pWrapper->removeChild(pChildWrapper);
            }
         }

         // Get the parent index
         QModelIndex parent;
         if (mpParent != NULL)
         {
            const vector<AttributeWrapper*>& siblings = mpParent->getChildren();
            for (vector<AttributeWrapper*>::size_type i = 0; i < siblings.size(); ++i)
            {
               AttributeWrapper* pSiblingWrapper = siblings[i];
               if (pSiblingWrapper == this)
               {
                  parent = mpModel->createIndex(static_cast<int>(i), 0, this);
                  break;
               }
            }
         }

         // Remove the item
         mpModel->beginRemoveRows(parent, row, row);
         vector<AttributeWrapper*>::iterator fwdIter = iter.base();
         mChildren.erase(--fwdIter);
         mpModel->endRemoveRows();
         delete pWrapper;
         return true;
      }
   }

   return false;
}

bool DynamicObjectItemModel::AttributeWrapper::removeChild(const string& name, DataVariant* pValue)
{
   if ((name.empty() == true) || (pValue == NULL))
   {
      return false;
   }

   AttributeWrapper* pWrapper = getChild(name, pValue);
   if (pWrapper != NULL)
   {
      return removeChild(pWrapper);
   }

   return false;
}

void DynamicObjectItemModel::AttributeWrapper::clear()
{
   // Delete the child wrappers
   while (mChildren.empty() == false)
   {
      AttributeWrapper* pWrapper = mChildren.back();
      VERIFYNRV(pWrapper != NULL);

      removeChild(pWrapper);
   }
}
