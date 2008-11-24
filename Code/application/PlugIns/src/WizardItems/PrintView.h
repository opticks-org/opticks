/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PRINTVIEW_H
#define PRINTVIEW_H

#include "DesktopItems.h"

class RasterElement;

class PrintView : public DesktopItems
{
public:
   PrintView();
   ~PrintView();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   RasterElement* mpRasterElement;
   bool mbPrintDialog;
};

#endif
