/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <float.h>

#include <QtGui/QCursor>
#include <QtGui/QInputDialog>
#include <QtGui/QMouseEvent>

#include "glCommon.h"
#include "PlotViewImp.h"
#include "AnnotationElementAdapter.h"
#include "AnnotationLayerAdapter.h"
#include "AppVerify.h"
#include "ArrowAdapter.h"
#include "CartesianGridlinesAdapter.h"
#include "ConfigurationSettings.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "CurveAdapter.h"
#include "CurveCollectionAdapter.h"
#include "DataDescriptorAdapter.h"
#include "DesktopServices.h"
#include "GraphicGroupImp.h"
#include "HistogramAdapter.h"
#include "LocatorAdapter.h"
#include "MouseModeImp.h"
#include "PlotGroupAdapter.h"
#include "PlotView.h"
#include "PolarGridlinesAdapter.h"
#include "PolygonPlotObjectAdapter.h"
#include "PointAdapter.h"
#include "PointSetAdapter.h"
#include "PropertiesPlotView.h"
#include "RegionObjectAdapter.h"
#include "SecurityMarkingsDlg.h"
#include "SessionManager.h"
#include "TextAdapter.h"
#include "TextObjectImp.h"
#include "Undo.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <xmlreader.h>
#include "DrawUtil.h"

const int DISPLAY_LIST_SIZE = 32;

using namespace std;
using XERCES_CPP_NAMESPACE_QUALIFIER DOMElement;
using XERCES_CPP_NAMESPACE_QUALIFIER XMLString;

PlotViewImp::PlotViewImp(const string& id, const string& viewName, QGLContext* drawContext, QWidget* parent) :
   OrthographicViewImp(id, viewName, drawContext, parent),
   mpAnnotationLayer(NULL),
   mMouseLocator(this, false),
   mSelectionArea(this, false),
   mDisplayListIndex(0),
   mTemporaryDisplayListIndex(0),
   mbTemporaryDisplayList(false),
   mSelectionMode(NORMAL_SELECTION), 
   mSelectionDisplayMode(SYMBOL_SELECTION),
   mEnableShading(false),
   mExtentsMargin(0.0)
{
   Service<DesktopServices> pDesktop;
   string shortcutContext = "Plot";
   list<ContextMenuAction> menuActions;

   // Mouse mode menu
   mpMouseModeMenu = new QMenu("&Mouse Mode", this);
   if (mpMouseModeMenu != NULL)
   {
      string mouseModeContext = shortcutContext + string("/Mouse Mode");

      mpMouseModeGroup = new QActionGroup(mpMouseModeMenu);
      mpMouseModeGroup->setExclusive(true);

      mpObjectSelectAction = mpMouseModeGroup->addAction(QIcon(":/icons/Edit"), "&Object Selection");
      mpObjectSelectAction->setAutoRepeat(false);
      mpObjectSelectAction->setCheckable(true);
      mpObjectSelectAction->setStatusTip("Enables selection of objects within the plot area");
      pDesktop->initializeAction(mpObjectSelectAction, mouseModeContext);

      mpPanAction = mpMouseModeGroup->addAction(QIcon(":/icons/Pan"), "&Pan");
      mpPanAction->setAutoRepeat(false);
      mpPanAction->setCheckable(true);
      mpPanAction->setStatusTip("Enables panning within the plot area");
      pDesktop->initializeAction(mpPanAction, mouseModeContext);

      mpZoomAction = mpMouseModeGroup->addAction(QIcon(":/icons/ZoomRect"), "&Zoom");
      mpZoomAction->setAutoRepeat(false);
      mpZoomAction->setCheckable(true);
      mpZoomAction->setStatusTip("Enables zooming within the plot area");
      pDesktop->initializeAction(mpZoomAction, mouseModeContext);

      mpLocateAction = mpMouseModeGroup->addAction(QIcon(":/icons/DrawPixel"), "&Locate Point");
      mpLocateAction->setAutoRepeat(false);
      mpLocateAction->setCheckable(true);
      mpLocateAction->setStatusTip("Enables display of plot values at a given point");
      pDesktop->initializeAction(mpLocateAction, mouseModeContext);

      mpAnnotateAction = mpMouseModeGroup->addAction(QIcon(":/icons/Annotation"), "&Annotation");
      mpAnnotateAction->setAutoRepeat(false);
      mpAnnotateAction->setCheckable(true);
      mpAnnotateAction->setStatusTip("Adds an annotation object (arrow with text) to the plot at a given point");
      pDesktop->initializeAction(mpAnnotateAction, mouseModeContext);

      mpMouseModeMenu->addActions(mpMouseModeGroup->actions());
      VERIFYNR(connect(mpMouseModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(setMouseMode(QAction*))));
      menuActions.push_back(ContextMenuAction(mpMouseModeMenu->menuAction(), APP_PLOTVIEW_MOUSE_MODE_MENU_ACTION));

      // Separator
      QAction* pSeparatorAction = new QAction(this);
      pSeparatorAction->setSeparator(true);
      menuActions.push_back(ContextMenuAction(pSeparatorAction, APP_PLOTVIEW_MOUSE_MODE_SEPARATOR_ACTION));
   }

   // Rescale axes
   QAction* pRescaleAction = new QAction(QIcon(":/icons/ZoomToFit"), "&Rescale Axes", this);
   pRescaleAction->setAutoRepeat(false);
   pRescaleAction->setStatusTip("Restores the plot axes to the original scale");
   VERIFYNR(connect(pRescaleAction, SIGNAL(triggered()), this, SLOT(zoomExtents())));
   pDesktop->initializeAction(pRescaleAction, shortcutContext);
   menuActions.push_back(ContextMenuAction(pRescaleAction, APP_PLOTVIEW_RESCALE_AXES_ACTION));

   // Separator
   QAction* pSeparator2Action = new QAction(this);
   pSeparator2Action->setSeparator(true);
   menuActions.push_back(ContextMenuAction(pSeparator2Action, APP_PLOTVIEW_RESCALE_AXES_SEPARATOR_ACTION));

   // Security markings
   QAction* pSecurityAction = new QAction("Security Mar&kings...", this);
   pSecurityAction->setAutoRepeat(false);
   pSecurityAction->setStatusTip("Sets the security markings on the plot");
   VERIFYNR(connect(pSecurityAction, SIGNAL(triggered()), this, SLOT(setSecurityMarkings())));
   pDesktop->initializeAction(pSecurityAction, shortcutContext);
   menuActions.push_back(ContextMenuAction(pSecurityAction, APP_PLOTVIEW_SECURITY_MARKINGS_ACTION));

   // Annotation layer
   DataDescriptorAdapter descriptor("Annotation", "AnnotationElement", NULL);
   mpAnnotationLayer = new AnnotationLayerAdapter(SessionItemImp::generateUniqueId(), "Annotation",
      new AnnotationElementAdapter(descriptor, SessionItemImp::generateUniqueId()));
   mpAnnotationLayer->setView(this);
   VERIFYNR(connect(mpAnnotationLayer, SIGNAL(modified()), this, SLOT(refresh())));
   VERIFYNR(connect(this, SIGNAL(displayAreaChanged()), this, SLOT(updateAnnotationObjects())));

   // Initialization
   blockUndo();
   setBackgroundColor(Qt::white);
   setContextMenuActions(menuActions);
   addPropertiesPage(PropertiesPlotView::getName());
   unblockUndo();

   addMouseMode(new MouseModeImp("LocatorMode", QCursor(Qt::CrossCursor), mpLocateAction));
   addMouseMode(new MouseModeImp("PanMode", QCursor(Qt::OpenHandCursor), mpPanAction));
   addMouseMode(new MouseModeImp("SelectionMode", QCursor(Qt::ArrowCursor), mpObjectSelectAction));
   addMouseMode(new MouseModeImp("ZoomBoxMode", QCursor(QPixmap(":/icons/ZoomRectCursor"), 0, 0),
      mpZoomAction));
   addMouseMode(new MouseModeImp("AnnotationMode", mpAnnotationLayer->getMouseCursor(), mpAnnotateAction));

   mMouseLocator.setVisible(false);
   mSelectionArea.setVisible(false);

   // Connections
   VERIFYNR(connect(this, SIGNAL(mouseModeAdded(const MouseMode*)), this,
      SLOT(addMouseModeAction(const MouseMode*))));
   VERIFYNR(connect(this, SIGNAL(mouseModeRemoved(const MouseMode*)), this,
      SLOT(removeMouseModeAction(const MouseMode*))));
   VERIFYNR(connect(this, SIGNAL(mouseModeEnabled(const MouseMode*, bool)), this,
      SLOT(enableMouseModeAction(const MouseMode*, bool))));
   VERIFYNR(connect(this, SIGNAL(mouseModeChanged(const MouseMode*)), this,
      SLOT(updateMouseModeAction(const MouseMode*))));
   VERIFYNR(connect(&mMouseLocator, SIGNAL(locationChanged(const LocationType&)), this, SLOT(refresh())));
}

