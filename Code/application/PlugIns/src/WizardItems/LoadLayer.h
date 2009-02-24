/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOADLAYER_H
#define LOADLAYER_H

#include "LayerItems.h"

class Filename;
class SpatialDataView;

////////////////////////////////////////////////////////////
class LoadLayer : public LayerItems
{
public:
   LoadLayer();
   ~LoadLayer();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   Filename* mpFilename;
   SpatialDataView* mpView;
};

////////////////////////////////////////////////////////////
class LoadAoi : public LoadLayer
{
public:
   LoadAoi();
   ~LoadAoi();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class LoadAnnotation : public LoadLayer
{
public:
   LoadAnnotation();
   ~LoadAnnotation();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class LoadGcpList : public LoadLayer
{
public:
   LoadGcpList();
   ~LoadGcpList();

protected:
   LayerType getLayerType() const;
};

#endif
