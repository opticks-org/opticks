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

#include <ossim/projection/ossimRpcModel.h>
#include <memory>

#define NUM_RPC_COEFFICIENTS 20

class DynamicObject;
class PlugInArgList;
class RasterElement;
class RpcGui;

namespace Nitf
{
   class RpcGeoreference : public GeoreferenceShell
   {
   public:
      RpcGeoreference();
      virtual ~RpcGeoreference();

      bool getInputSpecification(PlugInArgList*& pArgList);
      bool execute(PlugInArgList* pInParam, PlugInArgList* pOutParam);

      unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const;
      QWidget* getWidget(RasterDataDescriptor* pDescriptor);
      bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const;
      LocationType pixelToGeo(LocationType pixel, bool* pAccurate = NULL) const;
      LocationType geoToPixel(LocationType geo, bool* pAccurate = NULL) const;

      bool serialize(SessionItemSerializer &serializer) const;
      bool deserialize(SessionItemDeserializer &deserializer);

   private:
      const DynamicObject* getRpcInstance(const RasterDataDescriptor* pDescriptor) const;

      RasterElement* mpRaster;
      mutable std::string mRpcVersion;

      std::auto_ptr<Nitf::ChipConverter> mpChipConverter;

      ossimRpcModel mModel;
      double mHeight;
      RpcGui* mpGui;
   };
}

#endif
