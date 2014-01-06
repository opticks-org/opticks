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

#include <QtCore/QAbstractTableModel>

#include "AnyData.h"
#include "AttachmentPtr.h"
#include "ConnectionParameters.h"
#include "DisplayQueryOptions.h"
#include "EnumWrapper.h"
#include "FeatureClassProperties.h"
#include "FeatureQueryOptions.h"
#include "GraphicElement.h"
#include "GraphicGroup.h"
#include "ObjectResource.h"
#include "LocationType.h"

#include <vector>
#include <utility>

class FeatureProxyConnector;
class GraphicGroup;
class GraphicLayer;
class Progress;

class FeatureClass : public QAbstractTableModel, public AnyData
{
   Q_OBJECT

public:
   FeatureClass();
   virtual ~FeatureClass();

   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

   bool setParentElement(GraphicElement* pParentElement);
   GraphicElement* getParentElement();
   const GraphicElement* getParentElement() const;

   bool open(std::string& errorMessage);
   bool close(std::string& errorMessage);

   bool hasLabels() const;

   bool setConnectionParameters(const ArcProxyLib::ConnectionParameters& connection);
   const ArcProxyLib::ConnectionParameters& getConnectionParameters() const;

   void removeQuery(const std::string& queryName);
   bool addQuery(const FeatureQueryOptions& query);
   const std::vector<FeatureQueryOptions>& getQueries() const;
   std::vector<DisplayQueryOptions> getDisplayQueryOptions(const std::string& queryName);
   bool replaceQueryNameInQueriesLists(const std::string& oldName, const std::string& newName);
   FeatureQueryOptions* getQueryByName(const std::string& name);
   void updateQuery(const FeatureQueryOptions& query);
   void copyQueryGraphicIds(const std::vector<FeatureQueryOptions>& queries);

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

   bool update(Progress* pProgress, std::string& errorMessage, bool bEditDisplayOnly);

   GraphicLayer* getFeatureLayer() const;

   FactoryResource<DynamicObject> toDynamicObject() const;
   bool fromDynamicObject(const DynamicObject *pDynObj);

   static bool testConnection(const ArcProxyLib::ConnectionParameters &connection, 
      ArcProxyLib::FeatureClassProperties &properties, std::string &errorMessage);

   static const std::string DEFAULT_LAYER_NAME;
   void getFieldValues(const std::string& field, std::vector<std::string>& values);
   unsigned int getCurrentQueryIndex() const;
   void incrementCurrentQueryIndex();

public slots:
   void addDisplayQuery(DisplayQueryOptions* pQuery);
   void modifyDisplayQuery(const std::vector<DisplayQueryOptions*>& pQueries, bool bGraphicChange);
   void renameDisplayQuery(const std::string& queryName, const std::string& oldName, const std::string& newName);
   void removeDisplayQuery(const std::vector<DisplayQueryOptions*>& pQueries);

   void populateDisplayQueries(const std::string& queryName, const std::string& field, bool isUniqueLineColor,
      bool isUniqueFillColor);

protected slots:
   void addFeature(const ArcProxyLib::Feature &feature);

private:
   FeatureClass(const FeatureClass& rhs);
   FeatureClass& operator=(const FeatureClass& rhs);
   void refreshVerticalHeader(Subject& subject, const std::string& signal, const boost::any& data);
   void addObject(Subject& subject, const std::string& signal, const boost::any& data);
   void removeObject(Subject& subject, const std::string& signal, const boost::any& data);
   void populateDisplayQueries();

   static const std::string CONNECTION_KEY;
   static const std::string QUERY_KEY;
   static const std::string DISPLAY_QUERY_KEY;
   static const std::string CLIPPING_TYPE_KEY;
   static const std::string CLIP_LL_KEY;
   static const std::string CLIP_UR_KEY;
   static const std::string LAYER_NAME_KEY;
   static const std::string PROPERTIES_KEY;
   static const std::string FEATURE_ATTRIBUTES_NAME;


   // Don't need to listen for deletion of or clean up this pointer, 
   // since this AnyData will be deleted before the parent element
   GraphicElement* mpParentElement;
   AttachmentPtr<GraphicGroup> mpGraphicGroup;

   std::string mFeatureClassId;

   ArcProxyLib::ConnectionParameters mConnection;
   ArcProxyLib::FeatureClassProperties mProperties;
   std::vector<std::string> mGraphicObjectIds;
   std::vector<FeatureQueryOptions> mQueries;
   std::vector<DisplayQueryOptions*> mDisplayQueries;
   ClippingType mClippingType;
   LocationType mLlClip;
   LocationType mUrClip;
   std::string mLayerName;

   // only valid during load
   GraphicGroup* mpLoadGroup;
   Progress* mpLoadProgress;
   FeatureQueryOptions* mpLoadQueryOptions;
   unsigned int mProgress;
   unsigned int mProgressBase;
   unsigned int mProgressSize;
   std::vector<std::vector<std::string> > mAttributeValues;
   bool mbUniqueFillColor;
   bool mbUniqueLineColor;
   std::string mUniqueField;
   std::string mUniqueQueryName;
   unsigned int mCurrentQueryIndex;
};

#endif