PlotViewImp::~PlotViewImp()
{
   // Destroy the annotation layer and element
   if (mpAnnotationLayer != NULL)
   {
      // Destroy the element because the layer will not delete it since it does not exist in the model
      DataElementImp* pElement = dynamic_cast<DataElementImp*>(mpAnnotationLayer->getDataElement());

      delete pElement;
      delete mpAnnotationLayer;
   }

   // Destroy the mouse modes
   deleteMouseMode(getMouseMode("LocatorMode"));
   deleteMouseMode(getMouseMode("PanMode"));
   deleteMouseMode(getMouseMode("SelectionMode"));
   deleteMouseMode(getMouseMode("ZoomBoxMode"));
   deleteMouseMode(getMouseMode("AnnotationMode"));

   // Destroy the display lists
   if (mDisplayListIndex != 0)
   {
      GlContextSave contextSave(this);
      glDeleteLists(mDisplayListIndex, DISPLAY_LIST_SIZE);
   }

   for (list<PlotObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); )
   {
      PlotObject* pObject = *iter;
      iter = mObjects.erase(iter);
      if (pObject != NULL)
      {
         emit objectDeleted(pObject);
         notify(SIGNAL_NAME(PlotView, ObjectDeleted), pObject);
         delete dynamic_cast<PlotObjectImp*>(pObject);
      }
   }
}

bool PlotViewImp::canRename() const
{
   return true;
}

const string& PlotViewImp::getObjectType() const
{
   static string type("PlotViewImp");
   return type;
}

bool PlotViewImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlotView"))
   {
      return true;
   }

   return OrthographicViewImp::isKindOf(className);
}

bool PlotViewImp::isKindOfView(const string& className)
{
   if ((className == "PlotViewImp") || (className == "PlotView"))
   {
      return true;
   }

   return OrthographicViewImp::isKindOfView(className);
}

void PlotViewImp::getViewTypes(vector<string>& classList)
{
   classList.push_back("PlotView");
   OrthographicViewImp::getViewTypes(classList);
}

PlotViewImp& PlotViewImp::operator= (const PlotViewImp& plotView)
{
   if (this != &plotView)
   {
      OrthographicViewImp::operator= (plotView);

      for (list<PlotObject*>::iterator iter = mObjects.begin(); iter != mObjects.end(); )
      {
         PlotObject* pObject = *iter;
         if (pObject != NULL)
         {
            iter = mObjects.erase(iter);
            delete dynamic_cast<PlotObjectImp*>(pObject);
         }
      }

      list<PlotObject*>::const_iterator iter = plotView.mObjects.begin();
      while (iter != plotView.mObjects.end())
      {
         PlotObject* pObject = NULL;
         pObject = *iter;
         if (pObject != NULL)
         {
            PlotObjectType objectType = pObject->getType();
            bool bPrimary = pObject->isPrimary();

            PlotObject* pNewObject = NULL;
            pNewObject = addObject(objectType, bPrimary);
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

      updateExtents();

      if ((mpAnnotationLayer != NULL) && (plotView.mpAnnotationLayer != NULL))
      {
         *mpAnnotationLayer = *(plotView.mpAnnotationLayer);
      }

      notify(SIGNAL_NAME(Subject, Modified));
   }

   return *this;
}

ViewType PlotViewImp::getViewType() const
{
   return PLOT_VIEW;
}

PlotObject* PlotViewImp::createObject(const PlotObjectType& objectType, bool bPrimary)
{
   PlotObject* pObject = NULL;

   if (objectType == ARROW)
   {
      pObject = new ArrowAdapter(this, bPrimary);
   }
   else if (objectType == CURVE)
   {
      pObject = new CurveAdapter(this, bPrimary);
   }
   else if (objectType == CURVE_COLLECTION)
   {
      pObject = new CurveCollectionAdapter(this, bPrimary);
   }
   else if (objectType == HISTOGRAM)
   {
      pObject = new HistogramAdapter(this, bPrimary);
   }
   else if (objectType == LOCATOR)
   {
      pObject = new LocatorAdapter(this, bPrimary);
   }
   else if (objectType == REGION)
   {
      pObject = new RegionObjectAdapter(this, bPrimary);
   }
   else if (objectType == POINT_SET)
   {
      pObject = new PointSetAdapter(this, bPrimary);
   }
   else if (objectType == PLOT_GROUP)
   {
      pObject = new PlotGroupAdapter(this, bPrimary);
   }
   else if (objectType == POINT_OBJECT)
   {
      pObject = new PointAdapter(this, bPrimary);
   }
   else if (objectType == POLYGON_OBJECT_TYPE)
   {
      pObject = new PolygonPlotObjectAdapter(this, bPrimary);
   }
   else if (objectType == TEXT_OBJECT_TYPE)
   {
      pObject = new TextAdapter(this, bPrimary);
   }

   return pObject;
}

PlotObject* PlotViewImp::addObject(const PlotObjectType& objectType, bool bPrimary)
{
   PlotObject* pObject = createObject(objectType, bPrimary);
   if (pObject != NULL)
   {
      insertObject(pObject);
   }

   return pObject;
}

bool PlotViewImp::insertObject(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   // Do not insert the object if it is already in the list
   if (containsObject(pObject) == true)
   {
      return false;
   }

   if (pObject->isPrimary())
   {
      PlotObjectImp* pObjectImp = dynamic_cast<PlotObjectImp*>(pObject);
      VERIFY(pObjectImp != NULL);

      VERIFYNR(connect(pObjectImp, SIGNAL(extentsChanged()), this, SLOT(updateExtents())));
      VERIFYNR(connect(pObjectImp, SIGNAL(visibilityChanged(bool)), this, SLOT(updateExtents())));
   }

   mObjects.push_back(pObject);
   emit objectAdded(pObject);
   notify(SIGNAL_NAME(PlotView, ObjectAdded), boost::any(pObject));
   updateExtents();
   return true;
}

list<PlotObject*> PlotViewImp::getObjects() const
{
   return mObjects;
}

list<PlotObject*> PlotViewImp::getObjects(const PlotObjectType& objectType) const
{
   list<PlotObject*> objects;

   list<PlotObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         PlotObjectType currentType = pObject->getType();
         if (currentType == objectType)
         {
            objects.push_back(pObject);
         }
      }

      ++iter;
   }

   return objects;
}

