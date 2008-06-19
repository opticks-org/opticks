/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESFEATURECLASS_H
#define PROPERTIESFEATURECLASS_H

#include "PropertiesShell.h"

class FeatureClass;
class SessionItem;

class PropertiesFeatureClass : public PropertiesShell
{
public:
   PropertiesFeatureClass();
   ~PropertiesFeatureClass();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

protected:
   QWidget* createWidget();

private:
   FeatureClass* mpFeatureClass;
};

#endif
