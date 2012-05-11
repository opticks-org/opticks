/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef WIZARDCLIPBOARD_H
#define WIZARDCLIPBOARD_H

#include "WizardItem.h"

#include <vector>

class WizardClipboard
{
public:
   static WizardClipboard* instance();

   void setItems(const std::vector<WizardItem*>& items);
   const std::vector<WizardItem*>& getItems() const;
   void clearItems();

protected:
   WizardClipboard();
   ~WizardClipboard();

private:
   static WizardClipboard* mpInstance;
   std::vector<WizardItem*> mItems;
};

#endif   // WIZARDCLIPBOARD_H

 