list<PlotObject*> PlotViewImp::getObjectsAt(const QPoint& point) const
{
   list<PlotObject*> objects;

   LocationType worldPoint;
   translateScreenToWorld(point.x(), point.y(), worldPoint.mX, worldPoint.mY);

   list<PlotObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->isPrimary() == true)
         {
            bool bHit = (dynamic_cast<PlotObjectImp*>(pObject))->hit(worldPoint);
            if (bHit == true)
            {
               objects.push_back(pObject);
            }
            else
            {
               PointSet* pPointSet = dynamic_cast<PointSet*> (pObject);
               if (pPointSet != NULL)
               {
                  Point* pPoint = pPointSet->hitPoint(worldPoint);
                  if (pPoint != NULL)
                  {
                     objects.push_back(pPoint);
                  }
               }
            }
         }
      }

      ++iter;
   }

   return objects;
}

bool PlotViewImp::containsObject(PlotObject* pObject) const
{
   if (pObject == NULL)
   {
      return false;
   }

   list<PlotObject*>::const_iterator iter;
   iter = std::find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      return true;
   }

   return false;
}

unsigned int PlotViewImp::getNumObjects() const
{
   return mObjects.size();
}

bool PlotViewImp::deleteObject(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pCurrentObject = NULL;
      pCurrentObject = *iter;
      if (pCurrentObject == pObject)
      {
         mObjects.erase(iter);
         emit objectDeleted(pObject);
         notify(SIGNAL_NAME(PlotView, ObjectDeleted), boost::any(pObject));
         updateExtents();
         delete dynamic_cast<PlotObjectImp*>(pObject);
         return true;
      }

      ++iter;
   }

   return false;
}

bool PlotViewImp::moveObjectToFront(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<PlotObject*>::iterator iter;
   iter = std::find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      mObjects.erase(iter);
      mObjects.push_back(pObject);
      return true;
   }

   return false;
}

bool PlotViewImp::moveObjectToBack(PlotObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<PlotObject*>::iterator iter;
   iter = std::find(mObjects.begin(), mObjects.end(), pObject);
   if (iter != mObjects.end())
   {
      mObjects.erase(iter);
      mObjects.push_front(pObject);
      return true;
   }

   return false;
}

void PlotViewImp::selectObjects(const list<PlotObject*>& objects, bool bSelect)
{
   for (list<PlotObject*>::const_iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      PlotObject* pObject = *iter;
      if (pObject != NULL)
      {
         selectObject(pObject, bSelect);
      }
   }
}

void PlotViewImp::selectObjects(const QRegion& selectionRegion, bool bSelect)
{
   if (selectionRegion.isEmpty() == true)
   {
      return;
   }

   list<PlotObject*> objects;

   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->isPrimary() && pObject->isVisible())
         {
            if (pObject->getType() == POINT_SET)
            {
               selectObjects(selectionRegion, static_cast<PointSet*> (pObject), bSelect);
            }
            else
            {
               // Get the region from the object extents
               QRegion objectRegion = getObjectRegion(pObject);

               // Add the object to the selection list if the object region
               // is contained entirely within the selection region
               if (selectionRegion.intersect(objectRegion) == objectRegion)
               {
                  objects.push_back(pObject);
               }
            }
         }
      }

      ++iter;
   }

   // Select the objects
   selectObjects(objects, bSelect);
}

void PlotViewImp::selectObjects(const QRegion& selectionRegion, const PointSet* pPointSet, bool bSelect)
{
   if ((selectionRegion.isEmpty() == true) || (pPointSet == NULL))
   {
      return;
   }

   vector<Point*> points = pPointSet->getPoints();

   vector<Point*>::iterator iter = points.begin();
   while (iter != points.end())
   {
      Point* pPoint = NULL;
      pPoint = *iter;
      if (pPoint != NULL)
      {
         if (pPoint->isPrimary() && pPoint->isVisible())
         {
            // Get the region from the object extents
            QRegion objectRegion = getObjectRegion(pPoint);

            // Select the object if the object region is contained entirely within the selection region
            if (selectionRegion.intersect(objectRegion) == objectRegion)
            {
               pPoint->setSelected(bSelect);
            }
         }
      }

      ++iter;
   }
}

void PlotViewImp::selectObjects(const QPoint& point, const PointSet* pPointSet,
                                bool bSelect)
{
   VERIFYNRV(pPointSet != NULL);

   double dX = 0.0;
   double dY = 0.0;
   translateScreenToWorld(point.x(), point.y(), dX, dY);

   vector<Point*> points = pPointSet->getPoints();

   for (vector<Point*>::iterator pointIter = points.begin(); pointIter != points.end(); ++pointIter)
   {
      Point* pPoint = *pointIter;
      if (pPoint != NULL)
      {
         if (pPoint->isPrimary() && pPoint->isVisible())
         {
            if (pPoint->hit(LocationType(dX, dY)))
            {
               if (pPoint->isSelected() == true && bSelect == true)
               {
                  bSelect = !bSelect;
               }
               pPoint->setSelected(bSelect);
            }
         }
      }
   }
}

list<PlotObject*> PlotViewImp::getSelectedObjects(bool filterVisible) const
{
   list<PlotObject*> selectedObjects;

   list<PlotObject*>::const_iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
          //----- If the plot object is a pointset check for selected points
         if (pObject->getType() == POINT_SET && mSelectionMode == DEEP_SELECTION)
         {
            PointSet* pPointSet = static_cast<PointSet*>(pObject);

            //----- Get the pointsets points
            vector<Point*> pPoints = pPointSet->getPoints();
            for (vector<Point*>::iterator pointIter = pPoints.begin(); pointIter != pPoints.end(); ++pointIter)
            {
               Point* pPoint = *pointIter;
               if (pPoint->isSelected() && (!filterVisible || pPoint->isVisible()))
               {
                  selectedObjects.push_back(pPoint);
               }
            }
         }
         if (pObject->isSelected() && (!filterVisible || pObject->isVisible()))
         {
            selectedObjects.push_back(pObject);
         }
      }

      ++iter;
   }

   return selectedObjects;
}

unsigned int PlotViewImp::getNumSelectedObjects(bool filterVisible) const
{
   list<PlotObject*> selectedObjects = getSelectedObjects(filterVisible);
   return selectedObjects.size();
}

AnnotationLayer* PlotViewImp::getAnnotationLayer() const
{
   return mpAnnotationLayer;
}

Locator* PlotViewImp::getMouseLocator()
{
   return &mMouseLocator;
}

const Locator* PlotViewImp::getMouseLocator() const
{
   return &mMouseLocator;
}

