/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESCLASSIFICATION_H
#define PROPERTIESCLASSIFICATION_H

#include "ObjectResource.h"
#include "PropertiesShell.h"

class Classification;
class SessionItem;

class PropertiesClassification : public PropertiesShell
{
public:
   PropertiesClassification();
   virtual ~PropertiesClassification();

   virtual bool initialize(SessionItem* pSessionItem);
   virtual bool applyChanges();

protected:
   virtual QWidget* createWidget();

private:
   FactoryResource<Classification> mpClassification;
};

#endif
