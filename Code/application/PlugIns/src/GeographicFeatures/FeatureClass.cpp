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
#include "SessionResource.h"
#include "TypeConverter.h"
#include "Undo.h"

#include <algorithm>

#include <boost/any.hpp>
#include <boost/bind.hpp>

using namespace std;

const string FeatureClass::CONNECTION_KEY = "connection";
const string FeatureClass::QUERY_KEY = "query";
const string FeatureClass::DISPLAY_QUERY_KEY = "displayquery";
const string FeatureClass::CLIPPING_TYPE_KEY = "clippingtype";
const string FeatureClass::CLIP_LL_KEY = "clipll";
const string FeatureClass::CLIP_UR_KEY = "clipur";
const string FeatureClass::LAYER_NAME_KEY = "layername";
const string FeatureClass::DEFAULT_LAYER_NAME = "New feature class";
const string FeatureClass::PROPERTIES_KEY = "Properties";
const string FeatureClass::FEATURE_ATTRIBUTES_NAME = "FeatureAttributes";

FeatureClass::FeatureClass() :
   mpParentElement(NULL),
   mpGraphicGroup(NULL),
   mClippingType(SCENE_CLIP),
   mLayerName(DEFAULT_LAYER_NAME),
   mpLoadGroup(NULL),
   mpLoadProgress(NULL),
   mpLoadQueryOptions(NULL),
   mProgress(0),
   mProgressBase(0),
   mProgressSize(0),
   mbUniqueFillColor(false),
   mbUniqueLineColor(false),
   mCurrentQueryIndex(0)
{
   mQueries.push_back(FeatureQueryOptions());

   mpGraphicGroup.addSignal(SIGNAL_NAME(GraphicGroup, ObjectRemoved), Slot(this, &FeatureClass::removeObject));
   mpGraphicGroup.addSignal(SIGNAL_NAME(GraphicGroup, ObjectAdded), Slot(this, &FeatureClass::addObject));
}

FeatureClass::~FeatureClass()
{
   if (!mFeatureClassId.empty())
   {
      string errorMessage;
      close(errorMessage);
   }
}

int FeatureClass::rowCount(const QModelIndex& parent) const
{
   // Return size of graphic object ids (size of mpParentElement->getGroup->getObjects() contains objects
   // which are not depicted as geographic features (ie: arrow, view, scale)).
   return static_cast<int>(mGraphicObjectIds.size());
}

int FeatureClass::columnCount(const QModelIndex& parent) const
{
   int numColumns = 0;
   if (mpParentElement != NULL)
   {
      const DynamicObject* pValues = mpParentElement->getMetadata();
      if (pValues != NULL)
      {
         pValues = pValues->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
      }
      if (pValues != NULL)
      {
         numColumns = static_cast<int>(pValues->getNumAttributes());
      }
   }

   return numColumns;
}

