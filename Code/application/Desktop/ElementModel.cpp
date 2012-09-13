/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataElement.h"
#include "ElementModel.h"
#include "ModelServices.h"
#include "Slot.h"

using namespace std;

ElementModel::ElementModel(QObject* pParent) :
   SessionItemModel(pParent)
{
   // Initialization
   Service<ModelServices> pModel;

   vector<DataElement*> elements = pModel->getElements(NULL, string());
   for (vector<DataElement*>::size_type i = 0; i < elements.size(); ++i)
   {
      DataElement* pElement = elements[i];
      if (pElement != NULL)
      {
         addElementItem(pElement);
      }
   }

   // Connections
   pModel->attach(SIGNAL_NAME(ModelServices, ElementCreated), Slot(this, &ElementModel::addElement));
   pModel->attach(SIGNAL_NAME(ModelServices, ElementDestroyed), Slot(this, &ElementModel::removeElement));
   pModel->attach(SIGNAL_NAME(ModelServices, ElementReparented), Slot(this, &ElementModel::updateElementParent));
}

ElementModel::~ElementModel()
{
   // Detach from model services
   Service<ModelServices> pModel;
   pModel->detach(SIGNAL_NAME(ModelServices, ElementCreated), Slot(this, &ElementModel::addElement));
   pModel->detach(SIGNAL_NAME(ModelServices, ElementDestroyed), Slot(this, &ElementModel::removeElement));
   pModel->detach(SIGNAL_NAME(ModelServices, ElementReparented), Slot(this, &ElementModel::updateElementParent));
}

Qt::ItemFlags ElementModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags itemFlags = SessionItemModel::flags(index);
   if (index.isValid() == true)
   {
      itemFlags |= Qt::ItemIsDragEnabled;
   }
   return itemFlags;
}

void ElementModel::addElement(Subject& subject, const string& signal, const boost::any& value)
{
   DataElement* pElement = boost::any_cast<DataElement*>(value);
   if (pElement != NULL)
   {
      addElementItem(pElement);
   }
}

void ElementModel::removeElement(Subject& subject, const string& signal, const boost::any& value)
{
   DataElement* pElement = boost::any_cast<DataElement*>(value);
   if (pElement != NULL)
   {
      removeElementItem(pElement);
   }
}

void ElementModel::updateElementParent(Subject& subject, const string& signal, const boost::any& value)
{
   DataElement* pElement = boost::any_cast<DataElement*>(value);
   if (pElement != NULL)
   {
      removeElementItem(pElement);
      addElementItem(pElement);
   }
}

void ElementModel::addElementItem(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return;
   }

   // Add the element item
   SessionItemWrapper* pWrapper = getRootWrapper();

   DataElement* pParent = pElement->getParent();
   if (pParent != NULL)
   {
      pWrapper = getWrapper(pParent);
   }

   if (pWrapper != NULL)
   {
      pWrapper->addChild(pElement);
   }

   // Add the children items
   Service<ModelServices> pModel;

   vector<DataElement*> elements = pModel->getElements(pElement, string());
   for (vector<DataElement*>::size_type i = 0; i < elements.size(); ++i)
   {
      DataElement* pChild = elements[i];
      if (pChild != NULL)
      {
         addElementItem(pChild);
      }
   }
}

void ElementModel::removeElementItem(DataElement* pElement)
{
   if (pElement == NULL)
   {
      return;
   }

   // Remove the children items
   Service<ModelServices> pModel;

   vector<DataElement*> elements = pModel->getElements(pElement, string());
   for (vector<DataElement*>::size_type i = 0; i < elements.size(); ++i)
   {
      DataElement* pChild = elements[i];
      if (pChild != NULL)
      {
         removeElementItem(pChild);
      }
   }

   // Remove the element item
   SessionItemWrapper* pWrapper = getWrapper(pElement);
   if (pWrapper != NULL)
   {
      SessionItemWrapper* pParentWrapper = pWrapper->getParent();
      if (pParentWrapper != NULL)
      {
         pParentWrapper->removeChild(pElement);
      }
   }
}
