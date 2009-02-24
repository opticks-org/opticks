/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESDATADESCRIPTOR_H
#define PROPERTIESDATADESCRIPTOR_H

#include "PropertiesShell.h"

class SessionItem;

class PropertiesDataDescriptor : public PropertiesShell
{
public:
   PropertiesDataDescriptor();
   ~PropertiesDataDescriptor();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

protected:
   QWidget* createWidget();
};

#endif
