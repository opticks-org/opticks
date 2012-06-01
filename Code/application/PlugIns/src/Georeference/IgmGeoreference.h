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

#include "AttachmentPtr.h"
#include "GeoreferenceShell.h"
#include "GeoreferenceUtilities.h"
#include "ProgressTracker.h"
#include "RasterElement.h"

#include <string>
#include <vector>

class IgmGui;
class RasterDataDescriptor;

#define USE_EXISTING_ELEMENT "IGM Georeference/UseExistingElement"
#define IGM_FILENAME "IGM Georeference/IgmFilename"

class IgmGeoreference : public GeoreferenceShell
{
public:
   IgmGeoreference();
   virtual ~IgmGeoreference();

   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

   virtual unsigned char getGeoreferenceAffinity(const RasterDataDescriptor* pDescriptor) const;
   virtual QWidget* getWidget(RasterDataDescriptor* pDescriptor);
   virtual bool validate(const RasterDataDescriptor* pDescriptor, std::string& errorMessage) const;
   virtual LocationType geoToPixel(LocationType geo, bool* pAccurate) const;
   virtual LocationType pixelToGeo(LocationType pixel, bool* pAccurate) const;

   void elementDeleted(Subject& subject, const std::string& signal, const boost::any& data);

   virtual bool serialize(SessionItemSerializer& serializer) const;
   virtual bool deserialize(SessionItemDeserializer& deserializer);

protected:
   bool loadIgmFile(const std::string& igmFilename);

private:
   IgmGeoreference(const IgmGeoreference& rhs);
   IgmGeoreference& operator=(const IgmGeoreference& rhs);
   IgmGui* mpGui;

   ProgressTracker mProgress;
   RasterElement* mpRaster;
   AttachmentPtr<RasterElement> mpIgmRaster;
   unsigned int mNumRows;
   unsigned int mNumColumns;
   const RasterDataDescriptor* mpIgmDesc;
   unsigned int mZone;
   std::vector<double> mLatCoefficients;
   std::vector<double> mLonCoefficients;
};

#endif
