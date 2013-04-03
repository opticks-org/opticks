/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MOSAICMANAGER_H
#define MOSAICMANAGER_H

#include "AttachmentPtr.h"
#include "ExecutableShell.h"
#include "SpatialDataView.h"

#include <vector>

#include <boost/any.hpp>

class RasterDataDescriptor;
class RasterElement;
class RasterLayer;

class MosaicManager : public ExecutableShell
{
public:
   struct MosaicData
   {
      virtual ~MosaicData() {}

      bool createAnimation;
      std::vector<RasterElement*> mpRasters;
   };

   MosaicManager();
   virtual ~MosaicManager();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   bool geoStitch(MosaicManager::MosaicData* pData, Progress* pProgress);

private:
   MosaicManager(const MosaicManager& rhs);
   MosaicManager& operator=(const MosaicManager& rhs);

   void layerDeleted(Subject& subject, const std::string& signal, const boost::any& value);
   void changeFrame(Subject& subject, const std::string& signalName, const boost::any& data);
   bool createAnimation(bool haveTimes, Progress* pProgress);

   AttachmentPtr<SpatialDataView> mpView;
   MosaicData* mpData;
   std::vector<std::pair<Layer*, double> > mLayers;
   bool mTimeBased;
};

#endif
