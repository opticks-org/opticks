/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef _CLASSIFICATIONADAPTER
#define _CLASSIFICATIONADAPTER

#include "Classification.h"
#include "ClassificationImp.h"

class ClassificationAdapter  : public Classification, public ClassificationImp
{
public:
   ClassificationAdapter() {}
   ~ClassificationAdapter() {}

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   CLASSIFICATIONADAPTER_METHODS(ClassificationImp)
};

#endif