QVariant FeatureClass::data(const QModelIndex& index, int role) const
{
   if (role == Qt::UserRole)
   {
      //populate the table with the graphic object's IDs, which will be used
      //for indexing
      if (index.row() < static_cast<int>(mGraphicObjectIds.size()))
      {
         return QVariant(QString::fromStdString(mGraphicObjectIds[index.row()]));
      }
   }
   else if (role == Qt::DisplayRole)
   {
      if (mpParentElement != NULL)
      {
         const DynamicObject* pValues = mpParentElement->getMetadata();
         if (pValues != NULL)
         {
            pValues = pValues->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
         }
         if (pValues != NULL)
         {
            //populate the table with the names of the graphic objects
            std::vector<std::string> fields = mProperties.getFields();
            if (index.column() < static_cast<int>(fields.size()))
            {
               //populate the table from the dynamic object of the GraphicElement
               std::string attrName = fields.at(index.column());

               const DynamicObject* pAttributes = mpParentElement->getMetadata();
               if (pAttributes != NULL)
               {
                  pAttributes = pAttributes->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
               }
               if (pAttributes != NULL)
               {
                  const DataVariant& variant = pAttributes->getAttribute(attrName);

                  string type = variant.getTypeName();
                  //the types of double, int, short, float and string are the types
                  //that can come out of ArcProxy, since unsigned types are not expected
                  //we don't have them in this list
                  if (type == "vector<float>")
                  {
                     const vector<float>* pColumn = variant.getPointerToValue<vector<float> >();

                     if (pColumn != NULL)
                     {
                        if (index.row() < static_cast<int>(pColumn->size()))
                        {
                           float value = pColumn->at(index.row());
                           return QVariant(static_cast<double>(value));
                        }
                     }
                  }
                  else if (type == "vector<double>")
                  {
                     const vector<double>* pColumn = variant.getPointerToValue<vector<double> >();

                     if (pColumn != NULL)
                     {
                        if (index.row() < static_cast<int>(pColumn->size()))
                        {
                           double value = pColumn->at(index.row());
                           return QVariant(value);
                        }
                     }
                  }
                  else if (type == "vector<string>")
                  {
                     const vector<string>* pColumn = variant.getPointerToValue<vector<string> >();

                     if (pColumn != NULL)
                     {
                        if (index.row() < static_cast<int>(pColumn->size()))
                        {
                           std::string value = pColumn->at(index.row());
                           return QVariant(QString::fromStdString(value));
                        }
                     }
                  }
                  else if (type == "vector<int>")
                  {
                     const vector<int>* pColumn = variant.getPointerToValue<vector<int> >();

                     if (pColumn != NULL)
                     {
                        if (index.row() < static_cast<int>(pColumn->size()))
                        {
                           int value = pColumn->at(index.row());
                           return QVariant(value);
                        }
                     }
                  }
                  else if (type == "vector<short>")
                  {
                     const vector<short>* pColumn = variant.getPointerToValue<vector<short> >();

                     if (pColumn != NULL)
                     {
                        if (index.row() < static_cast<int>(pColumn->size()))
                        {
                           int value = pColumn->at(index.row());
                           return QVariant(static_cast<short>(value));
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return QVariant();
}

QVariant FeatureClass::headerData(int section, Qt::Orientation orientation, int role) const
{
   if (section >= 0)
   {
      if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
      {
         std::vector<string> fields = mProperties.getFields();
         if (section < static_cast<int>(fields.size()))
         {
            return QVariant(QString::fromStdString(fields[section]));
         }
      }
      else if ((orientation == Qt::Vertical) && (role == Qt::DisplayRole))
      {
         if (section < static_cast<int>(mGraphicObjectIds.size()))
         {
            std::string id = mGraphicObjectIds.at(section);
            if (mpParentElement != NULL)
            {
               const GraphicGroup* pGroup = mpParentElement->getGroup();
               if (pGroup != NULL)
               {
                  const std::list<GraphicObject*>& objects = pGroup->getObjects();
                  for (std::list<GraphicObject*>::const_iterator objectsIter = objects.begin();
                     objectsIter != objects.end();
                     ++objectsIter)
                  {
                     GraphicObject* pObject = *objectsIter;
                     if (pObject != NULL)
                     {
                        if (pObject->getId() == id)
                        {
                           std::string objectName = pObject->getName();
                           return QVariant(QString::fromStdString(objectName));
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return QVariant();
}

bool FeatureClass::open(string &errorMessage)
{
   if (!close(errorMessage))
   {
      return false;
   }

   FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
   if (VERIFYNR(pProxy != NULL))
   {
      if (!pProxy->openDataSource(mConnection, mFeatureClassId, errorMessage))
      {
         return false;
      }

      if (!pProxy->getFeatureClassProperties(mFeatureClassId, mProperties, errorMessage))
      {
         return false;
      }
   }

   return true;
}

bool FeatureClass::close(string &errorMessage)
{
   if (mFeatureClassId.empty())
   {
      return true; // nothing to close
   }

   FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
   VERIFY(pProxy != NULL);

   if (!pProxy->closeDataSource(mFeatureClassId, errorMessage))
   {
      return false;
   }

   mFeatureClassId.clear();
   return true;
}

bool FeatureClass::setParentElement(GraphicElement *pParentElement)
{
   if (pParentElement != mpParentElement)
   {
      beginResetModel();

      mpParentElement = pParentElement;
      if (pParentElement != NULL)
      {
         mpGraphicGroup.reset(pParentElement->getGroup());

         // Add any existing objects.
         if (mpGraphicGroup.get() != NULL)
         {
            const std::list<GraphicObject*>& graphicObjects = mpGraphicGroup->getObjects();
            for (std::list<GraphicObject*>::const_iterator iter = graphicObjects.begin();
               iter != graphicObjects.end();
               ++iter)
            {
               addObject(*(mpGraphicGroup.get()), SIGNAL_NAME(GraphicGroup, ObjectAdded), boost::any(*iter));
            }
         }
      }
      else
      {
         mpGraphicGroup.reset();
      }

      endResetModel();
      return true;
   }

   return false;
}

GraphicElement* FeatureClass::getParentElement()
{
   return mpParentElement;
}

const GraphicElement* FeatureClass::getParentElement() const
{
   return mpParentElement;
}

bool FeatureClass::setConnectionParameters(const ArcProxyLib::ConnectionParameters& connection)
{
   bool bChanged = false;
   if (mConnection != connection)
   {
      string errorMessage;
      close(errorMessage);
      
      bChanged = true;

      mConnection = connection;
   }
   return bChanged;
}

const ArcProxyLib::ConnectionParameters &FeatureClass::getConnectionParameters() const
{
   return mConnection;
}

bool FeatureClass::addQuery(const FeatureQueryOptions &query)
{
   mQueries.push_back(query);

   return true;
}

void FeatureClass::updateQuery(const FeatureQueryOptions& query)
{
   for (std::vector<FeatureQueryOptions>::iterator iter = mQueries.begin(); iter != mQueries.end(); ++iter)
   {
      //where the query names match, set the rest of the values
      if (iter->getQueryName() == query.getQueryName())
      {
         iter->setQueryString(query.getQueryString());
         iter->setFillColor(query.getFillColor());
         iter->setFillStyle(query.getFillStyle());
         iter->setFormatString(query.getFormatString());
         iter->setHatchStyle(query.getHatchStyle());
         iter->setLineColor(query.getLineColor());
         iter->setLineScaled(query.getLineScaled());
         iter->setLineState(query.getLineState());
         iter->setLineStyle(query.getLineStyle());
         iter->setLineWidth(query.getLineWidth());
         iter->setSymbolName(query.getSymbolName());
         iter->setSymbolSize(query.getSymbolSize());
         break;
      }
   }
}

void FeatureClass::removeQuery(const std::string& queryName)
{
   for (std::vector<FeatureQueryOptions>::iterator iter = mQueries.begin(); iter != mQueries.end(); ++iter)
   {
      if (iter->getQueryName() == queryName)
      {
         mQueries.erase(iter);
         break;
      }
   }
   std::vector<DisplayQueryOptions*> deleteOptions;
   for (unsigned int i = 0; i < mDisplayQueries.size(); i++)
   {
      if (mDisplayQueries[i]->getQueryName() == queryName)
      {
         deleteOptions.push_back(mDisplayQueries[i]);
      }
   }
   if (deleteOptions.size() > 0)
   {
      removeDisplayQuery(deleteOptions);
   }
}

const vector<FeatureQueryOptions> &FeatureClass::getQueries() const
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
            const RasterElement* pGrandParent = dynamic_cast<RasterElement*>(mpParentElement->getParent());
            if (pGrandParent != NULL)
            {
               const RasterDataDescriptor* pDesc =
                  dynamic_cast<const RasterDataDescriptor*>(pGrandParent->getDataDescriptor());
               if (pDesc != NULL)
               {
                  LocationType ll = pGrandParent->convertPixelToGeocoord(LocationType(0, 0));
                  LocationType lr = pGrandParent->convertPixelToGeocoord(LocationType(pDesc->getColumnCount(), 0));
                  LocationType ul = pGrandParent->convertPixelToGeocoord(LocationType(0, pDesc->getRowCount()));
                  LocationType ur = pGrandParent->convertPixelToGeocoord(LocationType(pDesc->getColumnCount(),
                     pDesc->getRowCount()));

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
      string featureClassName = mConnection.getFeatureClass();
      if (!featureClassName.empty())
      {
         const_cast<FeatureClass*>(this)->mLayerName = featureClassName;
      }
   }
   return mLayerName;
}

bool FeatureClass::update(Progress* pProgress, string& errorMessage, bool bEditDisplayOnly)
{
   // prevent session auto save while updating features
   SessionSaveLock lock;

   VERIFY(mpParentElement != NULL);
   GraphicGroup* pGroup = mpParentElement->getGroup();
   VERIFY(pGroup != NULL);

   View* pView = NULL;

   GraphicLayer* pLayer = pGroup->getLayer();
   if (pLayer != NULL)
   {
      pView = pLayer->getView();
   }

   UndoGroup undoGroup(pView, "Update Geographic Features");
   bool success = true;
   if (pProgress)
   {
      pProgress->updateProgress("Removing old shapes", 0, NORMAL);
   }
   DynamicObject* pMetaData = NULL;
   if (mpParentElement != NULL)
   {
      DynamicObject* pRootMetaData = mpParentElement->getMetadata();
      if (pRootMetaData != NULL)
      {
         pMetaData = pRootMetaData->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
         if (pMetaData == NULL)
         {
            FactoryResource<DynamicObject> pNewObj;
            if (pNewObj.get() != NULL)
            {
               pRootMetaData->setAttribute(FEATURE_ATTRIBUTES_NAME, *(pNewObj.get()));
               pMetaData = pRootMetaData->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
            }
         }
      }
   }
   if (bEditDisplayOnly == false)
   {
      beginResetModel();
      pGroup->removeAllObjects(true);

      FeatureProxyConnector* pProxy = FeatureProxyConnector::instance();
      VERIFY(pProxy != NULL);
      if (mFeatureClassId.empty())
      {
         if (!open(errorMessage))
         {
            return false;
         }
      }

      VERIFY(!mFeatureClassId.empty());

      mpParentElement->setInteractive(false);

      mpLoadGroup = pGroup;
      mpLoadProgress = pProgress;
      
      mProgressSize = 89;
      if (mQueries.size() > 0)
      {
         mProgressSize /= mQueries.size();
      }
      mProgressBase = 10;

      pair<LocationType, LocationType> clipping = getClipping();
      //set up the containers to hold attribute information
      std::vector<std::string> fieldList;
      std::vector<std::string> typeList;
      fieldList = mProperties.getFields();
      typeList = mProperties.getTypes();

      if (!mAttributeValues.empty())
      {
         mAttributeValues.clear();
      }
      if (!fieldList.empty())
      {
         for (unsigned int i = 0; i < fieldList.size(); i++)
         {
            std::vector<std::string> values;
            mAttributeValues.push_back(values);
         }
      }

      mGraphicObjectIds.clear();
      VERIFYNR(connect(pProxy, SIGNAL(featureLoaded(const ArcProxyLib::Feature&)),
         this, SLOT(addFeature(const ArcProxyLib::Feature&))));

      for (vector<FeatureQueryOptions>::iterator iter = mQueries.begin(); success && iter != mQueries.end(); ++iter)
      {
         mProgress = 0;
         mpLoadQueryOptions = &(*iter);
         mpLoadQueryOptions->clearGraphicObjectIds();

         success = pProxy->query(mFeatureClassId, errorMessage, mpLoadQueryOptions->getQueryString(),
            mpLoadQueryOptions->getFormatString(), clipping.first, clipping.second);
         mProgressBase += mProgressSize;
      }

      VERIFYNR(disconnect(pProxy, SIGNAL(featureLoaded(const ArcProxyLib::Feature&)), this,
         SLOT(addFeature(const ArcProxyLib::Feature&))));

      //now that the the attributes are populated in the class, add them to the 
      //dynamic object
      if (pMetaData != NULL)
      {
         //loop through each field, determining its proper type
         //and adding its values to the dynamic object
         for (unsigned int i = 0; i < mAttributeValues.size(); i++)
         {
            std::vector<int> intArray;
            std::vector<short> shortArray;
            std::vector<std::string> stringArray;
            std::vector<float> floatArray;
            std::vector<double> doubleArray;

            if (typeList[i] == "Small Integer")
            {
               for (unsigned int j = 0; j < mAttributeValues[i].size(); j++)
               {
                  short value = QString::fromStdString(mAttributeValues[i].at(j)).toShort();
                  shortArray.push_back(value);
               }
               pMetaData->setAttribute(fieldList[i], shortArray);
            }
            else if (typeList[i] == "Integer")
            {
               for (unsigned int j = 0; j < mAttributeValues[i].size(); j++)
               {
                  int value = QString::fromStdString(mAttributeValues[i].at(j)).toInt();
                  intArray.push_back(value);
               }
               pMetaData->setAttribute(fieldList[i], intArray);
            }
            else if (typeList[i] == "Single")
            {
               for (unsigned int j = 0; j < mAttributeValues[i].size(); j++)
               {
                  float value = QString::fromStdString(mAttributeValues[i].at(j)).toFloat();
                  floatArray.push_back(value);
               }
               pMetaData->setAttribute(fieldList[i], floatArray);
            }
            else if (typeList[i] == "Double")
            {
               for (unsigned int j = 0; j < mAttributeValues[i].size(); j++)
               {
                  double value = QString::fromStdString(mAttributeValues[i].at(j)).toDouble();
                  doubleArray.push_back(value);
               }
               pMetaData->setAttribute(fieldList[i], doubleArray);
            }
            else if (typeList[i] == "String" || typeList[i] == "Date/Time")
            {
               for (unsigned int j = 0; j < mAttributeValues[i].size(); j++)
               {
                  stringArray.push_back(mAttributeValues[i].at(j));
               }
               pMetaData->setAttribute(fieldList[i], stringArray);
            }
         }

         mpParentElement->getMetadata()->setAttribute(FEATURE_ATTRIBUTES_NAME, *pMetaData);

         // Add graphic object id as an additional dynamicobject attribute
         mpParentElement->getMetadata()->setAttribute("GraphicObjectIds", mGraphicObjectIds);
      }

      endResetModel();
   }
   mProgressBase = 0;

   std::list<GraphicObject*> objects = pGroup->getObjects();
   int progressSize = objects.size() / 100;

   list<GraphicObject*>::const_iterator objIter;
   pMetaData = mpParentElement->getMetadata();
   if (pMetaData != NULL)
   {
      pMetaData = pMetaData->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
   }
   //loop through each query defined
   for (vector<FeatureQueryOptions>::const_iterator queryIter = mQueries.begin();
      queryIter != mQueries.end();
      ++queryIter)
   {
      unsigned int index = 0;
      std::string queryName = queryIter->getQueryName();
      //get the display queries associated with each primary query
      std::vector<DisplayQueryOptions> displayOptions = getDisplayQueryOptions(queryName);
      const std::vector<std::string> objectIds = queryIter->getGraphicObjectIds();
      //traverse the list of objects in the graphic layer
      for (objIter = objects.begin(); objIter != objects.end(); ++objIter)
      {
         GraphicObject* pObject = *objIter;
         if (pObject != NULL)
         {
            std::string currentId = pObject->getId();
            bool bFoundId = false;
            //only do this step if there is more than one primary query
            //otherwise this will slow things unnecessarily
            if (mQueries.size() > 1)
            {
               //loop through the object id's stored in the query, only
               //perform display updates if object is part of query
               for (unsigned int i = 0; i < objectIds.size(); i++)
               {
                  if (currentId == objectIds[i])
                  {
                     bFoundId = true;
                     break;
                  }
               }
            }
            else
            {
               bFoundId = true;
            }
            if (bFoundId)
            {
               bool bFound = false;

               //loop through each display query, applying to each graphic
               //object that matches the query string
               for (vector<DisplayQueryOptions>::const_iterator iter = displayOptions.begin();
                  iter != displayOptions.end();
                  ++iter)
               {
                  bool bMatch = false;
                  DisplayQueryOptions pOption = *iter;
                  QString original = QString::fromStdString(pOption.getQueryString());
                  QStringList originalList = original.split(",");
                  int querySize = originalList.size();
                  if (querySize > 1 && pOption.getQueryActive())
                  {
                     //if the query string is set to run and its valid then
                     //get the attribute values from the metadata
                     DataVariant variant = pMetaData->getAttribute(originalList[1].toStdString());
                     std::vector<std::string>* pStrVec = NULL;
                     std::vector<float>* pFltVec = NULL;
                     std::vector<double>* pDblVec = NULL;
                     std::vector<int>* pIntVec = NULL;
                     std::vector<short>* pShtVec = NULL;
                     float fCompareValue1;
                     float fCompareValue2;
                     short sCompareValue1;
                     short sCompareValue2;
                     int iCompareValue1;
                     int iCompareValue2;
                     double dCompareValue1;
                     double dCompareValue2;
                     std::string strCompareValue;

                     //the types of double, int, short, float and string are the types
                     //that can come out of ArcProxy, since unsigned types are not expected
                     //we don't have them in this list
                     std::string typeName = variant.getTypeName();
                     if (typeName == "vector<string>")
                     {
                        pStrVec = variant.getPointerToValue<std::vector<std::string> >();
                        strCompareValue = originalList[3].toStdString();
                        //a string has no range, so only check that the 
                        //two values are equal
                        if (strCompareValue == pStrVec->at(index))
                        {
                           bMatch = true;
                        }
                     }
                     else if (typeName == "vector<double>")
                     {
                        pDblVec = variant.getPointerToValue<std::vector<double> >();
                        dCompareValue1 = originalList[3].toDouble();
                        //the query string is long, we need to get the 
                        //second value
                        if (querySize > 6)
                        {
                           dCompareValue2 = originalList[6].toDouble();
                           if (pDblVec->at(index) >= dCompareValue1 && pDblVec->at(index) <= dCompareValue2)
                           {
                              bMatch = true;
                           }
                        }
                        else
                        {
                           if (dCompareValue1 == pDblVec->at(index))
                           {
                              bMatch = true;
                           }
                        }
                     }
                     else if (typeName == "vector<float>")
                     {
                        pFltVec = variant.getPointerToValue<std::vector<float> >();
                        fCompareValue1 = originalList[3].toFloat();
                        //we need to get the second value
                        if (querySize > 6)
                        {
                           fCompareValue2 = originalList[6].toFloat();
                           if (pFltVec->at(index) >= fCompareValue1 && pFltVec->at(index) <= fCompareValue2)
                           {
                              bMatch = true;
                           }
                        }
                        else
                        {
                           if (fCompareValue1 == pFltVec->at(index))
                           {
                              bMatch = true;
                           }
                        }
                     }
                     else if (typeName == "vector<int>")
                     {
                        pIntVec = variant.getPointerToValue<std::vector<int> >();
                        iCompareValue1 = originalList[3].toInt();
                        //we need to get the second value
                        if (querySize > 6)
                        {
                           iCompareValue2 = originalList[6].toInt();
                           if (pIntVec->at(index) >= iCompareValue1 && pIntVec->at(index) <= iCompareValue2)
                           {
                              bMatch = true;
                           }
                        }
                        else
                        {
                           if (iCompareValue1 == pIntVec->at(index))
                           {
                              bMatch = true;
                           }
                        }
                     }
                     else if (typeName == "vector<short>")
                     {
                        pShtVec = variant.getPointerToValue<std::vector<short> >();
                        sCompareValue1 = originalList[3].toShort();
                        //we need to get the second value
                        if (querySize > 6)
                        {
                           sCompareValue2 = originalList[6].toShort();
                           if (pShtVec->at(index) >= sCompareValue1 && pShtVec->at(index) <= sCompareValue2)
                           {
                              bMatch = true;
                           }
                        }
                        else
                        {
                           if (sCompareValue1 == pShtVec->at(index))
                           {
                              bMatch = true;
                           }
                        }
                     }
                     if (bMatch)
                     {
                        //we found a match in the above query,
                        //set the query on the graphic object
                        pOption.setOnGraphicObject(pObject);
                        bFound = true;
                     }
                  } 
               }
               mProgressBase += progressSize;
               if (bFound == false)
               {
                  //we did not find the object in the query,
                  //so apply the default, if queries were not
                  //active, then found will be false and the
                  //graphic object will be set
                  queryIter->setOnGraphicObject(pObject);
               }
            }
         }
         index++;
      }
   }
   mpLoadGroup = NULL;
   mpLoadProgress = NULL;
   mpLoadQueryOptions = NULL;
   mProgress = 0;
   mProgressSize = 0;
   mProgressBase = 0;

   mpParentElement->setInteractive(true);

   if (pProgress != NULL)
   {
      unsigned int count = mGraphicObjectIds.size();
      QString value = "Complete: " + QString::number(count) + " shapes imported";
      pProgress->updateProgress(value.toStdString(), 100, NORMAL);
   }

   return success;
}

GraphicLayer* FeatureClass::getFeatureLayer() const
{
   GraphicLayer* pLayer = NULL;
   if (mpParentElement != NULL)
   {
      const GraphicGroup* pGroup = mpParentElement->getGroup();
      VERIFYRV(pGroup != NULL, pLayer);
      pLayer = pGroup->getLayer();
   }
   return pLayer;
}

void FeatureClass::addFeature(const ArcProxyLib::Feature& feature)
{
   // prevent session auto save while adding feature
   SessionSaveLock lock;

   VERIFYNRV(mpLoadGroup != NULL && mpLoadQueryOptions != NULL);

   if (mpLoadProgress != NULL)
   {
      unsigned int count = mGraphicObjectIds.size();
      if (count % 10 == 0)
      {
         QString value = "Inserting shapes...  count: " + QString::number(count);
         mpLoadProgress->updateProgress(value.toStdString(), mProgressBase + mProgress, NORMAL);
         mProgress = (mProgress + 1) % mProgressSize;
      }
   }

   GraphicObjectType grobType(VIEW_OBJECT); // VIEW_OBJECT not supported so we use this as a NULL
   switch (feature.getType())
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
      GraphicObject* pGraphic = mpLoadGroup->addObject(grobType, LocationType(0, 0));

      pGraphic->setName(feature.getLabel());

      std::vector<std::string> attributes;
      attributes = feature.getAttributes();
      for (unsigned int i = 0; i < mAttributeValues.size(); i++)
      {
         mAttributeValues[i].push_back(attributes[i]);
      }

      vector<pair<double, double> >::const_iterator currentVertex = feature.getVertices().begin();
      vector<LocationType>::size_type previousIndex = 0;
      for (vector<size_t>::const_iterator path = feature.getPaths().begin(); path != feature.getPaths().end(); ++path)
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
      std::string id = pGraphic->getId();
      mpLoadQueryOptions->addGraphicObjectId(id);
   }
}

FactoryResource<DynamicObject> FeatureClass::toDynamicObject() const
{
   FactoryResource<DynamicObject> pDynObj;

   FactoryResource<DynamicObject> pConnection = mConnection.toDynamicObject();
   VERIFYRV(pConnection.get() != NULL, pDynObj);

   pDynObj->setAttribute(CONNECTION_KEY, *pConnection.get());

   for (vector<FeatureQueryOptions>::const_iterator iter = mQueries.begin(); iter != mQueries.end(); ++iter)
   {
      FactoryResource<DynamicObject> pQuery(iter->toDynamicObject());
      VERIFYRV(pQuery.get() != NULL, pDynObj);

      pDynObj->setAttributeByPath(QUERY_KEY + "/" + iter->getQueryName(), *pQuery.get());
   }
   for (vector<DisplayQueryOptions*>::const_iterator iter = mDisplayQueries.begin();
      iter != mDisplayQueries.end();
      ++iter)
   {
      DisplayQueryOptions* pOption = *iter;
      FactoryResource<DynamicObject> pQuery(pOption->toDynamicObject());
      VERIFYRV(pQuery.get() != NULL, pDynObj);

      pDynObj->setAttributeByPath(DISPLAY_QUERY_KEY + "/" + pOption->getUniqueName(), *pQuery.get());
   }
   pDynObj->setAttribute(CLIPPING_TYPE_KEY, static_cast<int>(mClippingType));
   pDynObj->setAttribute(CLIP_LL_KEY, mLlClip);
   pDynObj->setAttribute(CLIP_UR_KEY, mUrClip);
   pDynObj->setAttribute(LAYER_NAME_KEY, mLayerName);
   pDynObj->setAttribute(PROPERTIES_KEY, mProperties.toString());

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

   const DynamicObject* pQueries = pDynObj->getAttribute(QUERY_KEY).getPointerToValue<DynamicObject>();
   if (pQueries != NULL)
   {
      vector<string> attributeNames;
      pQueries->getAttributeNames(attributeNames);

      for (vector<string>::const_iterator iter = attributeNames.begin(); iter != attributeNames.end(); ++iter)
      {
         FeatureQueryOptions query;
         query.fromDynamicObject(pQueries->getAttribute(*iter).getPointerToValue<DynamicObject>());
         mQueries.push_back(query);
      }
   }
   mDisplayQueries.clear();
   const DynamicObject* pDisplayQueries = pDynObj->getAttribute(DISPLAY_QUERY_KEY).getPointerToValue<DynamicObject>();
   if (pDisplayQueries != NULL)
   {
      vector<string> displayAttributeNames;
      pDisplayQueries->getAttributeNames(displayAttributeNames);

      for (vector<string>::const_iterator iter = displayAttributeNames.begin();
         iter != displayAttributeNames.end();
         ++iter)
      {
         DisplayQueryOptions* pQuery = new DisplayQueryOptions();
         pQuery->fromDynamicObject(pDisplayQueries->getAttribute(*iter).getPointerToValue<DynamicObject>());
         mDisplayQueries.push_back(pQuery);
      }
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
   DataVariant propertyVar = pDynObj->getAttribute(PROPERTIES_KEY);
   if (propertyVar.isValid())
   {
      mProperties.fromString(dv_cast<std::string>(propertyVar));
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
   for (vector<FeatureQueryOptions>::const_iterator iter = mQueries.begin(); iter != mQueries.end(); ++iter)
   {
      if (!iter->getFormatString().empty())
      {
         return true;
      }
   }
   return false;
}

void FeatureClass::refreshVerticalHeader(Subject& subject, const std::string& signal, const boost::any& data)
{
   GraphicObject* pObject = dynamic_cast<GraphicObject*>(&subject);
   if (pObject != NULL)
   {
      const std::string& objectId = pObject->getId();
      VERIFYNRV(objectId.empty() == false);

      for (std::vector<std::string>::size_type i = 0; i < mGraphicObjectIds.size(); ++i)
      {
         if (mGraphicObjectIds[i] == objectId)
         {
            int rowIndex = static_cast<int>(i);
            emit headerDataChanged(Qt::Vertical, rowIndex, rowIndex);
            break;
         }
      }
   }
}

void FeatureClass::addObject(Subject& subject, const std::string& signal, const boost::any& data)
{
   GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(&subject);
   if (pGroup != NULL)
   {
      GraphicObject* pGraphicObject = boost::any_cast<GraphicObject*>(data);
      if (pGraphicObject != NULL)
      {
         GraphicObjectTypeEnum type = pGraphicObject->getGraphicObjectType();
         int rowIndex = rowCount();
         if (rowIndex >= 0)
         {
            switch (type)
            {
               case LINE_OBJECT:             // fall through
               case RECTANGLE_OBJECT:        // fall through
               case TRIANGLE_OBJECT:         // fall through
               case ELLIPSE_OBJECT:          // fall through
               case ROUNDEDRECTANGLE_OBJECT: // fall through
               case ARC_OBJECT:              // fall through
               case POLYLINE_OBJECT:         // fall through
               case POLYGON_OBJECT:          // fall through
               case MULTIPOINT_OBJECT:       // fall through
               case HLINE_OBJECT:            // fall through
               case VLINE_OBJECT:
                  pGraphicObject->attach(SIGNAL_NAME(GraphicObject, NameChanged),
                     Slot(this, &FeatureClass::refreshVerticalHeader));
                  mGraphicObjectIds.push_back(pGraphicObject->getId());
                  if (!(mAttributeValues.size() > 0 && mGraphicObjectIds.size() <= mAttributeValues[0].size()))
                  {
                     // Metadata for the graphic objects created by the import of shapefiles is already populated in
                     // update() method (number of graphic object ids will equal the length of the attribute vectors).
                     // If user unlocks resulting layer and then manually adds graphic objects, metadata for the
                     // manually added graphic objects needs to be populated with default values (number of graphic
                     // object ids is less than the length of the attribute vectors).
                     DynamicObject* pMetadata = mpParentElement->getMetadata();
                     if (pMetadata != NULL)
                     {
                        pMetadata = pMetadata->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
                     }
                     std::vector<std::string> attributeNames;
                     pMetadata->getAttributeNames(attributeNames);
                     for (unsigned k = 0; k < attributeNames.size(); k++)
                     {
                        DataVariant& variant = pMetadata->getAttribute(attributeNames[k]);
                        if (variant.getTypeName() == "vector<double>")
                        {
                           vector<double>* pColumn = variant.getPointerToValue<vector<double> >();
                           if ((pColumn != NULL) && (pColumn->size() < mGraphicObjectIds.size()))
                           {
                              pColumn->push_back(0.0);
                           }
                        }
                        else if (variant.getTypeName() == "vector<int>")
                        {
                           vector<int>* pColumn = variant.getPointerToValue<vector<int> >();
                           if ((pColumn != NULL) && (pColumn->size() < mGraphicObjectIds.size()))
                           {
                              pColumn->push_back(0);
                           }
                        }
                        else if (variant.getTypeName() == "vector<float>")
                        {
                           vector<float>* pColumn = variant.getPointerToValue<vector<float> >();
                           if ((pColumn != NULL) && (pColumn->size() < mGraphicObjectIds.size()))
                           {
                              pColumn->push_back(0.0f);
                           }
                        }
                        else if (variant.getTypeName() == "vector<string>")
                        {
                           vector<string>* pColumn = variant.getPointerToValue<vector<string> >();
                           if ((pColumn != NULL) && (pColumn->size() < mGraphicObjectIds.size()))
                           {
                              pColumn->push_back(string());
                           }
                        }
                        else if (variant.getTypeName() == "vector<short>")
                        {
                           vector<short>* pColumn = variant.getPointerToValue<vector<short> >();
                           if ((pColumn != NULL) && (pColumn->size() < mGraphicObjectIds.size()))
                           {
                              pColumn->push_back(0);
                           }
                        }
                     }
                  }
                  beginInsertRows(QModelIndex(), rowIndex, rowIndex);
                  endInsertRows();
                  break;
               default:
                  break;
            }
         }
      }
   }
}

void FeatureClass::removeObject(Subject& subject, const std::string& signal, const boost::any& data)
{
   GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(&subject);
   if (pGroup != NULL)
   {
      GraphicObject* pGraphicObject = boost::any_cast<GraphicObject*>(data);
      if (pGraphicObject != NULL)
      {
         //find index of the deleted graphic object.
         const std::string& objectId = pGraphicObject->getId();

         int index = 0;
         for (std::vector<std::string>::iterator iter = mGraphicObjectIds.begin();
            iter != mGraphicObjectIds.end();
            ++iter, ++index)
         {
            if (*iter == objectId)
            {
               //remove the deleted graphic objects id
               mGraphicObjectIds.erase(iter);

               //grab the metadata vectors and move all the data up one and pop the back
               DynamicObject* pMetadata = mpParentElement->getMetadata();
               if (pMetadata != NULL)
               {
                  pMetadata = pMetadata->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
               }

               std::vector<std::string> attributeNames;
               if (pMetadata != NULL)
               {
                  pMetadata->getAttributeNames(attributeNames);
                  for (unsigned k = 0; k < attributeNames.size(); k++)
                  {
                     DataVariant& variant = pMetadata->getAttribute(attributeNames[k]);
                     if (variant.getTypeName() == "vector<double>")
                     {
                        vector<double>* pColumn = variant.getPointerToValue<vector<double> >();
                        if (pColumn != NULL && index < static_cast<int>(pColumn->size()))
                        {
                           pColumn->erase(pColumn->begin() + index);
                        }
                     }
                     else if (variant.getTypeName() == "vector<int>")
                     {
                        vector<int>* pColumn = variant.getPointerToValue<vector<int> >();
                        if (pColumn != NULL && index < static_cast<int>(pColumn->size()))
                        {
                           pColumn->erase(pColumn->begin() + index);
                        }
                     }
                     else if (variant.getTypeName() == "vector<float>")
                     {
                        vector<float>* pColumn = variant.getPointerToValue<vector<float> >();
                        if (pColumn != NULL && index < static_cast<int>(pColumn->size()))
                        {
                           pColumn->erase(pColumn->begin() + index);
                        }
                     }
                     else if (variant.getTypeName() == "vector<string>")
                     {
                        vector<string>* pColumn = variant.getPointerToValue<vector<string> >();
                        if (pColumn != NULL && index < static_cast<int>(pColumn->size()))
                        {
                           pColumn->erase(pColumn->begin() + index);
                        }
                     }
                     else if (variant.getTypeName() == "vector<short>")
                     {
                        vector<short>* pColumn = variant.getPointerToValue<vector<short> >();
                        if (pColumn != NULL && index < static_cast<int>(pColumn->size()))
                        {
                           pColumn->erase(pColumn->begin() + index);
                        }
                     }
                  }
               }

               //must call begin/end remove rows methods when removing data
               //from model.  ensures the selections are properly updated.
               beginRemoveRows(QModelIndex(), index, index);
               endRemoveRows();

               pGraphicObject->detach(SIGNAL_NAME(GraphicObject, NameChanged),
                  Slot(this, &FeatureClass::refreshVerticalHeader));
               break;
            }
         }
      }
   }
}

void FeatureClass::addDisplayQuery(DisplayQueryOptions* pQuery)
{
   mDisplayQueries.push_back(pQuery);
}

void FeatureClass::renameDisplayQuery(const std::string& oldName, const std::string& newName)
{
   for (unsigned int i = 0 ; i < mDisplayQueries.size(); i++)
   {
      if (mDisplayQueries.at(i)->getUniqueName() == oldName)
      {
         mDisplayQueries.at(i)->setUniqueName(newName);
      }
   }
}

void FeatureClass::modifyDisplayQuery(const std::vector<DisplayQueryOptions*>& pQueries, bool bGraphicChange)
{
   for (unsigned int i = 0 ; i < pQueries.size(); i++)
   {
      //loop through the queries searching for the modified one
      DisplayQueryOptions* pModQuery = pQueries[i];
      if (pModQuery != NULL)
      {
         for (unsigned int j = 0; j < mDisplayQueries.size(); j++)
         {
            DisplayQueryOptions* pQuery = mDisplayQueries[j];
            if (pQuery != NULL)
            {
               if (pModQuery->getUniqueName() == pQuery->getUniqueName() &&
                  pModQuery->getQueryName() == pQuery->getQueryName())
               {
                  //graphic change refers to if the change was to the graphic
                  //through the OptionsDisplay object, or through the TreeWidget
                  if (bGraphicChange)
                  {
                     pQuery->setHatchStyle(pModQuery->getHatchStyle());
                     pQuery->setFillStyle(pModQuery->getFillStyle());
                     pQuery->setLineScaled(pModQuery->getLineScaled());
                     pQuery->setLineState(pModQuery->getLineState());
                     pQuery->setLineStyle(pModQuery->getLineStyle());
                     pQuery->setLineWidth(pModQuery->getLineWidth());
                     pQuery->setSymbolName(pModQuery->getSymbolName());
                     pQuery->setSymbolSize(pModQuery->getSymbolSize());
                     pQuery->setQueryName(pModQuery->getQueryName());
                     pQuery->setFillColor(pModQuery->getFillColor());
                     pQuery->setLineColor(pModQuery->getLineColor());
                  }
                  else
                  {
                     pQuery->setQueryString(pModQuery->getQueryString());
                     pQuery->setQueryActive(pModQuery->getQueryActive());
                     pQuery->setOrder(pModQuery->getOrder());
                  }
                  pQuery->setUniqueName(pModQuery->getUniqueName());
               }
            }
         }
      }
   }
}

void FeatureClass::removeDisplayQuery(const std::vector<DisplayQueryOptions*>& pQueries)
{
   if (pQueries.size() == mDisplayQueries.size())
   {
      mDisplayQueries.clear();
   }
   else
   {
      std::vector<DisplayQueryOptions*> tempOptions = mDisplayQueries;
      mDisplayQueries.clear();
      for (unsigned int j = 0; j < tempOptions.size(); j++)
      {
         bool bRemove = false;
         for (unsigned int i = 0 ; i < pQueries.size(); i++)
         {
            DisplayQueryOptions* pModQuery = pQueries[i];
            DisplayQueryOptions* pQuery = tempOptions[j];
            if (pModQuery != NULL && pQuery != NULL)
            {
               if (pModQuery->getUniqueName() == pQuery->getUniqueName() &&
                  pModQuery->getQueryName() == pQuery->getQueryName())
               {
                  bRemove = true;
                  break;
               }
            }
         }
         if (bRemove == false)
         {
            mDisplayQueries.push_back(tempOptions[j]);
         }
      }
   }
}

std::vector<DisplayQueryOptions> FeatureClass::getDisplayQueryOptions(const std::string& queryName)
{
   std::vector<DisplayQueryOptions> outputOptions;

   for (unsigned int i = 0; i < mDisplayQueries.size(); i++)
   {
      //traverse list of display queries in the feature class,
      //saving off the ones whose query name equals the passed 
      //in query name
      if (mDisplayQueries[i] != NULL)
      {
         if (mDisplayQueries[i]->getQueryName() == queryName)
         {
            outputOptions.push_back(*mDisplayQueries[i]);
         }
      }
   }
   //take the order into account so we are always returning the queries
   //in the correct order
   std::sort(outputOptions.begin(), outputOptions.end());

   return outputOptions;
}

FeatureQueryOptions* FeatureClass::getQueryByName(const std::string& name)
{
   FeatureQueryOptions* pOption = NULL;
   for (unsigned int i = 0; i < mQueries.size(); i++)
   {
      if (mQueries[i].getQueryName() == name)
      {
         pOption = &mQueries[i];
      }
   }
   return pOption;
}

bool FeatureClass::replaceQueryNameInQueriesLists(const std::string& oldName, const std::string& newName)
{
   bool bFound = false;
   FeatureQueryOptions* pOption = getQueryByName(oldName);
   if (pOption != NULL)
   {
      pOption->setQueryName(newName);
      bFound = true;
   }
   for (unsigned int i = 0; i < mDisplayQueries.size(); i++)
   {
      //traverse list of display queries in the feature class,
      //saving off the ones whose query name equals the passed 
      //in query name
      if (mDisplayQueries[i] != NULL)
      {
         if (mDisplayQueries[i]->getQueryName() == oldName)
         {
            mDisplayQueries[i]->setQueryName(newName);
         }
      }
   }
   return bFound;
}

template<typename T>
void populateFieldValuesFromDynamicObject(const std::vector<T>* pInput, std::vector<std::string>& values)
{
   if (pInput != NULL)
   {
      unsigned int size = pInput->size();
      if (size > 0)
      {
         values.clear();
         vector<T> column(*pInput);
         std::sort(column.begin(), column.end());
         column.erase(std::unique(column.begin(), column.end()), column.end());
         for (unsigned int i = 0; i < column.size(); i++)
         {
            values.push_back(QString::number(column.at(i)).toStdString());
         }
      }
   }
}

void FeatureClass::getFieldValues(const std::string& field, std::vector<std::string>& values)
{
   values.clear();
   if (mpParentElement != NULL)
   {
      DynamicObject* pMetadata = mpParentElement->getMetadata();
      if (pMetadata != NULL)
      {
         pMetadata = pMetadata->getAttribute(FEATURE_ATTRIBUTES_NAME).getPointerToValue<DynamicObject>();
      }
      if (pMetadata != NULL)
      {
         std::vector<std::string> attributeNames;

         DataVariant variant = pMetadata->getAttribute(field);
         if (variant.isValid())
         {
            //the types of double, int, short, float and string are the types
            //that can come out of ArcProxy, since unsigned types are not expected
            //we don't have them in this list
            if (variant.getTypeName() == "vector<double>")
            {
               const vector<double>* pColumn = variant.getPointerToValue<vector<double> >();
               populateFieldValuesFromDynamicObject(pColumn, values);
            }
            else if (variant.getTypeName() == "vector<int>")
            {
               const vector<int>* pColumn = variant.getPointerToValue<vector<int> >();
               populateFieldValuesFromDynamicObject(pColumn, values);
            }
            else if (variant.getTypeName() == "vector<float>")
            {
               const vector<float>* pColumn = variant.getPointerToValue<vector<float> >();
               populateFieldValuesFromDynamicObject(pColumn, values);
            }
            else if (variant.getTypeName() == "vector<string>")
            {
               const vector<string>* pColumn = variant.getPointerToValue<vector<string> >();
               unsigned int size = pColumn->size();
               if (size > 0)
               {
                  std::vector<std::string> column(*pColumn);
                  std::sort(column.begin(), column.end());
                  values.push_back(column.front());
                  for (unsigned int i = 1; i < size; i++)
                  {
                     if (i < size && column[i] != column[i - 1])
                     {
                        values.push_back(column.at(i));
                     }
                  }
               }
            }
            else if (variant.getTypeName() == "vector<short>")
            {
               const vector<short>* pColumn = variant.getPointerToValue<vector<short> >();
               populateFieldValuesFromDynamicObject(pColumn, values);
            }
         }
      }
   }
}

void FeatureClass::populateDisplayQueries(const std::string& queryName, const std::string& field,
                                          bool isUniqueLineColor, bool isUniqueFillColor)
{
   mbUniqueFillColor = isUniqueFillColor;
   mbUniqueLineColor = isUniqueLineColor;
   mUniqueField = field;
   mUniqueQueryName = queryName;

   populateDisplayQueries();
}

void FeatureClass::populateDisplayQueries()
{
   std::vector<std::string> values;
   unsigned int currentOptionCount = 0;
   getFieldValues(mUniqueField, values);
   if (!values.empty())
   {
      std::vector<ColorType> excludedColors;
      std::vector<ColorType> colors;
      excludedColors.push_back(ColorType(0, 0, 0));

      std::vector<DisplayQueryOptions> options = 
         getDisplayQueryOptions(mUniqueQueryName);
      currentOptionCount = options.size();
      for (unsigned int i = 0; i < options.size(); i++)
      {
         if (mbUniqueLineColor)
         {
            excludedColors.push_back(options.at(i).getLineColor());
         }
         if (mbUniqueFillColor)
         {
            excludedColors.push_back(options.at(i).getFillColor());
         }
      }
      for (unsigned int j = 0; j < mQueries.size(); j++)
      {
         if (mQueries.at(j).getQueryName() == mUniqueQueryName)
         {
            if (mbUniqueLineColor)
            {
               excludedColors.push_back(mQueries.at(j).getLineColor());
            }
            if (mbUniqueFillColor)
            {
               excludedColors.push_back(mQueries.at(j).getFillColor());
            }
            ColorType::getUniqueColors(values.size(), colors, excludedColors);
            for (unsigned int i = 0; i < values.size(); i++)
            {
               //traverse list of colors so we can pass them into the get
               //unique color function
               ColorType color = colors[i];
               DisplayQueryOptions* pOption = new DisplayQueryOptions(mQueries.at(j));

               if (mbUniqueLineColor)
               {
                  pOption->setLineColor(color);
               }
               if (mbUniqueFillColor)
               {
                  pOption->setFillColor(color);
               }

               string queryName;
               //make sure we have a unique name when adding
               while (queryName.empty() == true)
               {
                  bool bFound = false;
                  QString tmpQueryName = QString("Query ") + QString::number(getCurrentQueryIndex());
                  for (unsigned int i = 0; i < mDisplayQueries.size(); i++)
                  {
                     if (mDisplayQueries[i]->getUniqueName() == tmpQueryName.toStdString())
                     {
                        incrementCurrentQueryIndex();
                        bFound = true;
                        break;
                     }
                  }
                  if (bFound == false)
                  {
                     queryName = tmpQueryName.toStdString();
                  }
               }

               pOption->setUniqueName(queryName);
               string queryString = "and," + mUniqueField + ",=," + values[i];
               pOption->setQueryString(queryString);
               pOption->setOrder(currentOptionCount + 1);

               mDisplayQueries.push_back(pOption);
               currentOptionCount++;
               incrementCurrentQueryIndex();
            }
         }
      }
      mbUniqueFillColor = false;
      mbUniqueLineColor = false;
      mUniqueQueryName.clear();
      mUniqueField.clear();
   }
}

void FeatureClass::copyQueryGraphicIds(const std::vector<FeatureQueryOptions>& queries)
{
   for (unsigned int i = 0; i < queries.size(); i++)
   {
      for (unsigned int j = 0; j < mQueries.size(); j++)
      {
         if (queries[i].getQueryName() == mQueries[j].getQueryName())
         {
            std::vector<std::string> ids = queries[i].getGraphicObjectIds();
            mQueries[j].clearGraphicObjectIds();
            for (unsigned int k = 0; k < ids.size(); k++)
            {
               mQueries[j].addGraphicObjectId(ids[k]);
            }
         }
      }
   }
}

unsigned int FeatureClass::getCurrentQueryIndex() const
{
   return mCurrentQueryIndex;
}

void FeatureClass::incrementCurrentQueryIndex()
{
   mCurrentQueryIndex++;
}