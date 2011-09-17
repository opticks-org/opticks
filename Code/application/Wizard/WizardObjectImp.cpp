/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataVariant.h"
#include "WizardObject.h"
#include "WizardObjectImp.h"
#include "WizardItemImp.h"
#include "WizardNodeImp.h"

#include <memory>
using namespace std;
XERCES_CPP_NAMESPACE_USE

WizardObjectImp::WizardObjectImp() :
   mbBatch(false)
{}

WizardObjectImp::~WizardObjectImp()
{
   clear();
}

void WizardObjectImp::setName(const string& name)
{
   if ((name.empty() == false) && (name != mName))
   {
      mName = name;
      notify(SIGNAL_NAME(WizardObjectImp, Renamed), boost::any(mName));
   }
}

const string& WizardObjectImp::getName() const
{
   return mName;
}

WizardItem* WizardObjectImp::addPlugInItem(const string& itemName, const string& itemType)
{
   if ((itemName.empty() == true) || (itemType.empty() == true))
   {
      return NULL;
   }

   WizardItemImp* pItem = new WizardItemImp(dynamic_cast<WizardObject*>(this), itemName, itemType);
   pItem->setBatchMode(mbBatch);
   addItem(pItem);

   return pItem;
}

WizardItem* WizardObjectImp::addValueItem(const string& itemName, const DataVariant& value)
{
   if (itemName.empty() == true)
   {
      return NULL;
   }

   WizardItemImp* pItem = new WizardItemImp(dynamic_cast<WizardObject*>(this), itemName, value);
   // default to batch otherwise user will be prompted to input every
   // value item every time the wizard is executed
   pItem->setBatchMode(true);
   addItem(pItem);

   return pItem;
}

void WizardObjectImp::addItem(WizardItem* pItem)
{
   WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
   if (pItemImp == NULL)
   {
      return;
   }

   if (find(mItems.begin(), mItems.end(), pItem) != mItems.end())
   {
      return;
   }

   // Ensure that an interactive item is not added to a batch wizard
   if ((mbBatch == true) && (pItemImp->getBatchMode() == false))
   {
      pItemImp->setBatchMode(true);
   }

   // Add the item
   mItems.push_back(pItem);
   pItemImp->attach(SIGNAL_NAME(WizardItemImp, ItemConnected), Slot(this, &WizardObjectImp::itemConnected));

   // Notify connected objects
   notify(SIGNAL_NAME(WizardObjectImp, ItemAdded), boost::any(pItem));
}

const vector<WizardItem*>& WizardObjectImp::getItems() const
{
   return mItems;
}

bool WizardObjectImp::removeItem(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return false;
   }

   for (vector<WizardItem*>::iterator iter = mItems.begin(); iter != mItems.end(); ++iter)
   {
      WizardItem* pCurrentItem = *iter;
      if (pCurrentItem == pItem)
      {
         static_cast<WizardItemImp*>(pItem)->detach(SIGNAL_NAME(WizardItemImp, ItemConnected),
            Slot(this, &WizardObjectImp::itemConnected));
         mItems.erase(iter);
         notify(SIGNAL_NAME(WizardObjectImp, ItemRemoved), boost::any(pItem));
         delete static_cast<WizardItemImp*>(pItem);
         return true;
      }
   }

   return false;
}

void WizardObjectImp::clear()
{
   while (mItems.empty() == false)
   {
      WizardItem* pItem = mItems.front();
      if (pItem != NULL)
      {
         bool bSuccess = removeItem(pItem);
         if (bSuccess == false)
         {
            break;
         }
      }
   }
}