bool PlotViewImp::selectObject(PlotObject* pObject, bool bSelect)
{
   if (pObject == NULL)
   {
      return false;
   }

   if (containsObject(pObject) == false)
   {
      return false;
   }

   if (pObject->isPrimary() == true)
   {
      if ((pObject->isSelected() != bSelect) || (mSelectionMode == DEEP_SELECTION) ||
         dynamic_cast<PointSet*>(pObject) != NULL)    // In normal selection, a PointSet can still
                                                      // have individual points selected within it
      {
         pObject->setSelected(bSelect);
         emit objectSelected(pObject, bSelect);
         notify(SIGNAL_NAME(PlotView, ObjectSelected), boost::any(pair<PlotObject*, bool>(pObject, bSelect)));
         return true;
      }
   }

   return false;
}

void PlotViewImp::selectObjects(bool bSelect)
{
   selectObjects(mObjects, bSelect);
}

void PlotViewImp::deleteSelectedObjects(bool filterVisible)
{
   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      bool deleteObject = false;

      PlotObject* pObj = *iter;
      PlotObjectImp* pObjImp = dynamic_cast<PlotObjectImp*>(pObj);
      VERIFYNRV(pObjImp != NULL);
      if (pObjImp->isSelected() && (!filterVisible || pObjImp->isVisible()))
      {
         deleteObject = true;
      }
      else
      {
         if (mSelectionMode == DEEP_SELECTION)
         {
            PointSetImp* pSet = dynamic_cast<PointSetImp*>(pObjImp);
            if (pSet != NULL)
            {
               pSet->deleteSelectedPoints(filterVisible);
               if (pSet->getNumPoints() == 0)
               {
                  deleteObject = true;
               }
            }
         }
      }

      list<PlotObject*>::iterator deleteIter = iter;
      ++iter;

      if (deleteObject)
      {
         mObjects.erase(deleteIter);
         emit objectDeleted(pObj);
         notify(SIGNAL_NAME(PlotView, ObjectDeleted), boost::any(pObj));
         delete pObjImp;
      }
   }

   updateExtents();
}

void PlotViewImp::clear()
{
   list<PlotObject*> deleteObjects;

   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->isPrimary() == true)
         {
            deleteObjects.push_back(pObject);
         }
      }

      ++iter;
   }

   iter = deleteObjects.begin();
   while (iter != deleteObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         deleteObject(pObject);
      }

      ++iter;
   }

   zoomExtents();
}

void PlotViewImp::keyPressEvent(QKeyEvent* pEvent)
{
   if (pEvent != NULL)
   {
      if (pEvent->key() == Qt::Key_Delete)
      {
         const MouseMode* pMouseMode = getCurrentMouseMode();
         if (pMouseMode != NULL)
         {
            string mouseMode = "";
            pMouseMode->getName(mouseMode);
            if ((mouseMode == "AnnotationMode") && (mpAnnotationLayer != NULL))
            {
               mpAnnotationLayer->deleteSelectedObjects();
            }
         }
      }
   }

   OrthographicViewImp::keyPressEvent(pEvent);
}

void PlotViewImp::mousePressEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Get the mouse coordinate
   mMouseStart = e->pos();
   mMouseStart.setY(height() - e->pos().y());

   double dStartX = 0;
   double dStartY = 0;
   translateScreenToWorld(mMouseStart.x(), mMouseStart.y(), dStartX, dStartY);

   // Get the mouse mode
   QString strMouseMode;

   const MouseModeImp* pMouseMode = static_cast<const MouseModeImp*>(getCurrentMouseMode());
   if (pMouseMode != NULL)
   {
      strMouseMode = pMouseMode->getName();
   }

   if (strMouseMode == "AnnotationMode")
   {
      if (mpAnnotationLayer != NULL)
      {
         bool bSuccess = mpAnnotationLayer->processMousePress(mMouseStart, e->button(), e->buttons(), e->modifiers());
         if (bSuccess == true)
         {
            updateGL();
         }
      }
   }
   else if (e->button() == Qt::LeftButton)
   {
      if (strMouseMode == "LocatorMode")
      {
         double dDataX = 0.0;
         double dDataY = 0.0;
         translateWorldToData(dStartX, dStartY, dDataX, dDataY);

         mMouseLocator.setLocation(LocationType(dDataX, dDataY));
         mMouseLocator.setVisible(true);

         setCursor(Qt::BlankCursor);
         updateGL();
      }
      else if (strMouseMode == "PanMode")
      {
         setCursor(Qt::ClosedHandCursor);
      }
      else if (strMouseMode == "SelectionMode")
      {
         if ((e->modifiers() != Qt::ShiftModifier) && (e->modifiers() != Qt::ControlModifier))
         {
            selectObjects(false);
         }

         list<PlotObject*> objects = getObjectsAt(mMouseStart);

         list<PlotObject*>::iterator iter = objects.begin();
         while (iter != objects.end())
         {
            PlotObject* pObject = NULL;
            pObject = *iter;
            if (pObject != NULL)
            {
               if ((pObject->getType() == POINT_SET) && 
                  (mSelectionMode == DEEP_SELECTION))
               {
                  selectObjects(mMouseStart, static_cast<PointSet*>(pObject), true);
               }
               else
               {
                  bool bSelected = pObject->isSelected();
                  selectObject(pObject, !bSelected);
               }
            }

            ++iter;
         }

         mSelectionArea.setLineStyle(DASHED);
         mSelectionArea.setVisible(true);

         updateGL();
      }
      else if (strMouseMode == "ZoomBoxMode")
      {
         setSelectionBox(mMouseStart, mMouseStart);
         updateGL();
      }
   }
   else if (e->button() == Qt::MidButton)
   {
      if (strMouseMode == "SelectionMode")
      {
         if ((e->modifiers() != Qt::ShiftModifier) && (e->modifiers() != Qt::ControlModifier))
         {
            selectObjects(false);
         }

         double dDataX = 0.0;
         double dDataY = 0.0;
         translateWorldToData(dStartX, dStartY, dDataX, dDataY);

         mSelectionArea.addPoint(dDataX, dDataY);
         mSelectionArea.setLineStyle(SOLID_LINE);
         mSelectionArea.setVisible(true);

         updateGL();
      }
   }

   mMouseCurrent = mMouseStart;
}

