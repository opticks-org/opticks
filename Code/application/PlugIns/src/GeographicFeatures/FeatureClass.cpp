/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "FeatureClass.h"
#include "FeatureProxyConnector.h"
#include "GraphicGroup.h"
#include "GraphicElement.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "ObjectResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "TypeConverter.h"
#include "Undo.h"

#include <algorithm>

#include <boost/bind.hpp>

using namespace std;

const string FeatureClass::CONNECTION_KEY = "connection";
const string FeatureClass::QUERY_KEY = "query";
const string FeatureClass::CLIPPING_TYPE_KEY = "clippingtype";
const string FeatureClass::CLIP_LL_KEY = "clipll";
const string FeatureClass::CLIP_UR_KEY = "clipur";
const string FeatureClass::LAYER_NAME_KEY = "layername";
const string FeatureClass::DEFAULT_LAYER_NAME = "New feature class";

FeatureClass::FeatureClass() : mpParentElement(NULL), mClippingType(SCENE_CLIP), 
   mLayerName(DEFAULT_LAYER_NAME), mpLoadGroup(NULL), mpLoadProgress(NULL),
   mpLoadQueryOptions(NULL), mProgress(0), mProgressBase(0), mProgressSize(0)

{
   mQueries.push_back(QueryOptions());
}

FeatureClass::~FeatureClass()
{
   if (!mFeatureClassId.empty())
   {
      string errorMessage;
      close(errorMessage);
   }
}

bool FeatureClass::open(string &errorMessage)
{
   if (!close(errorMessage))
   {
      return false;
   }

   FeatureProxyConnector *pProxy = FeatureProxyConnector::instance();
   if (VERIFYNR(pProxy != NULL))
   {
      if (!pProxy->openDataSource(mConnection, mFeatureClassId, errorMessage)) return false;
      if (!pProxy->getFeatureClassProperties(mFeatureClassId, mProperties, errorMessage)) return false;
   }

   return true;
}

bool FeatureClass::close(string &errorMessage)
{
   if (mFeatureClassId.empty())
   {
      return true; // nothing to close
   }

   FeatureProxyConnector *pProxy = FeatureProxyConnector::instance();
   VERIFY(pProxy != NULL);

   if (!pProxy->closeDataSource(mFeatureClassId, errorMessage))
   {
      return false;
   }

   mFeatureClassId.clear();
   return true;
}

AnyData *FeatureClass::copy() const
{
   return NULL;
}

bool FeatureClass::setParentElement(GraphicElement *pParentElement)
{
   mpParentElement = pParentElement;

   return true;
}

bool FeatureClass::setConnectionParameters(const ArcProxyLib::ConnectionParameters &connection)
{
   if (mConnection != connection)
   {
      string errorMessage;
      close(errorMessage);

      mConnection = connection;
   }
   return true;
}

const ArcProxyLib::ConnectionParameters &FeatureClass::getConnectionParameters() const
{
   return mConnection;
}

bool FeatureClass::addQuery(const QueryOptions &query)
{
   mQueries.push_back(query);

   return true;
}

bool FeatureClass::clearQueries()
{
   mQueries.clear();

   return true;
}

const vector<QueryOptions> &FeatureClass::getQueries() const
{
   return mQueries;
}

void FeatureClass::setClippingType(ClippingType clipping)
{
   mClippingType = clipping;
}

FeatureClass::ClippingType FeatureClass::getClippingType() const
{
   return mClippingType;
}

void FeatureClass::setClipping(LocationType ll, LocationType ur)
{
   mClippingType = SPECIFIED_CLIP;
   mLlClip = ll;
   mUrClip = ur;
}

pair<LocationType, LocationType> FeatureClass::getClipping() const
{
   pair<LocationType, LocationType> clipping;

   switch (mClippingType)
   {
   case SPECIFIED_CLIP:
      clipping.first = mLlClip;
      clipping.second = mUrClip;
      break;
   case SCENE_CLIP:
      {
         if (mpParentElement != NULL)
         {
            const RasterElement *pGrandParent = dynamic_cast<RasterElement*>(mpParentElement->getParent());
            if (pGrandParent != NULL)
            {
               const RasterDataDescriptor *pDesc = dynamic_cast<const RasterDataDescriptor*>(pGrandParent->getDataDescriptor());
               if (pDesc != NULL)
               {
                  LocationType ll = pGrandParent->convertPixelToGeocoord(LocationType(0,0));
                  LocationType lr = pGrandParent->convertPixelToGeocoord(LocationType(pDesc->getColumnCount(), 0));
                  LocationType ul = pGrandParent->convertPixelToGeocoord(LocationType(0, pDesc->getRowCount()));
                  LocationType ur = pGrandParent->convertPixelToGeocoord(LocationType(pDesc->getColumnCount(), pDesc->getRowCount()));

                  clipping.first.mX = min(ll.mX, ul.mX);
                  clipping.first.mX = min(clipping.first.mX, lr.mX);
                  clipping.first.mX = min(clipping.first.mX, ur.mX);

                  clipping.first.mY = min(ll.mY, ul.mY);
                  clipping.first.mY = min(clipping.first.mY, lr.mY);
                  clipping.first.mY = min(clipping.first.mY, ur.mY);

                  clipping.second.mX = max(ll.mX, ul.mX);
                  clipping.second.mX = max(clipping.second.mX, lr.mX);
                  clipping.second.mX = max(clipping.second.mX, ur.mX);

                  clipping.second.mY = max(ll.mY, ul.mY);
                  clipping.second.mY = max(clipping.second.mY, lr.mY);
                  clipping.second.mY = max(clipping.second.mY, ur.mY);
               }
            }
         }
      }
      break;
   case NO_CLIP:
   default: // fall through
      // default LocationType constructors are sufficient
      break;
   }

   // make sure first location is most southwest point and second is most northeast point
   double tmpDbl(0.0);
   if (clipping.first.mX > clipping.second.mX)
   {
      tmpDbl = clipping.first.mX;
      clipping.first.mX = clipping.second.mX;
      clipping.second.mX = tmpDbl;
   }
   if (clipping.first.mY > clipping.second.mY)
   {
      tmpDbl = clipping.first.mY;
      clipping.first.mY = clipping.second.mY;
      clipping.second.mY = tmpDbl;
   }

   return clipping;
}

