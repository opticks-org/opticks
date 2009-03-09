/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDOBJECTADAPTER_H
#define WIZARDOBJECTADAPTER_H

#include "WizardObject.h"
#include "WizardObjectImp.h"

class WizardObjectAdapter : public WizardObject, public WizardObjectImp WIZARDOBJECTADAPTEREXTENSION_CLASSES
{
public:
   WizardObjectAdapter();
   ~WizardObjectAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   WIZARDOBJECTADAPTER_METHODS(WizardObjectImp);
};

#endif