void PlotViewImp::mouseMoveEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }

   // Get the mouse coordinate
   QPoint ptMouse = e->pos();
   ptMouse.setY(height() - e->pos().y());

   double dStartX = 0;
   double dStartY = 0;
   translateScreenToWorld(mMouseStart.x(), mMouseStart.y(), dStartX, dStartY);

   double dX = 0;
   double dY = 0;
   translateScreenToWorld(ptMouse.x(), ptMouse.y(), dX, dY);

   // Get the mouse mode
   QString strMouseMode;

   const MouseModeImp* pMouseMode = static_cast<const MouseModeImp*>(getCurrentMouseMode());
   if (pMouseMode != NULL)
   {
      strMouseMode = pMouseMode->getName();
   }

   if (strMouseMode == "AnnotationMode")
   {
      if (mpAnnotationLayer != NULL)
      {
         bool bSuccess = mpAnnotationLayer->processMouseMove(ptMouse, e->button(), e->buttons(), e->modifiers());
         if (bSuccess == true)
         {
            updateGL();
         }
      }
   }
   else if ((e->buttons() & Qt::LeftButton) != 0)
   {
      if (strMouseMode == "LocatorMode")
      {
         double dDataX = 0.0;
         double dDataY = 0.0;
         translateScreenToData(ptMouse.x(), ptMouse.y(), dDataX, dDataY);

         mMouseLocator.setLocation(LocationType(dDataX, dDataY));
         updateGL();
      }
      else if (strMouseMode == "PanMode")
      {
         Service<DesktopServices> pDesktop;
         if (pDesktop->getPanMode() == PAN_INSTANT)
         {
            ViewImp::pan(ptMouse, mMouseCurrent);
            updateGL();
         }
      }
      else if (strMouseMode == "SelectionMode")
      {
         // Convert from world to data coordinates
         LocationType llCorner;
         LocationType lrCorner;
         LocationType urCorner;
         LocationType ulCorner;

         translateWorldToData(dStartX, dStartY, llCorner.mX, llCorner.mY);
         translateWorldToData(dX, dStartY, lrCorner.mX, lrCorner.mY);
         translateWorldToData(dX, dY, urCorner.mX, urCorner.mY);
         translateWorldToData(dStartX, dY, ulCorner.mX, ulCorner.mY);

         mSelectionArea.clear(true);
         mSelectionArea.addPoint(llCorner.mX, llCorner.mY);
         mSelectionArea.addPoint(lrCorner.mX, lrCorner.mY);
         mSelectionArea.addPoint(urCorner.mX, urCorner.mY);
         mSelectionArea.addPoint(ulCorner.mX, ulCorner.mY);
         mSelectionArea.addPoint(llCorner.mX, llCorner.mY);

         updateGL();
      }
      else if (strMouseMode == "ZoomBoxMode")
      {
         setSelectionBox(mMouseStart, ptMouse);
         updateGL();
      }
   }
   else if ((e->buttons() & Qt::MidButton) != 0)
   {
      if (strMouseMode == "SelectionMode")
      {
         // Convert from world to data coordinates
         double dDeltaX = 0.0;
         double dDeltaY = 0.0;
         translateWorldToData(dX, dY, dDeltaX, dDeltaY);

         mSelectionArea.addPoint(dDeltaX, dDeltaY);
         updateGL();
      }
      else if (strMouseMode == "ZoomBoxMode")
      {
         QPoint diff = mMouseCurrent - ptMouse;
         if (ConfigurationSettings::getSettingAlternateMouseWheelZoom())
         {
            diff = -diff;
         }

         zoomOnPoint(mMouseStart, diff);
         updateGL();
      }
   }

   mMouseCurrent = ptMouse;
}

void PlotViewImp::mouseReleaseEvent(QMouseEvent* e)
{
   if (e == NULL)
   {
      return;
   }
 
   double dStartX = 0;
   double dStartY = 0;
   translateScreenToWorld(mMouseStart.x(), mMouseStart.y(), dStartX, dStartY);

   // Get the mouse coordinate
   QPoint ptMouse = e->pos();
   ptMouse.setY(height() - e->pos().y());

   // Get the mouse mode
   QString strMouseMode;

   const MouseModeImp* pMouseMode = static_cast<const MouseModeImp*>(getCurrentMouseMode());
   if (pMouseMode != NULL)
   {
      strMouseMode = pMouseMode->getName();
   }

   if (strMouseMode == "AnnotationMode")
   {
      if (mpAnnotationLayer != NULL)
      {
         mpAnnotationLayer->processMouseRelease(ptMouse, e->button(), e->buttons(), e->modifiers());
      }
   }
   else if (e->button() == Qt::LeftButton)
   {
      if (strMouseMode == "LocatorMode")
      {
         mMouseLocator.setText(string(), string());
         mMouseLocator.setVisible(false);

         // Restore the mouse cursor
         QCursor mouseCursor(Qt::ArrowCursor);
         if (pMouseMode != NULL)
         {
            mouseCursor = pMouseMode->getCursor();
         }

         setCursor(mouseCursor);
      }
      else if (strMouseMode == "PanMode")
      {
         Service<DesktopServices> pDesktop;
         if (pDesktop->getPanMode() == PAN_DELAY)
         {
            ViewImp::pan(ptMouse, mMouseStart);
         }

         // Restore the cursor
         QCursor mouseCursor(Qt::OpenHandCursor);
         if (pMouseMode != NULL)
         {
            mouseCursor = pMouseMode->getCursor();
         }

         setCursor(mouseCursor);
      }
      else if (strMouseMode == "SelectionMode")
      {
         // Create a rectangle from the selection area
         QRect rcSelection(mMouseStart, ptMouse);

         // Select the objects that are within the selection region
         selectObjects(rcSelection.normalized(), true);

         // Clear the selection area
         mSelectionArea.clear(true);
         mSelectionArea.setVisible(false);
      }
      else if (strMouseMode == "ZoomBoxMode")
      {
         setSelectionBox(QRect());
         ViewImp::zoomToBox(mMouseStart, ptMouse);
      }
   }
   else if (e->button() == Qt::MidButton)
   {
      if (strMouseMode == "SelectionMode")
      {
         // Add the starting point to complete the polygon
         double dDataX = 0.0;
         double dDataY = 0.0;
         translateWorldToData(dStartX, dStartY, dDataX, dDataY);

         // Add the starting point to complete the polygon
         mSelectionArea.addPoint(dDataX, dDataY);

         // Create a polygonal region
         vector<Point*> points = mSelectionArea.getPoints();
         QPolygon regionPoints(points.size());

         for (unsigned int i = 0; i < points.size(); i++)
         {
            Point* pPoint = NULL;
            pPoint = points.at(i);
            if (pPoint != NULL)
            {
               const LocationType& pointLocation = pPoint->getLocation();

               double dScreenX = 0;
               double dScreenY = 0;
               translateDataToScreen(pointLocation.mX, pointLocation.mY, 
                  dScreenX, dScreenY);

               regionPoints.setPoint(i, QPoint(dScreenX, dScreenY));
            }
         }

         // Create a QRegion out of the selection area
         QRegion selectionRegion(regionPoints);

         // Select the objects that are within the selection region
         selectObjects(selectionRegion, true);

         // Clear the selction area
         mSelectionArea.clear(true);
         mSelectionArea.setVisible(false);
      }
   }

   refresh();
}

void PlotViewImp::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
   bool bSuccess = false;
   if (pEvent != NULL)
   {
      // Get the mouse coordinate
      QPoint ptMouse = pEvent->pos();
      ptMouse.setY(height() - pEvent->pos().y());

      // Get the mouse mode
      string mouseMode = "";

      const MouseMode* pMouseMode = getCurrentMouseMode();
      if (pMouseMode != NULL)
      {
         pMouseMode->getName(mouseMode);
      }

      if (mouseMode == "AnnotationMode")
      {
         if (mpAnnotationLayer != NULL)
         {
            bSuccess = mpAnnotationLayer->processMouseDoubleClick(ptMouse, pEvent->button(), pEvent->buttons(),
               pEvent->modifiers());
         }
      }
   }

   if (bSuccess == true)
   {
      pEvent->accept();
      refresh();
   }
   else
   {
      OrthographicViewImp::mouseDoubleClickEvent(pEvent);
   }
}

void PlotViewImp::resizeEvent(QResizeEvent* pEvent)
{
   OrthographicViewImp::resizeEvent(pEvent);
   updateAnnotationObjects();
}

void PlotViewImp::draw()
{
   drawContents();
   drawSelectionBox();
}

