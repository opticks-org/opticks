/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef DESKTOPITEMS_H
#define DESKTOPITEMS_H

#include "DesktopServices.h"
#include "WizardItems.h"

class DesktopItems : public WizardItems
{
public:
   DesktopItems();
   ~DesktopItems();

   virtual bool setBatch();
   virtual bool setInteractive();

protected:
   Service<DesktopServices> mpDesktop;
};

#endif   // DESKTOPITEMS_H

 
