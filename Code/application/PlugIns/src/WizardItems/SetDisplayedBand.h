/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETDISPLAYEDBAND_H
#define SETDISPLAYEDBAND_H

#include "DesktopItems.h"
#include "TypesFile.h"

class RasterElement;
class RasterLayer;

class SetDisplayedBand : public DesktopItems
{
public:
   SetDisplayedBand();
   ~SetDisplayedBand();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   RasterLayer* mpRasterLayer;
   RasterChannelType mRasterChannelType;
   RasterElement* mpRasterElement;
   unsigned int mOriginalNumber;
};

#endif
