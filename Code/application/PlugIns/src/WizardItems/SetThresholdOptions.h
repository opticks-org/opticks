/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETTHRESHOLDOPTIONS_H
#define SETTHRESHOLDOPTIONS_H

#include "ColorType.h"
#include "DesktopItems.h"
#include "TypesFile.h"

class ThresholdLayer;

class SetThresholdOptions : public DesktopItems
{
public:
   SetThresholdOptions();
   virtual ~SetThresholdOptions();

   virtual bool setBatch();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);

private:
   ThresholdLayer* mpLayer;
   double mFirstThreshold;
   double mSecondThreshold;
   bool mHasFirst;
   bool mHasSecond;
   PassArea mPassArea;
   RegionUnits mRegionUnits;
   SymbolType mSymbol;
   ColorType mColor;
};

#endif
