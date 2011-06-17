/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IGMGEOREFERENCE_H
#define IGMGEOREFERENCE_H


#include "ApplicationServices.h"
#include "AttachmentPtr.h"
#include "DimensionDescriptor.h"
#include "GeoreferenceShell.h"
#include "ModelServices.h"
#include "PlugInResource.h"
#include "UtilityServices.h"

class IgmGui;

#include <vector>

class IgmGeoreference : public GeoreferenceShell
{
public:
   IgmGeoreference();
   virtual ~IgmGeoreference();

   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool setInteractive();

   virtual LocationType geoToPixel(LocationType geo, bool* pAccurate) const;

   virtual LocationType pixelToGeo(LocationType pixel, bool* pAccurate) const;
   virtual LocationType pixelToGeoQuick(LocationType pixel, bool* pAccurate) const;
   virtual bool canHandleRasterElement(RasterElement* pRaster) const;
   virtual QWidget* getGui(RasterElement* pRaster);
   virtual bool validateGuiInput() const;
   void elementDeleted(Subject& subject, const std::string& signal, const boost::any& data);

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

private:
   IgmGui* mpGui;

   RasterElement* mpRaster;
   Georeference* mpGcpPlugIn;
   AttachmentPtr<DataElement> mpIgmGeo;
   AttachmentPtr<RasterElement> mpIgmRaster;
   std::vector<DimensionDescriptor> mLoadedColumns;
   std::vector<DimensionDescriptor> mLoadedRows;
   std::vector<DimensionDescriptor> mLoadedBands;
   unsigned int mZone;
};

#endif // IGMGEOREFERENCE_H
