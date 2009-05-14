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
{}

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
         WizardItemImp* pItem = new WizardItemImp(NULL, string(), string());
         if (pItem != NULL)
         {
            pItem->copyItem(pExistItem);
            mItems.push_back(pItem);
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
