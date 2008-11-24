/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <float.h>

#include "PlotGroupAdapter.h"
#include "ArrowAdapter.h"
#include "CartesianGridlinesAdapter.h"
#include "CurveAdapter.h"
#include "CurveCollectionAdapter.h"
#include "HistogramAdapter.h"
#include "LocatorAdapter.h"
#include "PlotView.h"
#include "PointAdapter.h"
#include "PointSetAdapter.h"
#include "PolarGridlinesAdapter.h"
#include "PolygonPlotObjectAdapter.h"
#include "RegionObjectAdapter.h"
#include "TextAdapter.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

PlotGroupImp::PlotGroupImp(PlotViewImp* pPlot, bool bPrimary) :
   PlotObjectImp(pPlot, bPrimary)
{
   connect(this, SIGNAL(objectAdded(PlotObject*)), this, SIGNAL(extentsChanged()));
   connect(this, SIGNAL(objectRemoved(PlotObject*)), this, SIGNAL(extentsChanged()));
}

PlotGroupImp::~PlotGroupImp()
{
}

PlotGroupImp& PlotGroupImp::operator= (const PlotGroupImp& object)
{
   if (this != &object)
   {
      PlotObjectImp::operator= (object);

      clear(true);

      vector<PlotObject*>::const_iterator iter = object.mObjects.begin();
      while (iter != object.mObjects.end())
      {
         PlotObject* pObject = NULL;
         pObject = *iter;
         if (pObject != NULL)
         {
            PlotObjectType objectType = pObject->getType();

            PlotObject* pNewObject = NULL;
            pNewObject = addObject(objectType);
            if (pNewObject != NULL)
            {
               if (objectType == ARROW)
               {
                  *(static_cast<ArrowAdapter*> (pNewObject)) = *(static_cast<ArrowAdapter*> (pObject));
               }
               else if (objectType == CARTESIAN_GRIDLINES)
               {
                  *(static_cast<CartesianGridlinesAdapter*>(pNewObject)) =
                     *(static_cast<CartesianGridlinesAdapter*>(pObject));
               }
               else if (objectType == CURVE)
               {
                  *(static_cast<CurveAdapter*> (pNewObject)) = *(static_cast<CurveAdapter*> (pObject));
               }
               else if (objectType == CURVE_COLLECTION)
               {
                  *(static_cast<CurveCollectionAdapter*> (pNewObject)) =
                     *(static_cast<CurveCollectionAdapter*> (pObject));
               }
               else if (objectType == HISTOGRAM)
               {
                  *(static_cast<HistogramAdapter*> (pNewObject)) = *(static_cast<HistogramAdapter*> (pObject));
               }
               else if (objectType == LOCATOR)
               {
                  *(static_cast<LocatorAdapter*> (pNewObject)) = *(static_cast<LocatorAdapter*> (pObject));
               }
               else if (objectType == PLOT_GROUP)
               {
                  *(static_cast<PlotGroupAdapter*> (pNewObject)) = *(static_cast<PlotGroupAdapter*> (pObject));
               }
               else if (objectType == POINT_OBJECT)
               {
                  *(static_cast<PointAdapter*> (pNewObject)) = *(static_cast<PointAdapter*> (pObject));
               }
               else if (objectType == POINT_SET)
               {
                  *(static_cast<PointSetAdapter*> (pNewObject)) = *(static_cast<PointSetAdapter*> (pObject));
               }
               else if (objectType == POLAR_GRIDLINES)
               {
                  *(static_cast<PolarGridlinesAdapter*> (pNewObject)) =
                     *(static_cast<PolarGridlinesAdapter*> (pObject));
               }
               else if (objectType == POLYGON_OBJECT_TYPE)
               {
                  *(static_cast<PolygonPlotObjectAdapter*> (pNewObject)) = *(static_cast<PolygonPlotObjectAdapter*> (pObject));
               }
               else if (objectType == REGION)
               {
                  *(static_cast<RegionObjectAdapter*> (pNewObject)) = *(static_cast<RegionObjectAdapter*> (pObject));
               }
               else if (objectType == TEXT_OBJECT_TYPE)
               {
                  *(static_cast<TextAdapter*> (pNewObject)) = *(static_cast<TextAdapter*> (pObject));
               }
            }
         }

         ++iter;
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

PlotObjectType PlotGroupImp::getType() const
{
   return PLOT_GROUP;
}

void PlotGroupImp::draw()
{
   if (isVisible() == false)
   {
      return;
   }

   vector<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObjectImp* pObject = dynamic_cast<PlotObjectImp*>(*iter);
      if (pObject != NULL)
      {
         pObject->draw();
      }

      ++iter;
   }
}

PlotObject* PlotGroupImp::addObject(const PlotObjectType& eType)
{
   PlotObject* pObject = NULL;

   PlotViewImp* pPlot = getPlot();
   bool bPrimary = isPrimary();

   switch (eType)
   {
      case ARROW:
         pObject = new ArrowAdapter(pPlot, bPrimary);
         break;

      case CURVE:
         pObject = new CurveAdapter(pPlot, bPrimary);
         break;

      case CURVE_COLLECTION:
         pObject = new CurveCollectionAdapter(pPlot, bPrimary);
         break;

      case HISTOGRAM:
         pObject = new HistogramAdapter(pPlot, bPrimary);
         break;

      case LOCATOR:
         pObject = new LocatorAdapter(pPlot, bPrimary);
         break;

      case PLOT_GROUP:
         pObject = new PlotGroupAdapter(pPlot, bPrimary);
         break;

      case POINT_OBJECT:
         pObject = new PointAdapter(pPlot, bPrimary);
         break;

      case POINT_SET:
         pObject = new PointSetAdapter(pPlot, bPrimary);
         break;

      case POLYGON_OBJECT_TYPE:
         pObject = new PolygonPlotObjectAdapter(pPlot, bPrimary);
         break;

      case REGION:
         pObject = new RegionObjectAdapter(pPlot, bPrimary);
         break;

      case TEXT_OBJECT_TYPE:
         pObject = new TextAdapter(pPlot, bPrimary);
         break;

      default:
         break;
   }

   if (pObject != NULL)
   {
      insertObject(pObject);
   }

   return pObject;
}

void PlotGroupImp::insertObjects(const vector<PlotObject*>& objects)
{
   vector<PlotObject*>::const_iterator iter = objects.begin();
   while (iter != objects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         insertObject(pObject);
      }

      ++iter;
   }
}

bool PlotGroupImp::hasObject(PlotObject* pObject) const
{
   if (pObject == NULL)
   {
      return false;
   }

   vector<PlotObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pCurrentObject = NULL;
      pCurrentObject = *iter;
      if (pCurrentObject == pObject)
      {
         return true;
      }

      ++iter;
   }

   return false;
}