bool WizardObjectImp::increaseItemOrder(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return false;
   }

   bool bFound = false;

   vector<WizardItem*>::iterator inputIter;
   vector<WizardItem*>::iterator iter = mItems.begin();
   while (iter != mItems.end())
   {
      WizardItem* pCurrentItem = *iter;
      if ((bFound == true) && (pCurrentItem != NULL))
      {
         WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
         if (pItemImp != NULL)
         {
            bool bConnected = pItemImp->isItemConnected(pCurrentItem, false);
            if (bConnected == false)
            {
               iter = mItems.erase(iter);
               mItems.insert(inputIter, pCurrentItem);
               notify(SIGNAL_NAME(WizardObjectImp, ExecutionOrderChanged));
               return true;
            }
         }
      }
      else if (pCurrentItem == pItem)
      {
         inputIter = iter;
         bFound = true;
      }

      iter++;
   }

   return false;
}

bool WizardObjectImp::decreaseItemOrder(WizardItem* pItem)
{
   if (pItem == NULL)
   {
      return false;
   }

   bool bFound = false;
   WizardItem* pRemoveItem = NULL;

   int iCount = mItems.size();
   for (int i = iCount - 1; i >= 0; i--)
   {
      WizardItem* pCurrentItem = mItems.at(i);
      if ((bFound == true) && (pCurrentItem != NULL))
      {
         WizardItemImp* pItemImp = static_cast<WizardItemImp*>(pItem);
         if (pItemImp != NULL)
         {
            bool bConnected = pItemImp->isItemConnected(pCurrentItem, true);
            if (bConnected == false)
            {
               pRemoveItem = pCurrentItem;
               break;
            }
         }
      }
      else if (pCurrentItem == pItem)
      {
         bFound = true;
      }
   }

   if (pRemoveItem == NULL)
   {
      return false;
   }

   vector<WizardItem*>::iterator iter = mItems.begin();
   while (iter != mItems.end())
   {
      WizardItem* pCurrentItem = *iter;
      if (pCurrentItem == pRemoveItem)
      {
         iter = mItems.erase(iter);
      }
      else
      {
         ++iter;
      }

      if (pCurrentItem == pItem)
      {
         mItems.insert(iter, pRemoveItem);
         notify(SIGNAL_NAME(WizardObjectImp, ExecutionOrderChanged));
         return true;
      }
   }

   return false;
}

int WizardObjectImp::getExecutionOrder(WizardItem* pItem) const
{
   if (pItem == NULL)
   {
      return -1;
   }

   for (vector<WizardItem*>::size_type i = 0; i < mItems.size(); ++i)
   {
      WizardItem* pCurrentItem = mItems[i];
      if (pCurrentItem == pItem)
      {
         return static_cast<int>(i + 1);
      }
   }

   return -1;
}

void WizardObjectImp::setBatch(bool bBatch)
{
   if (bBatch != mbBatch)
   {
      mbBatch = bBatch;
      if (mbBatch == true)
      {
         // Force all wizard items to be in batch mode
         for (vector<WizardItem*>::iterator iter = mItems.begin(); iter != mItems.end(); ++iter)
         {
            WizardItemImp* pItem = static_cast<WizardItemImp*>(*iter);
            if (pItem != NULL)
            {
               pItem->setBatchMode(true);
            }
         }
      }

      notify(SIGNAL_NAME(WizardObjectImp, BatchModeChanged), boost::any(mbBatch));
   }
}

bool WizardObjectImp::isBatch() const
{
   return mbBatch;
}

void WizardObjectImp::setMenuLocation(const string& location)
{
   if (location != mMenuLocation)
   {
      mMenuLocation = location;
      notify(SIGNAL_NAME(WizardObjectImp, MenuLocationChanged), boost::any(mMenuLocation));
   }
}

const string& WizardObjectImp::getMenuLocation() const
{
   return mMenuLocation;
}