void PlotViewImp::drawContents()
{
   setupWorldMatrices();

   // Plot objects
   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObjectImp* pObject = NULL;
      pObject = dynamic_cast<PlotObjectImp*>(*iter);
      if (pObject != NULL)
      {
         pObject->draw();
      }

      ++iter;
   }

   // Gridlines
   drawGridlines();

   // Annotation layer
   if (mpAnnotationLayer != NULL)
   {
      mpAnnotationLayer->draw();
   }

   // Selection area
   mSelectionArea.draw();

   // Mouse locator
   mMouseLocator.draw();
}

void PlotViewImp::setMouseMode(QAction* pAction)
{
   if (pAction == mpObjectSelectAction)
   {
      setMouseMode("SelectionMode");
   }
   else if (pAction == mpPanAction)
   {
      setMouseMode("PanMode");
   }
   else if (pAction == mpZoomAction)
   {
      setMouseMode("ZoomBoxMode");
   }
   else if (pAction == mpLocateAction)
   {
      setMouseMode("LocatorMode");
   }
   else if (pAction == mpAnnotateAction)
   {
      setMouseMode("AnnotationMode");
   }
   else if (pAction == NULL)
   {
      setMouseMode(reinterpret_cast<const MouseMode*>(NULL));
   }
   else
   {
      vector<const MouseMode*> mouseModes = getMouseModes();
      for (vector<const MouseMode*>::iterator iter = mouseModes.begin(); iter != mouseModes.end(); ++iter)
      {
         const MouseMode* pMouseMode = *iter;
         if (pMouseMode != NULL)
         {
            QAction* pCurrentAction = pMouseMode->getAction();
            if (pCurrentAction == pAction)
            {
               string modeName;
               pMouseMode->getName(modeName);
               if (modeName.empty() == false)
               {
                  setMouseMode(QString::fromStdString(modeName));
                  break;
               }
            }
         }
      }
   }
}

void PlotViewImp::addMouseModeAction(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   QAction* pAction = pMouseMode->getAction();
   if (pAction != NULL)
   {
      mpMouseModeMenu->addAction(pAction);
      mpMouseModeGroup->addAction(pAction);
      pAction->setCheckable(true);
   }
}

void PlotViewImp::removeMouseModeAction(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   QAction* pAction = pMouseMode->getAction();
   if (pAction != NULL)
   {
      mpMouseModeMenu->removeAction(pAction);
      mpMouseModeGroup->removeAction(pAction);
   }
}

void PlotViewImp::enableMouseModeAction(const MouseMode* pMouseMode, bool bEnable)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   // If disabling the current mouse mode, reset the current mouse mode
   if (bEnable == false)
   {
      if (pMouseMode == getCurrentMouseMode())
      {
         setMouseMode(reinterpret_cast<const MouseMode*>(NULL));
      }
   }

   // Enable or disable the action
   QAction* pAction = NULL;

   string modeName;
   pMouseMode->getName(modeName);
   if (modeName == "SelectionMode")
   {
      pAction = mpObjectSelectAction;
   }
   else if (modeName == "PanMode")
   {
      pAction = mpPanAction;
   }
   else if (modeName == "ZoomBoxMode")
   {
      pAction = mpZoomAction;
   }
   else if (modeName == "LocatorMode")
   {
      pAction = mpLocateAction;
   }
   else if (modeName == "AnnotationMode")
   {
      pAction = mpAnnotateAction;
   }
   else
   {
      pAction = pMouseMode->getAction();
   }

   if (pAction != NULL)
   {
      if (bEnable == false)
      {
         pAction->setChecked(false);
      }

      pAction->setEnabled(bEnable);
   }
}

void PlotViewImp::updateMouseModeAction(const MouseMode* pMouseMode)
{
   if (pMouseMode == NULL)
   {
      return;
   }

   string modeName;
   pMouseMode->getName(modeName);
   if (modeName == "SelectionMode")
   {
      mpObjectSelectAction->setChecked(true);
   }
   else if (modeName == "PanMode")
   {
      mpPanAction->setChecked(true);
   }
   else if (modeName == "ZoomBoxMode")
   {
      mpZoomAction->setChecked(true);
   }
   else if (modeName == "LocatorMode")
   {
      mpLocateAction->setChecked(true);
   }
   else if (modeName == "AnnotationMode")
   {
      mpAnnotateAction->setChecked(true);
   }
}

void PlotViewImp::updateExtents()
{
   double dMinX = DBL_MAX;
   double dMinY = DBL_MAX;
   double dMaxX = -DBL_MAX;
   double dMaxY = -DBL_MAX;

   bool bExtentsSet = false;

   list<PlotObject*>::iterator iter = mObjects.begin();
   while (iter != mObjects.end())
   {
      PlotObject* pObject = NULL;
      pObject = *iter;
      if (pObject != NULL)
      {
         if ((pObject->isPrimary() == true) && (pObject->isVisible() == true))
         {
            double dCurrentMinX = 0.0;
            double dCurrentMinY = 0.0;
            double dCurrentMaxX = 0.0;
            double dCurrentMaxY = 0.0;

            bool bSuccess = false;
            bSuccess = pObject->getExtents(dCurrentMinX, dCurrentMinY, dCurrentMaxX, dCurrentMaxY);
            if (bSuccess == true)
            {
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

               bExtentsSet = true;
            }
         }
      }

      ++iter;
   }

   if (bExtentsSet == false)
   {
      dMinX = -1.0;
      dMinY = -1.0;
      dMaxX = 1.0;
      dMaxY = 1.0;
   }

   if (dMinX == dMaxX)
   {
      dMinX -= 1.0;
      dMaxX += 1.0;
   }

   if (dMinY == dMaxY)
   {
      dMinY -= 1.0;
      dMaxY += 1.0;
   }

   double dXMargin = (dMaxX - dMinX) * mExtentsMargin;
   double dYMargin = (dMaxY - dMinY) * mExtentsMargin;

   setExtents(dMinX - dXMargin, dMinY - dYMargin, dMaxX + dXMargin, dMaxY + dYMargin);
}

void PlotViewImp::updateAnnotationObjects()
{
   list<GraphicObject*> textObjects;
   mpAnnotationLayer->getObjects(TEXT_OBJECT, textObjects);

   list<GraphicObject*>::iterator iter;
   for (iter = textObjects.begin(); iter != textObjects.end(); ++iter)
   {
      TextObjectImp* pTextObject = dynamic_cast<TextObjectImp*>(*iter);
      if (pTextObject != NULL)
      {
         pTextObject->updateBoundingBox();
      }
   }
}

void PlotViewImp::setSecurityMarkings()
{
   QString strCurrentMarkings = getClassificationText();

   SecurityMarkingsDlg dlg(this, strCurrentMarkings);

   int iReturn = dlg.exec();
   if (iReturn == QDialog::Accepted)
   {
      QString strNewMarkings = dlg.getSecurityMarkings();
      if (strNewMarkings.isEmpty() == false)
      {
         setClassificationText(strNewMarkings);
      }
   }
}