const vector<PlotObject*>& PlotGroupImp::getObjects() const
{
   return mObjects;
}

unsigned int PlotGroupImp::getNumObjects() const
{
   return mObjects.size();
}

PlotObject* PlotGroupImp::hitObject(LocationType point) const
{
   vector<PlotObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = *iter;
      if (pObject != NULL)
      {
         PlotObjectImp* pObjectImp = dynamic_cast<PlotObjectImp*>(pObject);
         if (pObjectImp != NULL)
         {
            bool bHit = pObjectImp->hit(point);
            if (bHit == true)
            {
               return pObject;
            }
         }
      }

      ++iter;
   }

   return NULL;
}

bool PlotGroupImp::hit(LocationType point) const
{
   PlotObject* pObject = hitObject(point);
   return (pObject != NULL);
}

bool PlotGroupImp::getExtents(double& dMinX, double& dMinY, double& dMaxX, double& dMaxY)
{
   unsigned int uiObjects = 0;
   uiObjects = mObjects.size();
   if (uiObjects == 0)
   {
      return false;
   }

   dMinX = DBL_MAX;
   dMinY = DBL_MAX;
   dMaxX = -DBL_MAX;
   dMaxY = -DBL_MAX;

   vector<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         double dCurrentMinX = 0.0;
         double dCurrentMinY = 0.0;
         double dCurrentMaxX = 0.0;
         double dCurrentMaxY = 0.0;

         bool bSuccess = false;
         bSuccess = pObject->getExtents(dCurrentMinX, dCurrentMinY, dCurrentMaxX, dCurrentMaxY);
         if (bSuccess == false)
         {
            dMinX = -1.0;
            dMinY = -1.0;
            dMaxX = 1.0;
            dMaxY = 1.0;

            return false;
         }

         if (dCurrentMinX < dMinX)
         {
            dMinX = dCurrentMinX;
         }

         if (dCurrentMinY < dMinY)
         {
            dMinY = dCurrentMinY;
         }

         if (dCurrentMaxX > dMaxX)
         {
            dMaxX = dCurrentMaxX;
         }

         if (dCurrentMaxY > dMaxY)
         {
            dMaxY = dCurrentMaxY;
         }
      }

      ++iter;
   }

   return true;
}

