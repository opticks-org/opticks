/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETVIEWDISPLAYAREA_H
#define SETVIEWDISPLAYAREA_H

#include "DesktopItems.h"
#include "LocationType.h"

class PerspectiveView;
class RasterElement;

class SetViewDisplayArea : public DesktopItems
{
public:
   SetViewDisplayArea();
   virtual ~SetViewDisplayArea();

   virtual bool setBatch();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);

private:
   PerspectiveView* mpView;
   RasterElement* mpRaster;
   LocationType mPixelCenter;
   double mZoom;
};

#endif
