/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FLATTENANNOTATIONLAYER_H
#define FLATTENANNOTATIONLAYER_H

#include "AlgorithmShell.h"
#include "ComplexData.h"
#include "DataAccessor.h"
#include "DimensionDescriptor.h"
#include <QtGui/QColor>

class FlattenAnnotationLayer : public AlgorithmShell
{
public:
   FlattenAnnotationLayer();
   virtual ~FlattenAnnotationLayer();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   struct ChannelInfo
   {
      ChannelInfo() : bandShift(0), acc(NULL, NULL), dataMin(0.0), dataMax(0.0), numRows(0), numCols(0) {}
      ChannelInfo(const ChannelInfo& other) : band(other.band), bandShift(other.bandShift),
               channel(other.channel), acc(other.acc), dataMin(other.dataMin), dataMax(other.dataMax),
               numRows(other.numRows), numCols(other.numCols) {}

      DimensionDescriptor band;
      int bandShift;
      RasterChannelType channel;
      DataAccessor acc;
      double dataMin;
      double dataMax;
      unsigned int numRows;
      unsigned int numCols;
   };

   template<typename T> void flattenData(T* pRasterData, ChannelInfo& channel, QRgb imageDatum);
};
template<> void FlattenAnnotationLayer::flattenData<IntegerComplex>(
      IntegerComplex* pRasterData, ChannelInfo& channel, QRgb imageDatum);
template<> void FlattenAnnotationLayer::flattenData<FloatComplex>(
      FloatComplex* pRasterData, ChannelInfo& channel, QRgb imageDatum);

#endif
