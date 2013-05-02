/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODISGEOREFERENCE_H
#define MODISGEOREFERENCE_H

#include "AttachmentPtr.h"
#include "DimensionDescriptor.h"
#include "GeoreferenceShell.h"
#include "RasterElement.h"

#include <string>
#include <vector>

class ModisGeoreference : public GeoreferenceShell
{
public:
   ModisGeoreference();
   virtual ~ModisGeoreference();

   virtual unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const;
   virtual bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const;
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   virtual LocationType geoToPixel(LocationType geo, bool* pAccurate) const;
   virtual LocationType pixelToGeo(LocationType pixel, bool* pAccurate) const;

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   void elementDeleted(Subject& subject, const std::string& signal, const boost::any& data);

   DimensionDescriptor getClosestGeocoordinate(const std::vector<DimensionDescriptor>& dims,
      unsigned int originalNumber, bool lessThan) const;

private:
   RasterElement* mpRaster;
   AttachmentPtr<RasterElement> mpLatitude;
   AttachmentPtr<RasterElement> mpLongitude;

   std::vector<double> mXCoefficients;
   std::vector<double> mYCoefficients;
};

#endif
