/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef DERIVELAYER_H
#define DERIVELAYER_H

#include "DesktopItems.h"

#include <string>

class RasterElement;

class DeriveAoi : public DesktopItems
{
public:
   DeriveAoi();
   ~DeriveAoi();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   std::string mLayerName;
   std::string mLayerType;
   RasterElement* mpResults;
};

#endif   // DERIVELAYER_H