QRegion PlotViewImp::getObjectRegion(const PlotObject* pObject) const
{
   QRegion objectRegion;
   if (pObject != NULL)
   {
      double dMinX = 0.0;
      double dMinY = 0.0;
      double dMaxX = 0.0;
      double dMaxY = 0.0;

      // TODO: Remove the const_cast once the getExtents() method is const
      bool bSuccess = false;
      bSuccess = (const_cast<PlotObject*> (pObject))->getExtents(dMinX, dMinY, dMaxX, dMaxY);
      if (bSuccess == true)
      {
         double dScreenMinX = 0;
         double dScreenMinY = 0;
         translateWorldToScreen(dMinX, dMinY, dScreenMinX, dScreenMinY);

         double dScreenMaxX = 0;
         double dScreenMaxY = 0;
         translateWorldToScreen(dMaxX, dMaxY, dScreenMaxX, dScreenMaxY);

         QRect rcObject(QPoint(dScreenMinX, dScreenMinY), QPoint(dScreenMaxX, dScreenMaxY));
         objectRegion = QRegion(rcObject.normalized());
      }
   }

   return objectRegion;
}

void PlotViewImp::translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const
{
   dataX = worldX;
   dataY = worldY;
}

void PlotViewImp::translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const
{
   worldX = dataX;
   worldY = dataY;
}

void PlotViewImp::translateScreenToData(double screenX, double screenY, double& dataX, double& dataY) const
{
   double worldX = 0.0;
   double worldY = 0.0;
   translateScreenToWorld(screenX, screenY, worldX, worldY);
   translateWorldToData(worldX, worldY, dataX, dataY);
}

void PlotViewImp::translateDataToScreen(double dataX, double dataY, double& screenX, double& screenY) const
{
   double worldX = 0.0;
   double worldY = 0.0;
   translateDataToWorld(dataX, dataY, worldX, worldY);
   translateWorldToScreen(worldX, worldY, screenX, screenY);
}

