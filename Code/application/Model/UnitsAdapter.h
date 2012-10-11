/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UNITSADAPTER_H
#define UNITSADAPTER_H

#include "Units.h"
#include "UnitsImp.h"

class UnitsAdapter : public Units, public UnitsImp UNITSADAPTEREXTENSION_CLASSES
{
public:
   UnitsAdapter();
   virtual ~UnitsAdapter();

   // TypeAwareObject
   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   UNITSADAPTER_METHODS(UnitsImp)

private:
   UnitsAdapter(const UnitsAdapter& rhs);
   UnitsAdapter& operator=(const UnitsAdapter& rhs);
};

#endif
