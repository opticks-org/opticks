/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include "WizardClipboard.h"
#include "WizardItemImp.h"

#include <string>
using namespace std;

WizardClipboard* WizardClipboard::mpInstance = NULL;

WizardClipboard* WizardClipboard::instance()
{
   if (mpInstance == NULL)
   {
      mpInstance = new WizardClipboard();
   }

   return mpInstance;
}

WizardClipboard::WizardClipboard()
{
}

WizardClipboard::~WizardClipboard()
{
   clearItems();
}

void WizardClipboard::setItems(const vector<WizardItem*>& items)
{
   clearItems();
   vector<WizardConnection> connections = WizardItemImp::getConnections(items);

   int iItems = items.size();
   for (int i = 0; i < iItems; i++)
   {
      WizardItemImp* pExistItem = static_cast<WizardItemImp*>(items.at(i));
      if (pExistItem != NULL)
      {
         const string& itemName = pExistItem->getName();
         const string& itemType = pExistItem->getType();

         WizardItemImp* pItem = new WizardItemImp(itemName, itemType);
         if (pItem != NULL)
         {
            *pItem = *pExistItem;
            mItems.push_back(static_cast<WizardItem*>(pItem));
         }
      }
   }

   WizardItemImp::setConnections(mItems, connections);
}

const vector<WizardItem*>& WizardClipboard::getItems() const
{
   return mItems;
}

void WizardClipboard::clearItems()
{
   int iItems = mItems.size();
   for (int i = 0; i < iItems; i++)
   {
      WizardItemImp* pItem = static_cast<WizardItemImp*>(mItems.at(i));
      if (pItem != NULL)
      {
         delete pItem;
      }
   }

   mItems.clear();
}