void PlotViewImp::initializeGL()
{
   ViewImp::initializeGL();

   // unable to share display lists while using renderPixmap method in the QGLWidget class
   // so a temporary display list is created for the openGL context created by renderPixmap
   GLuint displayListIndex = 0;
   if (mbTemporaryDisplayList == true)
   {
      mTemporaryDisplayListIndex = glGenLists(DISPLAY_LIST_SIZE);
      displayListIndex = mTemporaryDisplayListIndex;
   }
   else
   {
      mDisplayListIndex = glGenLists(DISPLAY_LIST_SIZE);
      displayListIndex = mDisplayListIndex;
   }

   // Create the symbol display lists
   double dPixelX1 = -1;
   double dPixelY1 = -1;
   double dPixelX2 = 1;
   double dPixelY2 = 1;

   // SOLID
   glNewList(displayListIndex + Point::SOLID, GL_COMPILE);
   glBegin(GL_QUADS);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // X
   glNewList(displayListIndex + Point::X, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // CROSS_HAIR
   glNewList(displayListIndex + Point::CROSS_HAIR, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // VERTICAL_LINE
   glNewList(displayListIndex + Point::VERTICAL_LINE, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // HORIZONTAL_LINE
   glNewList(displayListIndex + Point::HORIZONTAL_LINE, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // ASTERISK
   glNewList(displayListIndex + Point::ASTERISK, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // FORWARD_SLASH
   glNewList(displayListIndex + Point::FORWARD_SLASH, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glEnd();
   glEndList();

   // BACK_SLASH
   glNewList(displayListIndex + Point::BACK_SLASH, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // BOXED_X
   glNewList(displayListIndex + Point::BOXED_X, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);

   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // BOXED_CROSS_HAIR
   glNewList(displayListIndex + Point::BOXED_CROSS_HAIR, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);

   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // BOXED_VERTICAL_LINE
   glNewList(displayListIndex + Point::BOXED_VERTICAL_LINE, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);

   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // BOXED_HORIZONTAL_LINE
   glNewList(displayListIndex + Point::BOXED_HORIZONTAL_LINE, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);

   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // BOXED_ASTERISK
   glNewList(displayListIndex + Point::BOXED_ASTERISK, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);

   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // BOXED_FORWARD_SLASH
   glNewList(displayListIndex + Point::BOXED_FORWARD_SLASH, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);
   glEnd();
   glEndList();

   // BOXED_BACK_SLASH
   glNewList(displayListIndex + Point::BOXED_BACK_SLASH, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX1, dPixelY2);
   glVertex2d(dPixelX2, dPixelY1);
   glEnd();
   glEndList();

   // BOX
   glNewList(displayListIndex + Point::BOX, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // DIAMOND
   glNewList(displayListIndex + Point::DIAMOND, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // DIAMOND_FILLED
   glNewList(displayListIndex + Point::DIAMOND_FILLED, GL_COMPILE);
   glBegin(GL_QUADS);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // DIAMOND_CROSS_HAIR
   glNewList(displayListIndex + Point::DIAMOND_CROSS_HAIR, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);

   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();

   // TRIANGLE
   glNewList(displayListIndex + Point::TRIANGLE, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glEnd();
   glEndList();

   // TRIANGLE_FILLED
   glNewList(displayListIndex + Point::TRIANGLE_FILLED, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, dPixelY1);
   glEnd();
   glEndList();

   // RIGHT_TRIANGLE
   glNewList(displayListIndex + Point::RIGHT_TRIANGLE, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // RIGHT_TRIANGLE_FILLED
   glNewList(displayListIndex + Point::RIGHT_TRIANGLE_FILLED, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   glVertex2d(dPixelX1, dPixelY1);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // LEFT_TRIANGLE
   glNewList(displayListIndex + Point::LEFT_TRIANGLE, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // LEFT_TRIANGLE_FILLED
   glNewList(displayListIndex + Point::LEFT_TRIANGLE_FILLED, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   glVertex2d(dPixelX2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glEnd();
   glEndList();

   // DOWN_TRIANGLE
   glNewList(displayListIndex + Point::DOWN_TRIANGLE, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // DOWN_TRIANGLE_FILLED
   glNewList(displayListIndex + Point::DOWN_TRIANGLE_FILLED, GL_COMPILE);
   glBegin(GL_TRIANGLES);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d(dPixelX2, dPixelY2);
   glVertex2d(dPixelX1, dPixelY2);
   glEnd();
   glEndList();

   // CIRCLE
   glNewList(displayListIndex + Point::CIRCLE, GL_COMPILE);
   DrawUtil::drawEllipse(LocationType(dPixelX1, dPixelY2), LocationType(dPixelX2, dPixelY1), false);
   glEndList();

   // CIRCLE_FILLED
   glNewList(displayListIndex + Point::CIRCLE_FILLED, GL_COMPILE);
   DrawUtil::drawEllipse(LocationType(dPixelX1, dPixelY2), LocationType(dPixelX2, dPixelY1), true);
   glEndList();

   // OCTAGON
   glNewList(displayListIndex + Point::OCTAGON, GL_COMPILE);
   glBegin(GL_LINE_LOOP);
   glVertex2f( dPixelX1 / 2, dPixelY1);
   glVertex2f( dPixelX2 / 2, dPixelY1);
   glVertex2f( dPixelX2, dPixelY1 / 2);
   glVertex2f( dPixelX2, dPixelY2 / 2);
   glVertex2f( dPixelX2 / 2, dPixelY2);
   glVertex2f( dPixelX1 / 2, dPixelY2);
   glVertex2f( dPixelX1, dPixelY2 / 2);
   glVertex2f( dPixelX1, dPixelY1 / 2);
   glEnd();
   glEndList();
   
   // OCTAGON_FILLED
   glNewList(displayListIndex + Point::OCTAGON_FILLED, GL_COMPILE);
   glBegin(GL_POLYGON);
   glVertex2f( dPixelX1 / 2, dPixelY1);
   glVertex2f( dPixelX2 / 2, dPixelY1);
   glVertex2f( dPixelX2, dPixelY1 / 2);
   glVertex2f( dPixelX2, dPixelY2 / 2);
   glVertex2f( dPixelX2 / 2, dPixelY2);
   glVertex2f( dPixelX1 / 2, dPixelY2);
   glVertex2f( dPixelX1, dPixelY2 / 2);
   glVertex2f( dPixelX1, dPixelY1 / 2);
   glEnd();
   glEndList();
   
   // OCTAGON_CROSS_HAIR
   glNewList(displayListIndex + Point::OCTAGON_CROSS_HAIR, GL_COMPILE);
   glBegin(GL_LINES);
   glVertex2f( dPixelX1 / 2, dPixelY1);
   glVertex2f( dPixelX2 / 2, dPixelY1);
   glVertex2f( dPixelX2 / 2, dPixelY1);
   glVertex2f( dPixelX2, dPixelY1 / 2);
   glVertex2f( dPixelX2, dPixelY1 / 2);
   glVertex2f( dPixelX2, dPixelY2 / 2);
   glVertex2f( dPixelX2, dPixelY2 / 2);
   glVertex2f( dPixelX2 / 2, dPixelY2);
   glVertex2f( dPixelX2 / 2, dPixelY2);
   glVertex2f( dPixelX1 / 2, dPixelY2);
   glVertex2f( dPixelX1 / 2, dPixelY2);
   glVertex2f( dPixelX1, dPixelY2 / 2);
   glVertex2f( dPixelX1, dPixelY2 / 2);
   glVertex2f( dPixelX1, dPixelY1 / 2);
   glVertex2f( dPixelX1, dPixelY1 / 2);
   glVertex2f( dPixelX1 / 2, dPixelY1);

   glVertex2d(dPixelX1, (dPixelY1 + dPixelY2) / 2);
   glVertex2d(dPixelX2, (dPixelY1 + dPixelY2) / 2);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY1);
   glVertex2d((dPixelX1 + dPixelX2) / 2, dPixelY2);
   glEnd();
   glEndList();
}

void PlotViewImp::useTemporaryDisplayList(bool bTempList)
{
   mbTemporaryDisplayList = bTempList;
}

GLuint PlotViewImp::getDisplayListIndex() const
{
   if (mbTemporaryDisplayList == true)
   {
      return mTemporaryDisplayListIndex;
   }

   return mDisplayListIndex;
}

void PlotViewImp::setSelectionMode(PlotSelectionModeType mode)
{
   mSelectionMode = mode;
}

PlotSelectionModeType PlotViewImp::getSelectionMode() const
{
   return mSelectionMode;
}

void PlotViewImp::setSelectionDisplayMode(PointSelectionDisplayType mode)
{
   mSelectionDisplayMode = mode;
}

PointSelectionDisplayType PlotViewImp::getSelectionDisplayMode() const
{
   return mSelectionDisplayMode;
}

void PlotViewImp::setEnableShading(bool shading)
{
   mEnableShading = shading;
}

bool PlotViewImp::isShadingEnabled() const
{
   return mEnableShading;
}

void PlotViewImp::setExtentsMargin(double marginFactor)
{
   if (marginFactor < 0.0)
   {
      marginFactor = 0.0;
   }

   if (marginFactor != mExtentsMargin)
   {
      mExtentsMargin = marginFactor;
      updateExtents();
   }
}

double PlotViewImp::getExtentsMargin() const
{
   return mExtentsMargin;
}

bool PlotViewImp::toXml(XMLWriter* pXml) const
{
   if (!OrthographicViewImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("displayListIndex", mDisplayListIndex);
   pXml->addAttr("tempDisplayListIndex", mTemporaryDisplayListIndex);
   pXml->addAttr("tempDisplayList", mbTemporaryDisplayList);
   pXml->addAttr("plotSelectionMode", static_cast<int>(mSelectionMode));
   if (mpAnnotationLayer != NULL)
   {
      pXml->addAttr("annotationLayerId", mpAnnotationLayer->getId());
   }
   
   if (mObjects.size() > 0 && !isKindOf("HistogramPlot"))
   {
      pXml->pushAddPoint(pXml->addElement("PlotObjects"));
      list<PlotObject*>::const_iterator it;
      for (it = mObjects.begin(); it != mObjects.end(); ++it)
      {
         const PlotObjectImp* pObject = dynamic_cast<PlotObjectImp*>(*it);
         if (pObject != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("PlotObject"));
            bool success = pObject->toXml(pXml);
            DOMNode* pNode = pXml->popAddPoint();

            if (success == false)
            {
               if (pNode != NULL)
               {
                  pXml->removeChild(pNode);
               }
            }
         }
      }
      pXml->popAddPoint();
   }

   return true;
}

bool PlotViewImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (!OrthographicViewImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   if (pElem == NULL)
   {
      return false;
   }

   mDisplayListIndex = StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("displayListIndex"))));
   mTemporaryDisplayListIndex = StringUtilities::fromXmlString<int>(
      A(pElem->getAttribute(X("tempDisplayListIndex"))));
   mbTemporaryDisplayList = StringUtilities::fromXmlString<bool>(
      A(pElem->getAttribute(X("tempDisplayList"))));

   if (pElem->hasAttribute(X("annotationLayerId")))
   {
      AnnotationLayerAdapter* pAnnoLayer = dynamic_cast<AnnotationLayerAdapter*>(
         Service<SessionManager>()->getSessionItem(A(pElem->getAttribute(X("annotationLayerId")))));
      if (pAnnoLayer != NULL)
      {
         VERIFY(mpAnnotationLayer != NULL);
         GraphicGroupImp* pCurGroup = dynamic_cast<GraphicGroupImp*>(mpAnnotationLayer->getGroup());  
         GraphicGroup* pRestorGroup = pAnnoLayer->getGroup();
         if (pCurGroup != NULL && pRestorGroup != NULL)
         {
            pCurGroup->replicateObject(pRestorGroup);
         }
         delete pAnnoLayer;
      }
   }

   for (DOMNode* pChld = pDocument->getFirstChild();
      pChld != NULL;
      pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("PlotObjects")))
      {
         for (DOMNode* pGChld = pChld->getFirstChild();
            pGChld != NULL;
            pGChld = pGChld->getNextSibling())
         {
            if (XMLString::equals(pGChld->getNodeName(), X("PlotObject")))
            {
               pElem = static_cast<DOMElement*>(pGChld);
               string typeStr = A(pElem->getAttribute(X("type")));
               PlotObjectType eType = StringUtilities::fromXmlString<PlotObjectType>(typeStr);
               bool isPrimary = StringUtilities::fromXmlString<bool>(
                  A(pElem->getAttribute(X("primary"))));
               PlotObject* pObject = addObject(eType, isPrimary);
               PlotObjectImp* pImp = dynamic_cast<PlotObjectImp*>(pObject);
               if (pImp != NULL)
               {
                  if (!pImp->fromXml(pGChld, version))
                  {
                     return false;
                  }
               }
            }
         }
      }
   }

   refresh();

   return true;
}
