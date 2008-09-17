/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardObjectImp.h"
#include "WizardItemImp.h"
#include "WizardNodeImp.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

WizardObjectImp::WizardObjectImp() :
   mbBatch(false)
{
}

WizardObjectImp::~WizardObjectImp()
{
   clear();
}

void WizardObjectImp::setName(const string& name)
{
   if (name.empty() == false)
   {
      mName = name;
   }
}

const string& WizardObjectImp::getName() const
{
   return mName;
}

WizardItem* WizardObjectImp::addItem(std::string itemName, std::string itemType)
{
   WizardItem* pItem = new WizardItemImp(itemName, itemType);
   if (pItem != NULL)
   {
      mItems.push_back(pItem);
   }

   return pItem;
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

   vector<WizardItem*>::iterator iter;
   for (iter = mItems.begin(); iter != mItems.end(); ++iter)
   {
      WizardItem* pCurrentItem = *iter;
      if (pCurrentItem == pItem)
      {
         mItems.erase(iter);
         delete (static_cast<WizardItemImp*>(pItem));
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
   vector<WizardItem*>::iterator iter;
   iter = mItems.begin();
   while (iter != mItems.end())
   {
      WizardItem* pCurrentItem = NULL;
      pCurrentItem = *iter;

      if ((bFound == true) && (pCurrentItem != NULL))
      {
         bool bConnected = false;
         bConnected = ((WizardItemImp*) pItem)->isItemConnected(pCurrentItem, false);
         if (bConnected == false)
         {
            iter = mItems.erase(iter);
            mItems.insert(inputIter, pCurrentItem);
            return true;
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

   int iCount = 0;
   iCount = mItems.size();
   for (int i = iCount - 1; i >= 0; i--)
   {
      WizardItem* pCurrentItem = NULL;
      pCurrentItem = mItems.at(i);

      if ((bFound == true) && (pCurrentItem != NULL))
      {
         bool bConnected = false;
         bConnected = ((WizardItemImp*) pItem)->isItemConnected(pCurrentItem, true);
         if (bConnected == false)
         {
            pRemoveItem = pCurrentItem;
            break;
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
         return true;
      }
   }

   return false;
}

void WizardObjectImp::setBatch(bool bBatch)
{
   mbBatch = bBatch;
}

bool WizardObjectImp::isBatch() const
{
   return mbBatch;
}

void WizardObjectImp::setMenuLocation(const string& location)
{
   mMenuLocation = location;
}

const string& WizardObjectImp::getMenuLocation() const
{
   return mMenuLocation;
}

bool WizardObjectImp::toXml(XMLWriter* xml) const
{
   xml->addAttr("version", XmlBase::VERSION);
   xml->addAttr("name", mName);
   xml->addAttr("type", "WizardObject");
   xml->addAttr("batch", StringUtilities::toXmlString(mbBatch));
   xml->addAttr("menuLocation", mMenuLocation);

   vector<WizardItem*>::const_iterator iiter;
   for(iiter = mItems.begin(); iiter != mItems.end(); iiter++)
   {
      WizardItem* pItem(*iiter);
      if(pItem == NULL)
         continue;
      xml->pushAddPoint(xml->addElement("item"));
      bool rval(pItem->toXml(xml));
      xml->popAddPoint();
      if(!rval)
         return false;
   }

   vector<WizardConnection> connections = WizardItemImp::getConnections(mItems);
   vector<WizardConnection>::iterator citer;
   for(citer = connections.begin(); citer != connections.end(); citer++)
   {
      WizardConnection connection(*citer);
      xml->pushAddPoint(xml->addElement("connection"));

      xml->addAttr("inputItem", StringUtilities::toXmlString(connection.miInputItemIndex));
      xml->addAttr("inputNode", StringUtilities::toXmlString(connection.miInputNodeIndex));
      xml->addAttr("outputItem", StringUtilities::toXmlString(connection.miOutputItemIndex));
      xml->addAttr("outputNode", StringUtilities::toXmlString(connection.miOutputNodeIndex));
      xml->popAddPoint();
   }

   return true;
}

bool WizardObjectImp::fromXml(DOMNode* document, unsigned int version)
{
   DOMElement *elmnt(static_cast<DOMElement*>(document));
   mName = A(elmnt->getAttribute(X("name")));
   string v(A(elmnt->getAttribute(X("batch"))));
   mbBatch = StringUtilities::fromXmlString<bool>(v);
   mMenuLocation = A(elmnt->getAttribute(X("menuLocation")));

   mItems.clear();
   vector<WizardConnection> connections;
   for(DOMNode *chld = document->getFirstChild();
                chld != NULL;
                chld = chld->getNextSibling())
   {
      if(XMLString::equals(chld->getNodeName(), X("item")))
      {
         WizardItem *pItem(new WizardItemImp("", ""));
         if(pItem->fromXml(chld, version))
         {
            mItems.push_back(pItem);
         }
         else
         {
            delete dynamic_cast<WizardItemImp*>(pItem);
            return false;
         }
      }
      else if(XMLString::equals(chld->getNodeName(), X("connection")))
      {
         DOMElement *e(static_cast<DOMElement*>(chld));
         WizardConnection connection;

         connection.miInputItemIndex  = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("inputItem"))));
         connection.miInputNodeIndex  = StringUtilities::fromXmlString<int>(A(e->getAttribute(X("inputNode"))));
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
   static string type("WizardObjectImp");
   return type;
}

bool WizardObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WizardObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