bool WizardObjectImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("version", XmlBase::VERSION);
   pXml->addAttr("name", mName);
   pXml->addAttr("type", "WizardObject");
   pXml->addAttr("batch", StringUtilities::toXmlString(mbBatch));
   pXml->addAttr("menuLocation", mMenuLocation);

   for (vector<WizardItem*>::const_iterator iiter = mItems.begin(); iiter != mItems.end(); ++iiter)
   {
      WizardItem* pItem(*iiter);
      if (pItem == NULL)
      {
         continue;
      }
      pXml->pushAddPoint(pXml->addElement("item"));
      bool rval(pItem->toXml(pXml));
      pXml->popAddPoint();
      if (!rval)
      {
         return false;
      }
   }

   vector<WizardConnection> connections = WizardItemImp::getConnections(mItems);
   for (vector<WizardConnection>::iterator citer = connections.begin(); citer != connections.end(); ++citer)
   {
      WizardConnection connection(*citer);
      pXml->pushAddPoint(pXml->addElement("connection"));

      pXml->addAttr("inputItem", StringUtilities::toXmlString(connection.miInputItemIndex));
      pXml->addAttr("inputNode", StringUtilities::toXmlString(connection.miInputNodeIndex));
      pXml->addAttr("outputItem", StringUtilities::toXmlString(connection.miOutputItemIndex));
      pXml->addAttr("outputNode", StringUtilities::toXmlString(connection.miOutputNodeIndex));
      pXml->popAddPoint();
   }

   return true;
}

bool WizardObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   DOMElement* elmnt(static_cast<DOMElement*>(pDocument));
   mName = A(elmnt->getAttribute(X("name")));
   string v(A(elmnt->getAttribute(X("batch"))));
   mbBatch = StringUtilities::fromXmlString<bool>(v);
   mMenuLocation = A(elmnt->getAttribute(X("menuLocation")));

   clear();
   vector<WizardConnection> connections;
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("item")))
      {
         std::auto_ptr<WizardItemImp> pItem(new WizardItemImp(dynamic_cast<WizardObject*>(this), string(), string()));
         if (pItem->fromXml(pChld, version))
         {
            pItem->attach(SIGNAL_NAME(WizardItemImp, ItemConnected), Slot(this, &WizardObjectImp::itemConnected));
            mItems.push_back(pItem.release());
         }
         else
         {
            return false;
         }
      }
      else if (XMLString::equals(pChld->getNodeName(), X("connection")))
      {
         DOMElement* e(static_cast<DOMElement*>(pChld));
         WizardConnection connection;

         connection.miInputItemIndex = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("inputItem"))));
         connection.miInputNodeIndex = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("inputNode"))));
         connection.miOutputItemIndex = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("outputItem"))));
         connection.miOutputNodeIndex = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("outputNode"))));

         connections.push_back(connection);
      }
   }

   WizardItemImp::setConnections(mItems, connections);
   notify(SIGNAL_NAME(Subject, Modified));
   return true;
}

const string& WizardObjectImp::getObjectType() const
{
   static string sType("WizardObjectImp");
   return sType;
}

bool WizardObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WizardObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void WizardObjectImp::itemConnected(Subject& subject, const string& signal, const boost::any& data)
{
   WizardItem* pItem = dynamic_cast<WizardItem*>(&subject);
   VERIFYNRV(pItem != NULL);

   WizardItem* pConnectedItem = boost::any_cast<WizardItem*>(data);
   VERIFYNRV(pConnectedItem != NULL);

   WizardItem* pInputItem = NULL;
   WizardItem* pOutputItem = NULL;

   if (pItem->isItemConnected(pConnectedItem, true) == true)
   {
      pInputItem = pItem;
      pOutputItem = pConnectedItem;
   }
   else
   {
      pInputItem = pConnectedItem;
      pOutputItem = pItem;
   }

   if ((pInputItem == NULL) || (pOutputItem == NULL))
   {
      return;
   }

   int outputOrder = getExecutionOrder(pOutputItem);
   int inputOrder = getExecutionOrder(pInputItem);

   while (outputOrder > inputOrder)
   {
      if (decreaseItemOrder(pOutputItem) == false)
      {
         break;
      }

      outputOrder = getExecutionOrder(pOutputItem);
      inputOrder = getExecutionOrder(pInputItem);
   }
}
