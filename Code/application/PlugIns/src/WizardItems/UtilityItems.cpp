/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include "UtilityItems.h"

UtilityItems::UtilityItems()
{
   setShortDescription("Wizard Utility Item");
}

UtilityItems::~UtilityItems()
{
}

bool UtilityItems::setBatch()
{
   mbInteractive = false;
   return true;
}

bool UtilityItems::setInteractive()
{
   mbInteractive = true;
   return true;
}

 
