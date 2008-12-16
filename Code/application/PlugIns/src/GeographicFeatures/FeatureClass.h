/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FEATURECLASS_H
#define FEATURECLASS_H

#include "AnyData.h"
#include "ConnectionParameters.h"
#include "EnumWrapper.h"
#include "FeatureClassProperties.h"
#include "ObjectResource.h"
#include "QueryOptions.h"
#include "LocationType.h"

#include <QtCore/QObject>

#include <vector>
#include <utility>

class FeatureProxyConnector;
class GraphicElement;
class GraphicGroup;
class Progress;

class FeatureClass : public QObject, public AnyData
{
   Q_OBJECT

public:
   FeatureClass();
   ~FeatureClass();

   AnyData* copy() const;

   bool setParentElement(GraphicElement* pParentElement);

   bool open(std::string& errorMessage);
   bool close(std::string& errorMessage);

   bool hasLabels() const;

   bool setConnectionParameters(const ArcProxyLib::ConnectionParameters& connection);
   const ArcProxyLib::ConnectionParameters& getConnectionParameters() const;

   bool clearQueries();
   bool addQuery(const QueryOptions &query);
   const std::vector<QueryOptions> &getQueries() const;

   enum ClippingTypeEnum
   {
      NO_CLIP,
      SCENE_CLIP,
      SPECIFIED_CLIP
   };

   /**
    * @EnumWrapper FeatureClass::ClippingTypeEnum.
    */
   typedef EnumWrapper<ClippingTypeEnum> ClippingType;

   void setClippingType(ClippingType clipping);
   ClippingType getClippingType() const;
   void setClipping(LocationType ll, LocationType ur);
   std::pair<LocationType, LocationType> getClipping() const;

   void setLayerName(const std::string &layerName);
   const std::string &getLayerName() const;

   const ArcProxyLib::FeatureClassProperties &getFeatureClassProperties() const;

   bool update(Progress *pProgress, std::string &errorMessage);

   FactoryResource<DynamicObject> toDynamicObject() const;
   bool fromDynamicObject(const DynamicObject *pDynObj);

   static bool testConnection(const ArcProxyLib::ConnectionParameters &connection, 
      ArcProxyLib::FeatureClassProperties &properties, std::string &errorMessage);

   static const std::string DEFAULT_LAYER_NAME;

protected slots:
   void addFeature(const ArcProxyLib::Feature &feature);

private:
   static const std::string CONNECTION_KEY;
   static const std::string QUERY_KEY;
   static const std::string CLIPPING_TYPE_KEY;
   static const std::string CLIP_LL_KEY;
   static const std::string CLIP_UR_KEY;
   static const std::string LAYER_NAME_KEY;

   // Don't need to listen for deletion of or clean up this pointer, 
   // since this AnyData will be deleted before the parent element
   GraphicElement* mpParentElement;

   std::string mFeatureClassId;

   ArcProxyLib::ConnectionParameters mConnection;
   ArcProxyLib::FeatureClassProperties mProperties;
   std::vector<QueryOptions> mQueries;
   ClippingType mClippingType;
   LocationType mLlClip;
   LocationType mUrClip;
   std::string mLayerName;

   // only valid during load
   GraphicGroup* mpLoadGroup;
   Progress* mpLoadProgress;
   const QueryOptions* mpLoadQueryOptions;
   unsigned int mProgress;
   unsigned int mProgressBase;
   unsigned int mProgressSize;
};

#endif
