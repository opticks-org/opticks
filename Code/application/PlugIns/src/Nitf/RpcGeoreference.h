/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RPCGEOREFERENCE_H
#define RPCGEOREFERENCE_H

#include "GeoreferenceShell.h"
#include "NitfChipConverter.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"
#include <ossim/projection/ossimRpcModel.h>

#define NUM_RPC_COEFFICIENTS 20 

class RasterElement;
class DynamicObject;
class PlugInArgList;

namespace Nitf
{
   class RpcGeoreference : public GeoreferenceShell
   {
   public:
       RpcGeoreference();
       ~RpcGeoreference();

       bool execute(PlugInArgList* pInParam, PlugInArgList* pOutParam);
       bool getInputSpecification(PlugInArgList*& pArgList);
       bool getOutputSpecification(PlugInArgList*& pArgList);

      LocationType pixelToGeo(LocationType pixel) const;
      LocationType geoToPixel(LocationType geo) const;
      bool canHandleRasterElement(RasterElement* pRaster) const;
      
      bool hasAbort();

      bool serialize(SessionItemSerializer &serializer) const;
      bool deserialize(SessionItemDeserializer &deserializer);

      QWidget* getGui(RasterElement *pRaster);

   private:
      const DynamicObject* getRpcInstance(RasterElement *pRaster) const;

      RasterElement *mpRaster;
      mutable std::string mRpcVersion;
      
      std::auto_ptr<Nitf::ChipConverter> mpChipConverter;

      ossimRpcModel mModel;
   };
}

#endif
