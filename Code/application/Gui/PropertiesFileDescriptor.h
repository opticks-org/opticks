/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESFILEDESCRIPTOR_H
#define PROPERTIESFILEDESCRIPTOR_H

#include "PropertiesShell.h"

class SessionItem;

class PropertiesFileDescriptor : public PropertiesShell
{
public:
   PropertiesFileDescriptor();
   ~PropertiesFileDescriptor();

   bool initialize(SessionItem* pSessionItem);
   bool applyChanges();

protected:
   QWidget* createWidget();
};

#endif