void FeatureClass::setLayerName(const string &layerName)
{
   if (!layerName.empty())
   {
      mLayerName = layerName;
   }
}

const string &FeatureClass::getLayerName() const
{
   if (mLayerName.find(DEFAULT_LAYER_NAME) != string::npos)
   {
      string featureClassName = this->mConnection.getFeatureClass();
      if (!featureClassName.empty())
      {
         const_cast<FeatureClass*>(this)->mLayerName = featureClassName;
      }
   }
   return mLayerName;
}

bool FeatureClass::update(Progress *pProgress, string &errorMessage)
{
   VERIFY(mpParentElement != NULL);
   GraphicGroup *pGroup = mpParentElement->getGroup();
   VERIFY(pGroup != NULL);

   GraphicObjectExt1* pGroupExt1 = dynamic_cast<GraphicObjectExt1*>(pGroup);
   VERIFY(pGroupExt1 != NULL);

   View* pView = NULL;
   GraphicLayer* pLayer = pGroupExt1->getLayer();
   if (pLayer != NULL)
   {
      pView = pLayer->getView();
   }

   UndoGroup undoGroup(pView, "Update Geographic Features");

   if (pProgress) pProgress->updateProgress("Removing old shapes", 0, NORMAL);
   pGroup->removeAllObjects(true);

   FeatureProxyConnector *pProxy = FeatureProxyConnector::instance();
   VERIFY(pProxy != NULL);
   if (mFeatureClassId.empty())
   {
      if (!open(errorMessage))
      {
         return false;
      }
   }

   VERIFY(!mFeatureClassId.empty());

   bool success = true;
   mpParentElement->setInteractive(false);

   mpLoadGroup = pGroup;
   mpLoadProgress = pProgress;
   mProgressSize = 89 / mQueries.size();
   mProgressBase = 10;

   pair<LocationType, LocationType> clipping = getClipping();

   VERIFYNR(connect(pProxy, SIGNAL(featureLoaded(const ArcProxyLib::Feature&)), 
      this, SLOT(addFeature(const ArcProxyLib::Feature&))));


   for (vector<QueryOptions>::const_iterator iter = mQueries.begin();
      success && iter != mQueries.end(); ++iter)
   {
      mProgress = 0;
      mpLoadQueryOptions = &(*iter);
      success = pProxy->query(mFeatureClassId, errorMessage, mpLoadQueryOptions->getQueryString(),
         mpLoadQueryOptions->getFormatString(), clipping.first, clipping.second);
      mProgressBase += mProgressSize;
   }

   VERIFYNR(disconnect(pProxy, SIGNAL(featureLoaded(const ArcProxyLib::Feature&)), 
      this, SLOT(addFeature(const ArcProxyLib::Feature&))));

   mpLoadGroup = NULL;
   mpLoadProgress = NULL;
   mpLoadQueryOptions = NULL;
   mProgress = 0;
   mProgressSize = 0;
   mProgressBase = 0;

   mpParentElement->setInteractive(true);

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Complete", 100, NORMAL);
   }
   return success;
}

