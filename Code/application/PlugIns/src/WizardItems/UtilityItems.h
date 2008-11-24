/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef UTILITYITEMS_H
#define UTILITYITEMS_H

#include "WizardItems.h"

class UtilityItems : public WizardItems
{
public:
   UtilityItems();
   ~UtilityItems();

   virtual bool setBatch();
   virtual bool setInteractive();
};

#endif   // UTILITYITEMS_H

 
