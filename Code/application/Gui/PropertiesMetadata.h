/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESMETADATA_H
#define PROPERTIESMETADATA_H

#include "DynamicObjectAdapter.h"
#include "PropertiesShell.h"

class SessionItem;

class PropertiesMetadata : public PropertiesShell
{
public:
   PropertiesMetadata();
   virtual ~PropertiesMetadata();

   virtual bool initialize(SessionItem* pSessionItem);
   virtual bool applyChanges();

protected:
   QWidget* createWidget();

private:
   DynamicObjectAdapter mMetadata;
};

#endif
