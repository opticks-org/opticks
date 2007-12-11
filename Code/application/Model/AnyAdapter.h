/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANYADAPTER_H
#define ANYADAPTER_H

#include "Any.h"
#include "AnyImp.h"

class AnyAdapter : public Any, public AnyImp
{
public:
   AnyAdapter(const std::string& dataType, const DataDescriptorImp& descriptor, const std::string& id);
   ~AnyAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   ANYADAPTER_METHODS(AnyImp)
};

#endif
