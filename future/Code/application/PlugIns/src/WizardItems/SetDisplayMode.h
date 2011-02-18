/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETDISPLAYMODE_H
#define SETDISPLAYMODE_H

#include "DesktopItems.h"
#include "TypesFile.h"

class RasterLayer;

class SetDisplayMode : public DesktopItems
{
public:
   SetDisplayMode();
   ~SetDisplayMode();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   RasterLayer* mpRasterLayer;
   DisplayMode mDisplayMode;
};

#endif
