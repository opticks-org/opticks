/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef AOIELEMENTADAPTER_H
#define AOIELEMENTADAPTER_H

#include "AoiElement.h"
#include "AoiElementImp.h"

class AoiElementAdapter : public AoiElement, public AoiElementImp AOIELEMENTADAPTEREXTENSION_CLASSES
{
public:
   AoiElementAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~AoiElementAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   AOIELEMENTADAPTER_METHODS(AoiElementImp)

private:
   AoiElementAdapter(const AoiElementAdapter& rhs);
   AoiElementAdapter& operator=(const AoiElementAdapter& rhs);
};

#endif
