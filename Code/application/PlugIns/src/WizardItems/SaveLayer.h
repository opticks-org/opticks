/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAVELAYER_H
#define SAVELAYER_H

#include "LayerItems.h"

#include <string>

class DataElement;
class Filename;
class RasterElement;

////////////////////////////////////////////////////////////
class SaveLayer : public LayerItems
{
public:
   SaveLayer();
   ~SaveLayer();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);

private:
   Filename* mpOutputFilename;
   DataElement* mpElement;
};

////////////////////////////////////////////////////////////
class SaveAoi : public SaveLayer
{
public:
   SaveAoi();
   ~SaveAoi();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class SaveGcpList : public SaveLayer
{
public:
   SaveGcpList();
   ~SaveGcpList();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class SaveLayerFromDataSet : public LayerItems
{
public:
   SaveLayerFromDataSet();
   ~SaveLayerFromDataSet();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);

private:
   Filename* mpOutputFilename;
   RasterElement* mpRasterElement;
   std::string mLayerName;
};

////////////////////////////////////////////////////////////
class SaveAoiFromDataSet : public SaveLayerFromDataSet
{
public:
   SaveAoiFromDataSet();
   ~SaveAoiFromDataSet();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class SaveAnnotation : public SaveLayerFromDataSet
{
public:
   SaveAnnotation();
   ~SaveAnnotation();

protected:
   LayerType getLayerType() const;
};

////////////////////////////////////////////////////////////
class SaveGcpListFromDataSet : public SaveLayerFromDataSet
{
public:
   SaveGcpListFromDataSet();
   ~SaveGcpListFromDataSet();

protected:
   LayerType getLayerType() const;
};

#endif