void PlotGroupImp::setVisible(bool bVisible)
{
   if (isVisible() != bVisible)
   {
      vector<PlotObject*>::iterator iter = mObjects.begin();
      while (iter != mObjects.end())
      {
         PlotObject* pObject = NULL;
         pObject = *iter;
         if (pObject != NULL)
         {
            pObject->setVisible(bVisible);
         }

         ++iter;
      }
   }

   PlotObjectImp::setVisible(bVisible);
}

void PlotGroupImp::setSelected(bool bSelect)
{
   if (isSelected() != bSelect)
   {
      vector<PlotObject*>::iterator iter = mObjects.begin();
      while (iter != mObjects.end())
      {
         PlotObject* pObject = NULL;
         pObject = *iter;
         if (pObject != NULL)
         {
            pObject->setSelected(bSelect);
         }

         ++iter;
      }
   }

   PlotObjectImp::setSelected(bSelect);
}

bool PlotGroupImp::insertObject(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   if (hasObject(pObject) == true)
   {
      return false;
   }

   mObjects.push_back(pObject);
   emit objectAdded(pObject);
   notify(SIGNAL_NAME(PlotGroup, ObjectAdded), boost::any(pObject));
   return true;
}

bool PlotGroupImp::removeObject(PlotObject* pObject, bool bDelete)
{
   if (pObject == NULL)
   {
      return false;
   }

   vector<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pCurrentObject = NULL;
      pCurrentObject = *iter;
      if (pCurrentObject == pObject)
      {
         mObjects.erase(iter);
         emit objectRemoved(pObject);

         if (bDelete == true)
         {
            delete dynamic_cast<PlotObjectImp*>(pObject);
         }

         notify(SIGNAL_NAME(Subject, Modified));
         return true;
      }

      ++iter;
   }

   return false;
}

void PlotGroupImp::clear(bool bDelete)
{
   while (mObjects.size() > 0)
   {
      PlotObject* pObject = NULL;
      pObject = mObjects.front();
      if (pObject != NULL)
      {
         removeObject(pObject, bDelete);
      }
   }
}

bool PlotGroupImp::toXml(XMLWriter* pXml) const
{
   if (!PlotObjectImp::toXml(pXml))
   {
      return false;
   }

   for (vector<PlotObject*>::const_iterator it = mObjects.begin(); it != mObjects.end(); ++it)
   {
      PlotObjectImp* pObject = dynamic_cast<PlotObjectImp*>(*it);
      if (pObject != NULL)
      {
         pXml->pushAddPoint(pXml->addElement("PlotObject"));
         pXml->addAttr("type", (*it)->getType());
         if (!pObject->toXml(pXml))
         {
            return false;
         }
      }
   }

   return true;
}

bool PlotGroupImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !PlotObjectImp::fromXml(pDocument, version))
   {
      return false;
   }
   for (DOMNode* pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("PlotObject")))
      {
         PlotObjectType objectType = StringUtilities::fromXmlString<PlotObjectType>(
            A(static_cast<DOMElement*>(pChld)->getAttribute(X("type"))));
         PlotObjectImp* pObject = dynamic_cast<PlotObjectImp*>(addObject(objectType));
         if (pObject == NULL || !pObject->fromXml(pDocument, version))
         {
            return false;
         }
      }
   }
   return true;
}
