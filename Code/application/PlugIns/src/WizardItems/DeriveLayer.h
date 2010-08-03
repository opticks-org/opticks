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
#include "TypesFile.h"

class Layer;

class DeriveLayer : public DesktopItems
{
public:
   DeriveLayer();
   virtual ~DeriveLayer();

   virtual bool setBatch();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   Layer* mpInputLayer;
   LayerType mNewLayerType;
   std::string mNewLayerName;
};

#endif   // DERIVELAYER_H
