/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "setArg.h"

//
// Construct arg lists and run bands algorithm.
//
void setArg(PlugInManagerServices* pPlugInManager,
            PlugInArgList* pArgList,
            const char* name,
            const char* type,
            const void* defaultValue,
            const void* actualValue)
{
   PlugInArg *p1 = pPlugInManager->getPlugInArg();
   p1->setName(name);
   p1->setType(type);
   if (actualValue)
      p1->setActualValue(actualValue);
   p1->setDefaultValue(defaultValue);
   pArgList->addArg(*p1);
}
