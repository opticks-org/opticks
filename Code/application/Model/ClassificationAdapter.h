/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONADAPTER_H
#define CLASSIFICATIONADAPTER_H

#include "Classification.h"
#include "ClassificationImp.h"

class ClassificationAdapter  : public Classification, public ClassificationImp CLASSIFICATIONADAPTEREXTENSION_CLASSES
{
public:
   ClassificationAdapter();
   ~ClassificationAdapter();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CLASSIFICATIONADAPTER_METHODS(ClassificationImp)
};

#endif
