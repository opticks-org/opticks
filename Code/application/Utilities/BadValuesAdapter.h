/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BADVALUESADAPTER_H
#define BADVALUESADAPTER_H

#include "BadValues.h"
#include "BadValuesImp.h"

class BadValuesAdapter : public BadValues, public BadValuesImp BADVALUESADAPTEREXTENSION_CLASSES
{
public:
   BadValuesAdapter();
   virtual ~BadValuesAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   BADVALUESADAPTER_METHODS(BadValuesImp)
};

#endif
