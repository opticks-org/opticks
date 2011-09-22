/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROPERTIESWAVELENGTHS_H
#define PROPERTIESWAVELENGTHS_H

#include "PropertiesShell.h"
#include "WavelengthsImp.h"

class SessionItem;

class PropertiesWavelengths : public PropertiesShell
{
public:
   PropertiesWavelengths();
   virtual ~PropertiesWavelengths();

   virtual bool initialize(SessionItem* pSessionItem);
   virtual bool applyChanges();

protected:
   QWidget* createWidget();

private:
   WavelengthsImp mWavelengths;
};

#endif