void FeatureClass::addFeature(const ArcProxyLib::Feature &feature)
{
   VERIFYNRV(mpLoadGroup != NULL && mpLoadQueryOptions != NULL);

   if(mpLoadProgress != NULL)
   {
      mpLoadProgress->updateProgress("Inserting shapes...", mProgressBase + mProgress, NORMAL);
      mProgress = (mProgress+1) % mProgressSize;
   }

   GraphicObjectType grobType(VIEW_OBJECT); // VIEW_OBJECT not supported so we use this as a NULL
   switch(feature.getType())
   {
   case ArcProxyLib::POINT: // fall through
   case ArcProxyLib::MULTIPOINT:
      grobType = MULTIPOINT_OBJECT; 
      break;
   case ArcProxyLib::POLYLINE:
      grobType = POLYLINE_OBJECT;
      break;
   case ArcProxyLib::POLYGON:
      grobType = POLYGON_OBJECT;
      break;
   default:
      grobType = VIEW_OBJECT;
      break;
   }
   if (grobType != VIEW_OBJECT)
   {
      GraphicObject *pGraphic = mpLoadGroup->addObject(grobType, LocationType(0, 0));

      pGraphic->setName(feature.getLabel());

      vector<pair<double, double> >::const_iterator currentVertex = feature.getVertices().begin();
      vector<LocationType>::size_type previousIndex = 0;
      for(vector<size_t>::const_iterator path = feature.getPaths().begin();
                                         path != feature.getPaths().end(); ++path)
      {
         vector<pair<double, double> >::const_iterator endVertex = currentVertex + (*path - previousIndex);
         vector<LocationType> vertices;
         std::copy(currentVertex, endVertex, back_inserter(vertices));
         pGraphic->addGeoVertices(vertices);
         pGraphic->newPath();
         currentVertex = endVertex;
         previousIndex = *path;
      }
      vector<LocationType> vertices;
      std::copy(currentVertex, feature.getVertices().end(), back_inserter(vertices));
      pGraphic->addGeoVertices(vertices);
      mpLoadQueryOptions->setOnGraphicObject(pGraphic);
   }
}

FactoryResource<DynamicObject> FeatureClass::toDynamicObject() const
{
   FactoryResource<DynamicObject> pDynObj;

   FactoryResource<DynamicObject> pConnection = mConnection.toDynamicObject();
   VERIFYRV(pConnection.get() != NULL, pDynObj);

   pDynObj->setAttribute(CONNECTION_KEY, *pConnection.get());

   for (vector<QueryOptions>::const_iterator iter = mQueries.begin();
      iter != mQueries.end(); ++iter)
   {
      FactoryResource<DynamicObject> pQuery(iter->toDynamicObject());
      VERIFYRV(pQuery.get() != NULL, pDynObj);

      pDynObj->setAttributeByPath(QUERY_KEY + "/" + iter->getQueryName(), *pQuery.get());
   }

   pDynObj->setAttribute(CLIPPING_TYPE_KEY, static_cast<int>(mClippingType));
   pDynObj->setAttribute(CLIP_LL_KEY, mLlClip);
   pDynObj->setAttribute(CLIP_UR_KEY, mUrClip);
   pDynObj->setAttribute(LAYER_NAME_KEY, mLayerName);
   
   return pDynObj;
}

bool FeatureClass::fromDynamicObject(const DynamicObject *pDynObj)
{
   if (pDynObj == NULL)
   {
      return false;
   }

   string errorMessage;
   close(errorMessage);

   mQueries.clear();

   VERIFY(mConnection.fromDynamicObject(pDynObj->getAttribute(CONNECTION_KEY).getPointerToValue<DynamicObject>()));

   const DynamicObject *pQueries = pDynObj->getAttribute(QUERY_KEY).getPointerToValue<DynamicObject>();
   VERIFY(pQueries != NULL);

   vector<string> attributeNames;
   pQueries->getAttributeNames(attributeNames);

   for (vector<string>::const_iterator iter = attributeNames.begin();
      iter != attributeNames.end(); ++iter)
   {
      QueryOptions query;
      query.fromDynamicObject(pQueries->getAttribute(*iter).getPointerToValue<DynamicObject>());
      mQueries.push_back(query);
   }

   try
   {
      mClippingType = static_cast<ClippingTypeEnum>(dv_cast<int>(pDynObj->getAttribute(CLIPPING_TYPE_KEY)));
      mLlClip = dv_cast<LocationType>(pDynObj->getAttribute(CLIP_LL_KEY));
      mUrClip = dv_cast<LocationType>(pDynObj->getAttribute(CLIP_UR_KEY));
      mLayerName = dv_cast<string>(pDynObj->getAttribute(LAYER_NAME_KEY));
   }
   catch (bad_cast e)
   {
      VERIFY_MSG(false, "Clipping information missing");
   }

   return true;
}

const ArcProxyLib::FeatureClassProperties &FeatureClass::getFeatureClassProperties() const
{
   return mProperties;
}

bool FeatureClass::testConnection(const ArcProxyLib::ConnectionParameters &connection, 
   ArcProxyLib::FeatureClassProperties &properties, string &errorMessage)
{
   FeatureClass featureClass;
   featureClass.setConnectionParameters(connection);
   if (!featureClass.open(errorMessage))
   {
      return false;
   }

   properties = featureClass.getFeatureClassProperties();

   return true;
}

bool FeatureClass::hasLabels() const
{
   for (vector<QueryOptions>::const_iterator iter = mQueries.begin();
      iter != mQueries.end(); ++iter)
   {
      if (!iter->getFormatString().empty())
      {
         return true;
      }
   }
   return false;
}
