/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#if defined(WIN_API)
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

#include "AnnotationLayerImp.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DataElement.h"
#include "DesktopServicesImp.h"
#include "DirectionalArrowObjectImp.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "GraphicElement.h"
#include "GraphicElementImp.h"
#include "GraphicLayerAdapter.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "GraphicGroup.h"
#include "GraphicGroupImp.h"
#include "GuiFunctors.h"
#include "LatLonInsertObject.h"
#include "MultiLineTextDialog.h"
#include "OrthographicView.h"
#include "PlotWidgetAdapter.h"
#include "PlotWindow.h"
#include "PlotWindowImp.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "PolylineObjectImp.h"
#include "ScaleBarObject.h"
#include "SymbolManager.h"
#include "Undo.h"
#include "View.h"
#include "ViewImp.h"
#include "WidgetImageObjectImp.h"

#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtOpenGL/QGLWidget>

#include <algorithm>
#include <functional>
#include <list>
#include <math.h>
#include <vector>
#include <boost/bind.hpp>
using namespace std;

XERCES_CPP_NAMESPACE_USE

GraphicLayerImp::GraphicLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mpExplorer(Service<SessionExplorer>().get(), SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &GraphicLayerImp::updateContextMenu)),
   mbHideSelectionBox(false),
   mShowLabels(false),
   mLayerLocked(false),
   mProcessingMouseEvent(false),
   mGroupHasLayerSet(false),
   mpInsertingObject(NULL),
   mCurrentType(MOVE_OBJECT),
   mpUndoLock(NULL),
   mHandleSize(4.0)
{
   addAcceptableGraphicType(MOVE_OBJECT);
   addAcceptableGraphicType(ROTATE_OBJECT);

   GraphicElementImp* pGraphicElement = dynamic_cast<GraphicElementImp*>(pElement);
   if (pGraphicElement != NULL)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(pGraphicElement->getGroup());
      if (pGroup != NULL)
      {
         VERIFYNR(connect(pGroup, SIGNAL(abortedAdd(GraphicObject*)), this, SLOT(cleanUpBadObject(GraphicObject*))));
         VERIFYNR(connect(pGroup, SIGNAL(extentsModified()), this, SIGNAL(extentsModified())));
         VERIFYNR(connect(pGroup, SIGNAL(modified()), this, SIGNAL(modified())));
      }
   }
}

GraphicLayerImp::~GraphicLayerImp()
{
   completeInsertion();
}

const string& GraphicLayerImp::getObjectType() const
{
   static string sType("GraphicLayerImp");
   return sType;
}

bool GraphicLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GraphicLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

bool GraphicLayerImp::isKindOfLayer(const string& className)
{
   if ((className == "GraphicLayerImp") || (className == "GraphicLayer"))
   {
      return true;
   }

   return LayerImp::isKindOfLayer(className);
}

void GraphicLayerImp::getLayerTypes(vector<string>& classList)
{
   classList.push_back("GraphicLayer");
   LayerImp::getLayerTypes(classList);
}

GraphicLayerImp& GraphicLayerImp::operator= (const GraphicLayerImp& graphicLayer)
{
   if (this != &graphicLayer)
   {
      LayerImp::operator =(graphicLayer);
      getGroup();    // Sets the layer into the new objects

      mHandleSize = graphicLayer.mHandleSize;
      mbHideSelectionBox = graphicLayer.mbHideSelectionBox;
      mShowLabels = graphicLayer.mShowLabels;

      emit modified();
   }

   return *this;
}

LayerType GraphicLayerImp::getLayerType() const
{
   return GRAPHIC_LAYER;
}

vector<ColorType> GraphicLayerImp::getColors() const
{
   vector<ColorType> colors;

   list<GraphicObject*> allObjects = getObjects();
   for_each(allObjects.begin(), allObjects.end(), GetAnnoObjectColors(colors));
   colors.push_back(ColorType(255, 255, 255));

   return colors;
}

void GraphicLayerImp::drawGroup()
{
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getView());
   double zoomPercent = 100;
   if (pView != NULL)
   {
      zoomPercent = pView->getZoomPercentage();
   }

   GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
   if (pGroup != NULL)
   {
      pGroup->draw(zoomPercent/100);
   }
}

void GraphicLayerImp::draw()
{
   vector<LocationType> selectionNodes;
   bool bSuccess = true;

   GraphicElement* pElement = dynamic_cast<GraphicElement*>(getDataElement());
   if (pElement != NULL)
   {
      if (!pElement->getInteractive())
      {
         return;
      }
   }

   UndoLock lock(getView());
   drawGroup();

   LocationType p1(1e30, 1e30);
   LocationType p2(-1e30, -1e30);
   LocationType p3;
   bool handlesFound = false;
   unsigned int i = 0;

   for (list<GraphicObject*>::iterator it = mSelectedObjects.begin(); bSuccess && it != mSelectedObjects.end(); ++it)
   {
      GraphicObject* pObj = *it;
      GraphicObjectImp* pImpl = dynamic_cast<GraphicObjectImp*>(pObj);
      VERIFYNRV(pImpl != NULL);

      selectionNodes = pImpl->getHandles();
      for (i = 0; bSuccess && i < selectionNodes.size(); ++i)
      {
         if (selectionNodes[i].mX < p1.mX)
         {
            p1.mX = selectionNodes[i].mX;
         }

         if (selectionNodes[i].mX > p2.mX)
         {
            p2.mX = selectionNodes[i].mX;
         }

         if (selectionNodes[i].mY < p1.mY)
         {
            p1.mY = selectionNodes[i].mY;
         }

         if (selectionNodes[i].mY > p2.mY)
         {
            p2.mY = selectionNodes[i].mY;
         }

         handlesFound = true;
      }
   }

   if (handlesFound && bSuccess && !mbHideSelectionBox)
   {
      for (list<GraphicObject*>::iterator it = mSelectedObjects.begin(); it != mSelectedObjects.end(); ++it)
      {
         GraphicObjectImp* pObj = dynamic_cast<GraphicObjectImp*>(*it);
         pObj->rotateViewMatrix();

         selectionNodes = pObj->getHandles();
         drawSelectionRectangle (pObj->getLlCorner(), pObj->getUrCorner());
         bool hasCornerHandles = pObj->hasCornerHandles();
         for (i = 0; i < selectionNodes.size(); ++i)
         {
            bool bSelectionHandle = false;
            if (i > 7 || !hasCornerHandles)
            {
               bSelectionHandle = true;
            }

            drawHandle(selectionNodes[i], bSelectionHandle);
         }

         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();
      }
   }
}

bool GraphicLayerImp::isVisibleObjectType(GraphicObjectType eType) const
{
   return !(MOVE_OBJECT == eType || ROTATE_OBJECT == eType);
}

GraphicObject* GraphicLayerImp::addObject(const GraphicObjectType& objectType, LocationType point)
{
   if (getGroup() == NULL)
   {
      return NULL;
   }

   GlContextSave contextSave(dynamic_cast<QGLWidget*>(getView()));
   GraphicObject* pObject = getGroup()->addObject(objectType, point);
   if (pObject == NULL)
   {
      // attempted to create an object which cannot be placed in the layer
      QString message = "This graphic object cannot be placed into this layer.";
      QMessageBox::critical(NULL, "Graphic Layer", message);
      return NULL;
   }

   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));

   return pObject;
}

bool GraphicLayerImp::removeObject(GraphicObject* pObject, bool bDelete)
{
   if (pObject == NULL)
   {
      return false;
   }

   GlContextSave contextSave(dynamic_cast<QGLWidget*>(getView()));
   bool bSelected = isObjectSelected(pObject);
   if (bSelected == true)
   {
      deselectObject(pObject);
   }
   GraphicObjectImp* pObjImp = dynamic_cast<GraphicObjectImp*>(pObject);
   if (pObjImp == mpInsertingObject)
   {
      mpInsertingObject = NULL;
   }

   bool bSuccess = getGroup()->removeObject(pObject, bDelete);
   if (bSuccess == true)
   {
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return bSuccess;
}

bool GraphicLayerImp::hasObject(GraphicObject* pObject) const
{
   bool bContains = false;
   if (pObject != NULL)
   {
      bContains = getGroup()->hasObject(pObject);
   }

   return bContains;
}

list<GraphicObject*> GraphicLayerImp::getObjects() const
{
   GraphicGroup* pGroup = getGroup();
   if (pGroup == NULL)
   {
      return list<GraphicObject*>();
   }
   return pGroup->getObjects();
}

list<GraphicObject*> GraphicLayerImp::getObjects(const GraphicObjectType& objectType) const
{
   list<GraphicObject*> objects;

   const list<GraphicObject*>& allObjects = getObjects();

   list<GraphicObject*>::const_iterator iter = allObjects.begin();
   while (iter != allObjects.end())
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->getGraphicObjectType() == objectType)
         {
            objects.push_back(pObject);
         }
      }

      ++iter;
   }

   return objects;
}

unsigned int GraphicLayerImp::getNumObjects() const
{
   const list<GraphicObject*>& objects = getObjects();
   return objects.size();
}

unsigned int GraphicLayerImp::getNumObjects(const GraphicObjectType& objectType) const
{
   list<GraphicObject*> objects = getObjects(objectType);
   return objects.size();
}

GraphicObject* GraphicLayerImp::getObjectByName(const string& name) const
{
   const list<GraphicObject*>& allObjects = getObjects();

   list<GraphicObject*>::const_iterator iter;
   for (iter = allObjects.begin(); iter != allObjects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         string currentName = pObject->getName();
         if (currentName == name)
         {
            return pObject;
         }
      }
   }

   return NULL;
}

bool GraphicLayerImp::selectObject(GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::reverse_iterator it = find(mSelectedObjects.rbegin(), mSelectedObjects.rend(), pObject);
   if (it == mSelectedObjects.rend())
   {
      mSelectedObjects.push_back(pObject);
      emit objectsSelected(mSelectedObjects);
      emit modified();
      notify(SIGNAL_NAME(GraphicLayer, ObjectsSelected), boost::any(mSelectedObjects));
      return true;
   }

   return false;
}

void GraphicLayerImp::selectAllObjects()
{
   list<GraphicObject*> objects = getObjects();
   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         selectObject(pObject);
      }
   }
}

bool GraphicLayerImp::isObjectSelected(GraphicObject* pObject) const
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::const_iterator it = find(mSelectedObjects.begin(), mSelectedObjects.end(), pObject);
   if (it != mSelectedObjects.end())
   {
      return true;
   }

   return false;
}

void GraphicLayerImp::getSelectedObjects(list<GraphicObject*>& selectedObjects) const
{
   getSelectedObjectsImpl(selectedObjects);
}

void GraphicLayerImp::getSelectedObjectsImpl(list<GraphicObject*>& selectedObjects) const
{
   selectedObjects.clear();
   selectedObjects = mSelectedObjects;
}

void GraphicLayerImp::getSelectedObjects(const GraphicObjectType& objectType,
                                         list<GraphicObject*>& selectedObjects) const
{
   getSelectedObjectsImpl(objectType, selectedObjects);
}

void GraphicLayerImp::getSelectedObjectsImpl(const GraphicObjectType& objectType,
                                             list<GraphicObject*>& selectedObjects) const
{
   selectedObjects.clear();

   list<GraphicObject*>::const_iterator iter = mSelectedObjects.begin();
   while (iter != mSelectedObjects.end())
   {
      GraphicObject* pObject = *iter;
      if (pObject != NULL)
      {
         if (pObject->getGraphicObjectType() == objectType)
         {
            selectedObjects.push_back(pObject);
         }
      }

      ++iter;
   }
}

unsigned int GraphicLayerImp::getNumSelectedObjects() const
{
   list<GraphicObject*> selectedObjects;
   getSelectedObjects(selectedObjects);

   return selectedObjects.size();
}

unsigned int GraphicLayerImp::getNumSelectedObjectsImpl() const
{
   list<GraphicObject*> selectedObjects;
   getSelectedObjectsImpl(selectedObjects);

   return selectedObjects.size();
}

unsigned int GraphicLayerImp::getNumSelectedObjects(const GraphicObjectType& objectType) const
{
   list<GraphicObject*> selectedObjects;
   getSelectedObjects(objectType, selectedObjects);

   return selectedObjects.size();
}

unsigned int GraphicLayerImp::getNumSelectedObjectsImpl(const GraphicObjectType& objectType) const
{
   list<GraphicObject*> selectedObjects;
   getSelectedObjectsImpl(objectType, selectedObjects);

   return selectedObjects.size();
}

bool GraphicLayerImp::deselectObject(GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   list<GraphicObject*>::iterator it = find(mSelectedObjects.begin(), mSelectedObjects.end(), pObject);
   if (it != mSelectedObjects.end())
   {
      mSelectedObjects.erase(it);
      emit objectsSelected(mSelectedObjects);
      emit modified();
      notify(SIGNAL_NAME(GraphicLayer, ObjectsSelected), boost::any(mSelectedObjects));
      return true;
   }

   return false;
}

void GraphicLayerImp::deselectAllObjects()
{
   if (getNumSelectedObjects() > 0)
   {
      mSelectedObjects.clear();
      emit objectsSelected(mSelectedObjects);
      emit modified();
      notify(SIGNAL_NAME(GraphicLayer, ObjectsSelected), boost::any(mSelectedObjects));
   }
}

void GraphicLayerImp::deleteSelectedObjects()
{
   if (!mLayerLocked)
   {
      list<GraphicObject*> objects = getObjects();

      UndoGroup group(getView(), "Delete Selected Objects");

      for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject == NULL)
         {
            continue;
         }

         bool bSelected = isObjectSelected(pObject);
         if (bSelected == true)
         {
            removeObject(pObject, true);
         }
      }

      View* pView = getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

void GraphicLayerImp::clear()
{
   if (!mLayerLocked)
   {
      UndoGroup group(getView(), "Clear Objects");
      list<GraphicObject*> objects = getObjects();
      for (list<GraphicObject*>::iterator oit = objects.begin(); oit != objects.end(); ++oit)
      {
         if (*oit != NULL)
         {
            removeObject(*oit, true);
         }
      }

      View* pView = getView();
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}

bool GraphicLayerImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   if (!LayerImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("showLabels", mShowLabels);
   pXml->addAttr("layerLocked", mLayerLocked);

   return true;
}

bool GraphicLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || !LayerImp::fromXml(pDocument, version))
   {
      return false;
   }
   DOMElement* pElement = static_cast<DOMElement*> (pDocument);
   mShowLabels = StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("showLabels"))));
   setLayerLocked(StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("layerLocked")))));
   return true;
}

void GraphicLayerImp::drawSelectionRectangle(LocationType ll, LocationType ur) const
{
   glEnable(GL_LINE_STIPPLE);

   glLineWidth(1);

   glLineStipple(1, 0x7070);
   glColor3ub(255, 255, 255);
   glBegin(GL_LINE_LOOP);
   glVertex2f(ll.mX, ll.mY);
   glVertex2f(ll.mX, ur.mY);
   glVertex2f(ur.mX, ur.mY);
   glVertex2f(ur.mX, ll.mY);
   glEnd();

   glLineStipple(1, 0x0707);
   glColor3ub(0, 0, 0);
   glBegin(GL_LINE_LOOP);
   glVertex2f(ll.mX, ll.mY);
   glVertex2f(ll.mX, ur.mY);
   glVertex2f(ur.mX, ur.mY);
   glVertex2f(ur.mX, ll.mY);
   glEnd();

   glDisable(GL_LINE_STIPPLE);
}

void GraphicLayerImp::drawHandle(LocationType point, bool bSelectionHandle)
{
   double x1 = 0.0;
   double x2 = 0.0;
   double y1 = 0.0;
   double y2 = 0.0;

   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(getView());
   if (pView != NULL)
   {
      double zoomFactor = 100.0 / pView->getZoomPercentage();
      double xScale = zoomFactor / getXScaleFactor();
      double yScale = zoomFactor / getYScaleFactor();

      x1 = point.mX - (mHandleSize * xScale);
      x2 = point.mX + (mHandleSize * xScale);
      y1 = point.mY - (mHandleSize * yScale);
      y2 = point.mY + (mHandleSize * yScale);
   }

   OrthographicView* pOrthoView = dynamic_cast<OrthographicView*>(getView());
   if (pOrthoView != NULL)
   {
      double dScreenX = 0.0;
      double dScreenY = 0.0;
      translateDataToScreen(point.mX, point.mY, dScreenX, dScreenY);

      translateScreenToData(dScreenX - mHandleSize, dScreenY - mHandleSize, x1, y1);
      translateScreenToData(dScreenX + mHandleSize, dScreenY + mHandleSize, x2, y2);
   }

   if (bSelectionHandle == true)
   {
      glColor3ub (255, 255, 0);
   }
   else
   {
      glColor3ub (255, 255, 255);
   }

   glLineWidth (1);

   glBegin (GL_POLYGON);
   if (bSelectionHandle == true)
   {
      glVertex2d(x1, (y1 + y2) / 2);
      glVertex2d((x1 + x2) / 2, y1);
      glVertex2d(x2, (y1 + y2) / 2);
      glVertex2d((x1 + x2) / 2, y2);
   }
   else
   {
      glVertex2d(x1, y1);
      glVertex2d(x2, y1);
      glVertex2d(x2, y2);
      glVertex2d(x1, y2);
   }

   glEnd ();

   glColor3ub (0, 0, 0);

   glBegin (GL_LINE_LOOP);
   if (bSelectionHandle == true)
   {
      glVertex2d(x1, (y1 + y2) / 2);
      glVertex2d((x1 + x2) / 2, y1);
      glVertex2d(x2, (y1 + y2) / 2);
      glVertex2d((x1 + x2) / 2, y2);
   }
   else
   {
      glVertex2d(x1, y1);
      glVertex2d(x2, y1);
      glVertex2d(x2, y2);
      glVertex2d(x1, y2);
   }

   glEnd ();
}

bool GraphicLayerImp::acceptsMouseEvents() const
{
   return true;
}

bool GraphicLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                        Qt::KeyboardModifiers modifiers)
{
   if (mProcessingMouseEvent)
   {
      return false;
   }
   ResetVariableOnDestroy<bool> setter(mProcessingMouseEvent, true);

   LocationType point;

   View* pView = getView();
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
      translateWorldToData(point.mX, point.mY, point.mX, point.mY);
   }

   bool bUsedEvent = false;
   if (mpInsertingObject == NULL)
   {
      if (button == Qt::LeftButton)
      {
         mpUndoLock = new UndoLock(pView);

         GraphicObject* pObj = addObject(getCurrentGraphicObjectType(), point);
         mpInsertingObject = dynamic_cast<GraphicObjectImp*>(pObj);
         if (mpInsertingObject == NULL)
         {
            return false;
         }
         connect(mpInsertingObject, SIGNAL(destroyed()), this, SLOT(clearInsertingObject()));
         emit objectAdded(pObj);
      }
   }

   if (mpInsertingObject != NULL)
   {
      if (mpInsertingObject->insertionUndoable() == false)
      {
         if (mpUndoLock != NULL)
         {
            delete mpUndoLock;
            mpUndoLock = NULL;
         }
      }

      bUsedEvent = mpInsertingObject->processMousePress(point, button, buttons, modifiers);
   }

   if (bUsedEvent)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      if (pGroup != NULL)
      {
         pGroup->updateBoundingBox();
      }
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
   return bUsedEvent;

}

bool GraphicLayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                       Qt::KeyboardModifiers modifiers)
{
   if (mProcessingMouseEvent)
   {
      return false;
   }
   ResetVariableOnDestroy<bool> setter(mProcessingMouseEvent, true);

   LocationType point;

   ViewImp* pView = dynamic_cast<ViewImp*> (getView());
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
      translateWorldToData(point.mX, point.mY, point.mX, point.mY);
   }

   bool bUsedEvent = false;
   if (mpInsertingObject != NULL)
   {
      bUsedEvent = mpInsertingObject->processMouseMove(point, button, buttons, modifiers);
   }
   if (bUsedEvent)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      if (pGroup != NULL)
      {
         pGroup->updateBoundingBox();
      }
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return bUsedEvent;
}

int GraphicLayerImp::selectObjects(const LocationType& corner1, const LocationType& corner2)
{
   int count = 0;
   double minx = min(corner1.mX, corner2.mX);
   double maxx = max(corner1.mX, corner2.mX);
   double miny = min(corner1.mY, corner2.mY);
   double maxy = max(corner1.mY, corner2.mY);
   deselectAllObjects();

   list<GraphicObject*> objects = getObjects();
   for (list<GraphicObject*>::iterator it = objects.begin(); it != objects.end(); ++it)
   {
      GraphicObjectImp* pObject = dynamic_cast<GraphicObjectImp*>(*it);
      if (pObject != NULL)
      {
         LocationType llCorner = pObject->getLlCorner();
         LocationType urCorner = pObject->getUrCorner();
         LocationType ulCorner(llCorner.mX, urCorner.mY);
         LocationType lrCorner(urCorner.mX, llCorner.mY);

         LocationType adjustedLlCorner = llCorner;
         LocationType adjustedUrCorner = urCorner;
         LocationType adjustedUlCorner = ulCorner;
         LocationType adjustedLrCorner = lrCorner;

         double dRotation = pObject->getRotation();
         if (dRotation != 0.0)
         {
            LocationType center;
            center.mX = (llCorner.mX + urCorner.mX) / 2.0;
            center.mY = (llCorner.mY + urCorner.mY) / 2.0;

            adjustedLlCorner = DrawUtil::getRotatedCoordinate(llCorner, center, dRotation);
            adjustedUrCorner = DrawUtil::getRotatedCoordinate(urCorner, center, dRotation);
            adjustedUlCorner = DrawUtil::getRotatedCoordinate(ulCorner, center, dRotation);
            adjustedLrCorner = DrawUtil::getRotatedCoordinate(lrCorner, center, dRotation);
         }

         LocationType proj[4];
         proj[0].mX = adjustedLlCorner.mX;
         proj[0].mY = adjustedLlCorner.mY;
         proj[1].mX = adjustedUrCorner.mX;
         proj[1].mY = adjustedUrCorner.mY;
         proj[2].mX = adjustedUlCorner.mX;
         proj[2].mY = adjustedUlCorner.mY;
         proj[3].mX = adjustedLrCorner.mX;
         proj[3].mY = adjustedLrCorner.mY;

         double minProjx = 1e38;
         double minProjy = 1e38;
         double maxProjx = -1e38;
         double maxProjy = -1e38;
         int i;
         for (i = 0; i < 4; ++i)
         {
            minProjx = min(minProjx, proj[i].mX);
            maxProjx = max(maxProjx, proj[i].mX);
            minProjy = min(minProjy, proj[i].mY);
            maxProjy = max(maxProjy, proj[i].mY);
         }
         if (minProjx > minx && minProjx < maxx &&
            minProjy > miny && minProjy < maxy &&
            maxProjx > minx && maxProjx < maxx &&
            maxProjy > miny && maxProjy < maxy)
         {
            mSelectedObjects.push_back(*it);
            selectObject(*it);
            ++count;
         }
      }
   }

   if (count != 0)
   {
      emit objectsSelected(mSelectedObjects);
      emit modified();
      notify(SIGNAL_NAME(GraphicLayer, ObjectsSelected), boost::any(mSelectedObjects));
   }
   return count;
}


bool GraphicLayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                          Qt::KeyboardModifiers modifiers)
{
   if (mProcessingMouseEvent)
   {
      return false;
   }
   ResetVariableOnDestroy<bool> setter(mProcessingMouseEvent, true);

   LocationType point;

   ViewImp* pView = dynamic_cast<ViewImp*> (getView());
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
      translateWorldToData(point.mX, point.mY, point.mX, point.mY);
   }
   bool bUsedEvent = false;
   if (mpInsertingObject != NULL)
   {
      bUsedEvent = mpInsertingObject->processMouseRelease(
         point, button, buttons, modifiers);
   }
   if (bUsedEvent)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      if (pGroup != NULL)
      {
         pGroup->updateBoundingBox();
      }
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return bUsedEvent;
}

bool GraphicLayerImp::processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button,
                                              Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (mProcessingMouseEvent)
   {
      return false;
   }

   ResetVariableOnDestroy<bool> setter(mProcessingMouseEvent, true);
   LocationType point;

   View* pView = getView();
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), point.mX, point.mY);
      translateWorldToData(point.mX, point.mY, point.mX, point.mY);
   }

   bool bUsedEvent = false;
   if (mpInsertingObject != NULL)
   {
      bUsedEvent = mpInsertingObject->processMouseDoubleClick(point, button, buttons, modifiers);
   }

   if (bUsedEvent)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      if (pGroup != NULL)
      {
         pGroup->updateBoundingBox();
      }

      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }

   return bUsedEvent;
}

bool GraphicLayerImp::grabHandle(const QPoint& screenCoord, GraphicObject*& pObject, int& handle) const
{
   LocationType sceneCoord;

   View* pView = getView();
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), sceneCoord.mX, sceneCoord.mY);
      translateWorldToData(sceneCoord.mX, sceneCoord.mY, sceneCoord.mX, sceneCoord.mY);
   }

   return grabHandle(sceneCoord, pObject, handle);
}

bool GraphicLayerImp::grabHandle(LocationType sceneCoord, GraphicObject*& pObject, int& handle) const
{
   // Compute the size of the handle between the center and edges in data coordinates
   double dScreenX = 0.0;
   double dScreenY = 0.0;
   translateDataToScreen(sceneCoord.mX, sceneCoord.mY, dScreenX, dScreenY);

   double xCoord = 0.0;
   double yCoord = 0.0;
   translateScreenToData(dScreenX + mHandleSize, dScreenY + mHandleSize, xCoord, yCoord);

   double xSize = fabs(xCoord - sceneCoord.mX);
   double ySize = fabs(yCoord - sceneCoord.mY);

   glColor3ub (255, 255, 255);
   glLineWidth (1);

   int i = -1;

   list<GraphicObject*>::const_reverse_iterator it;
   for (it = mSelectedObjects.rbegin(); it != mSelectedObjects.rend(); ++it)
   {
      GraphicObjectImp* pSelectedObject = dynamic_cast<GraphicObjectImp*>(*it);
      if (pSelectedObject != NULL)
      {
         LocationType adjustedCoord = sceneCoord;

         double dRotation = pSelectedObject->getRotation();
         if (dRotation != 0.0)
         {
            LocationType llCorner = pSelectedObject->getLlCorner();
            LocationType urCorner = pSelectedObject->getUrCorner();

            LocationType center;
            center.mX = (llCorner.mX + urCorner.mX) / 2.0;
            center.mY = (llCorner.mY + urCorner.mY) / 2.0;

            adjustedCoord = DrawUtil::getRotatedCoordinate(sceneCoord, center, -dRotation);
         }

         vector<LocationType> handles = pSelectedObject->getHandles();
         vector<LocationType>::reverse_iterator cit;
         for (i = handles.size() - 1, cit = handles.rbegin(); cit != handles.rend(); ++cit, --i)
         {
            if ((fabs(adjustedCoord.mX - (*cit).mX) < xSize) && (fabs(adjustedCoord.mY - (*cit).mY) < ySize))
            {
               break;
            }
         }

         if (i >= 0)
         {
            break;
         }
      }
   }

   if (i >= 0)
   {
      pObject = *it;
      handle = i;
      return true;
   }

   pObject = NULL;
   handle = -1;
   return false;
}

void GraphicLayerImp::groupSelection()
{
   list<GraphicObject*> selected = mSelectedObjects;
   if (selected.size() < 2)
   {
      return;
   }

   View* pView = getView();
   GraphicGroup* pGroup = NULL;
   {
      UndoLock lock(pView);

      for_each(selected.begin(), selected.end(), boost::bind(&GraphicLayerImp::removeObject, this, _1, false));

      pGroup = static_cast<GraphicGroup*>(addObject(GROUP_OBJECT, LocationType()));
      if (pGroup == NULL)
      {
         GraphicGroup* pBaseGroup = getGroup();
         if (pBaseGroup != NULL)
         {
            for_each(selected.begin(), selected.end(), boost::bind(&GraphicGroup::insertObject, pBaseGroup, _1));
            for_each(selected.begin(), selected.end(), boost::bind(&GraphicLayerImp::selectObject, this, _1));
         }

         return;
      }

      for_each(selected.begin(), selected.end(), boost::bind(&GraphicGroup::insertObject, pGroup, _1));
   }

   UndoGroup group(pView, "Group Objects");
   if (pView != NULL)
   {
      UndoAction* pUndoAction = new GroupGraphicObjects(dynamic_cast<GraphicLayer*>(this), pGroup);
      pView->addUndoAction(pUndoAction);
   }

   GraphicGroupImp* pGroupImp = dynamic_cast<GraphicGroupImp*>(pGroup);
   REQUIRE(pGroupImp != NULL);
   pGroupImp->updateBoundingBox();
   selectObject(pGroup);
   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));
}

void GraphicLayerImp::ungroupSelection()
{
   int iGroupCount = 0;
   list<GraphicObject*> objects = getObjects();

   View* pView = getView();
   UndoGroup group(pView, "Ungroup Objects");

   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject == NULL)
      {
         continue;
      }

      bool bSelected = isObjectSelected(pObject);
      if (bSelected == false)
      {
         continue;
      }

      GraphicGroup* pGroup = dynamic_cast<GraphicGroup*>(pObject);
      if (pGroup == NULL)
      {
         continue;
      }

      if (pView != NULL)
      {
         UndoAction* pUndoAction = new UngroupGraphicObjects(dynamic_cast<GraphicLayer*>(this), pGroup);
         pView->addUndoAction(pUndoAction);
      }

      iGroupCount++;

      LocationType llGroupCorner = pGroup->getLlCorner();
      LocationType urGroupCorner = pGroup->getUrCorner();

      LocationType centerGroup;
      centerGroup.mX = (llGroupCorner.mX + urGroupCorner.mX) / 2.0;
      centerGroup.mY = (llGroupCorner.mY + urGroupCorner.mY) / 2.0;

      double angleGroup = 0.0;
      angleGroup = pGroup->getRotation();
      if (angleGroup < 0.0)
      {
         angleGroup = 0.0;
      }

      list<GraphicObject*> groupObjects = pGroup->getObjects();

      list<GraphicObject*>::iterator groupIter;
      for (groupIter = groupObjects.begin(); groupIter != groupObjects.end(); ++groupIter)
      {
         GraphicObject* pGroupObject = *groupIter;
         if (pGroupObject == NULL)
         {
            continue;
         }

         LocationType llItemCorner = pGroupObject->getLlCorner();
         LocationType urItemCorner = pGroupObject->getUrCorner();

         LocationType centerItem;
         centerItem.mX = (llItemCorner.mX + urItemCorner.mX) / 2.0;
         centerItem.mY = (llItemCorner.mY + urItemCorner.mY) / 2.0;

         double corC1x = centerItem.mX - centerGroup.mX;
         double corC1y = centerItem.mY - centerGroup.mY;
         double angleGroupRad = (PI / 180.0) * angleGroup;
         double calcCos1 = cos(angleGroupRad) - 1;
         double calcSin = sin(angleGroupRad);
         double xOffset = (corC1x * calcCos1) - (calcSin * corC1y);
         double yOffset = (corC1y * calcCos1) + (corC1x * calcSin);

         dynamic_cast<GraphicObjectImp*>(pGroupObject)->move(LocationType(xOffset, yOffset));

         double angleObject = pGroupObject->getRotation();
         if (angleObject < 0.0)
         {
            angleObject = 0.0;
         }

         angleObject += angleGroup;
         pGroupObject->setRotation(angleObject);

         pGroup->removeObject(pGroupObject, false);
         getGroup()->insertObject(pGroupObject);
         selectObject(pGroupObject);
      }

      UndoLock lock(pView);
      removeObject(pGroup, true);
   }

   if (iGroupCount > 0)
   {
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void GraphicLayerImp::pushSelectedObjectToBack()
{
   list<GraphicObject*>::reverse_iterator sit;

   View* pView = getView();
   UndoGroup group(pView, "Move Objects To Back");
   for (sit = mSelectedObjects.rbegin(); sit != mSelectedObjects.rend(); ++sit)
   {
      int oldIndex = getObjectStackingIndex(*sit);

      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      bool bSuccess = pGroup != NULL && pGroup->moveObjectToBack(*sit);
      if (bSuccess == true)
      {
         if (pView != NULL)
         {
            int newIndex = getObjectStackingIndex(*sit);
            pView->addUndoAction(new SetGraphicStackingOrder(dynamic_cast<GraphicLayer*>(this), *sit,
               oldIndex, newIndex));
         }
      }
   }

   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));
}

void GraphicLayerImp::popSelectedObjectToFront()
{
   list<GraphicObject*>::iterator sit;

   View* pView = getView();
   UndoGroup group(pView, "Move Objects To Front");
   for (sit = mSelectedObjects.begin(); sit != mSelectedObjects.end(); ++sit)
   {
      int oldIndex = getObjectStackingIndex(*sit);

      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      bool bSuccess = pGroup != NULL && pGroup->moveObjectToFront(*sit);
      if (bSuccess == true)
      {
         if (pView != NULL)
         {
            int newIndex = getObjectStackingIndex(*sit);
            pView->addUndoAction(new SetGraphicStackingOrder(dynamic_cast<GraphicLayer*>(this), *sit,
               oldIndex, newIndex));
         }
      }
   }

   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));
}

int GraphicLayerImp::getObjectStackingIndex(GraphicObject* pObject) const
{
   GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
   if (pGroup != NULL)
   {
      return pGroup->getObjectStackingIndex(pObject);
   }
   return -1;
}

void GraphicLayerImp::setObjectStackingIndex(GraphicObject* pObject, int index)
{
   if (pObject == NULL)
   {
      return;
   }

   int currentIndex = getObjectStackingIndex(pObject);
   if (index != currentIndex)
   {
      GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
      VERIFYNRV(pGroup != NULL);

      View* pView = getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new SetGraphicStackingOrder(dynamic_cast<GraphicLayer*>(this), pObject,
            currentIndex, index));
      }

      pGroup->setObjectStackingIndex(pObject, index);
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void GraphicLayerImp::cleanUpBadObject(GraphicObject* pObj)
{
   // this stuff will move to a slot method that is connected to a Group's signal stating that something went wrong
   getGroup()->removeObject(pObj, false);
   mSelectedObjects.remove(pObj);
}

void GraphicLayerImp::cloneSelection(GraphicLayer* pDest)
{
   GraphicLayerImp* pDestImp = dynamic_cast<GraphicLayerImp*>(pDest);
   list<GraphicObject*> objects = getObjects();

   View* pView = getView();
   UndoGroup group(pView, "Copy Object");

   for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
   {
      GraphicObject* pObject = *iter;
      if (pObject == NULL)
      {
         continue;
      }

      bool bSelected = isObjectSelected(pObject);
      if (bSelected == false)
      {
         continue;
      }

      GraphicObjectType eType = pObject->getGraphicObjectType();

      GraphicObject* pCloneObject = pDest->addObject(eType);
      if (pCloneObject != NULL)
      {
         bool bSuccess = dynamic_cast<GraphicObjectImp*>(pCloneObject)->replicateObject(pObject);
         if (bSuccess == false)
         {
            return;
         }

         pDest->selectObject(pCloneObject);
      }

      if (pDestImp == this)
      {
         deselectObject(pObject);
      }
   }

   if (pDestImp == this)
   {
      if (pView != NULL)
      {
         pView->refresh();
      }
   }
}//end GraphicLayerImp::cloneSelection

GraphicObject* GraphicLayerImp::hit(const QPoint& screenCoord) const
{
   LocationType sceneCoord;

   View* pView = getView();
   if (pView != NULL)
   {
      pView->translateScreenToWorld(screenCoord.x(), screenCoord.y(), sceneCoord.mX, sceneCoord.mY);
      translateWorldToData(sceneCoord.mX, sceneCoord.mY, sceneCoord.mX, sceneCoord.mY);
   }

   return hit(sceneCoord);
}

GraphicObject* GraphicLayerImp::hit(LocationType sceneCoord) const
{
   list<GraphicObject*>::const_reverse_iterator iter;
   for (iter = mSelectedObjects.rbegin(); iter != mSelectedObjects.rend(); ++iter)
   {
      GraphicObject* pObject = *iter;
      GraphicObjectImp* pObjectImp = dynamic_cast<GraphicObjectImp*>(pObject);
      if (pObject != NULL && pObjectImp != NULL)
      {
         bool bHit = false;

         double dRotation = pObject->getRotation();
         if (dRotation != 0.0)
         {
            LocationType llCorner = pObject->getLlCorner();
            LocationType urCorner = pObject->getUrCorner();

            LocationType center;
            center.mX = (llCorner.mX + urCorner.mX) / 2.0;
            center.mY = (llCorner.mY + urCorner.mY) / 2.0;

            LocationType adjustedCoord = DrawUtil::getRotatedCoordinate(sceneCoord, center, -dRotation);
            bHit = pObjectImp->hit(adjustedCoord);
         }
         else
         {
            bHit = pObjectImp->hit(sceneCoord);
         }

         if (bHit == true)
         {
            return pObject;
         }
      }
   }
   GraphicGroupImp* pGroup = dynamic_cast<GraphicGroupImp*>(getGroup());
   VERIFY(pGroup != NULL);
   return pGroup->hitObject(sceneCoord);
}

void GraphicLayerImp::replicateObject(GraphicObject* pDest, GraphicObject* pSource)
{
   GraphicObjectImp* pDestImp = dynamic_cast<GraphicObjectImp*>(pDest);
   if (pDestImp && pSource)
   {
      UndoGroup group(getView(), "Copy Object");
      pDestImp->replicateObject(pSource);
   }
}

void GraphicLayerImp::nudgeSelectedObjects(int x, int y)
{
   LocationType data1;
   LocationType data2;

   View* pView = getView();
   if (pView != NULL)
   {
      LocationType world1;
      LocationType world2;
      pView->translateScreenToWorld(0, 0, world1.mX, world1.mY);
      translateWorldToData(world1.mX, world1.mY, data1.mX, data1.mY);
      pView->translateScreenToWorld(x, y, world2.mX, world2.mY);
      translateWorldToData(world2.mX, world2.mY, data2.mX, data2.mY);
   }

   LocationType delta (2.0*(data2.mX-data1.mX), 2.0*(data2.mY-data1.mY));
   moveSelectedObjects(delta);
}

void GraphicLayerImp::moveSelectedObjects(LocationType delta)
{
   UndoGroup group(getView(), "Move Objects");

   for (list<GraphicObject*>::iterator it = mSelectedObjects.begin(); it != mSelectedObjects.end(); ++it)
   {
      dynamic_cast<GraphicObjectImp*>(*it)->move(delta);
   }
   emit modified();
   notify(SIGNAL_NAME(Subject, Modified));
}

static void alignCorners(LocationType& llCorner, LocationType& urCorner)
{
   LocationType alignedLlCorner;
   LocationType alignedUrCorner;

   alignedLlCorner.mX = min(llCorner.mX, urCorner.mX);
   alignedLlCorner.mY = min(llCorner.mY, urCorner.mY);
   alignedUrCorner.mX = max(llCorner.mX, urCorner.mX);
   alignedUrCorner.mY = max(llCorner.mY, urCorner.mY);

   llCorner = alignedLlCorner;
   urCorner = alignedUrCorner;
}

static bool getGroupedBoundingBox(const list<GraphicObject*>& objectList, LocationType& llCorner,
                                  LocationType& urCorner)
{
   if (objectList.size() == 0)
   {
      return false;
   }

   llCorner.mX = llCorner.mY = 1e38;
   urCorner.mX = urCorner.mY = -1e38;

   for (list<GraphicObject*>::const_iterator it = objectList.begin(); it != objectList.end(); ++it)
   {
      LocationType objectLlCorner = (*it)->getLlCorner();
      LocationType objectUrCorner = (*it)->getUrCorner();
      alignCorners(objectLlCorner, objectUrCorner);

      llCorner.mX = min(llCorner.mX, objectLlCorner.mX);
      llCorner.mY = min(llCorner.mY, objectLlCorner.mY);
      urCorner.mX = max(urCorner.mX, objectUrCorner.mX);
      urCorner.mY = max(urCorner.mY, objectUrCorner.mY);
   }

   return true;
}

void GraphicLayerImp::alignSelectedObjects(GraphicAlignment alignment)
{
   LocationType llCorner;
   LocationType urCorner; // maximum extents of the selected objects' bounding boxes
   if (!getGroupedBoundingBox(mSelectedObjects, llCorner, urCorner))
   {
      return;
   }

   bool bHorizontalFlip = false;
   bool bVerticalFlip = false;
   isFlipped(llCorner, urCorner, bHorizontalFlip, bVerticalFlip);

   double desiredLocation = llCorner.mX;
   double llMultiplier = 1.0;
   double urMultiplier = 1.0;
   switch (alignment)
   {
      case ALIGN_LEFT:
         desiredLocation = bHorizontalFlip ? urCorner.mX : llCorner.mX;
         llMultiplier = bHorizontalFlip ? 0.0 : 1.0;
         urMultiplier = bHorizontalFlip ? 1.0 : 0.0;
         break;
      case ALIGN_CENTER:
         desiredLocation = (llCorner.mX + urCorner.mX) / 2.0;
         llMultiplier = urMultiplier = 0.5;
         break;
      case ALIGN_RIGHT:
         desiredLocation = bHorizontalFlip ? llCorner.mX : urCorner.mX;
         llMultiplier = bHorizontalFlip ? 1.0 : 0.0;
         urMultiplier = bHorizontalFlip ? 0.0 : 1.0;
         break;
      case ALIGN_TOP:
         desiredLocation = bVerticalFlip ? llCorner.mY : urCorner.mY;
         llMultiplier = bVerticalFlip ? 1.0 : 0.0;
         urMultiplier = bVerticalFlip ? 0.0 : 1.0;
         break;
      case ALIGN_MIDDLE:
         desiredLocation = (llCorner.mY + urCorner.mY) / 2.0;
         llMultiplier = urMultiplier = 0.5;
         break;
      case ALIGN_BOTTOM:
         desiredLocation = bVerticalFlip ? urCorner.mY : llCorner.mY;
         llMultiplier = bVerticalFlip ? 0.0 : 1.0;
         urMultiplier = bVerticalFlip ? 1.0 : 0.0;
         break;
      default:
         break;
   }

   bool horizontal = (alignment == ALIGN_LEFT || alignment == ALIGN_CENTER || alignment == ALIGN_RIGHT);

   UndoGroup group(getView(), "Align Objects");

   LocationType delta(0.0, 0.0);
   LocationType objectLlCorner;
   LocationType objectUrCorner;
   for (list<GraphicObject*>::iterator it = mSelectedObjects.begin(); it != mSelectedObjects.end(); ++it)
   {
      objectLlCorner = (*it)->getLlCorner();
      objectUrCorner = (*it)->getUrCorner();

      alignCorners(objectLlCorner, objectUrCorner);

      if (horizontal)
      {
         delta.mX = desiredLocation - (objectLlCorner.mX*llMultiplier + objectUrCorner.mX*urMultiplier);
      }
      else
      {
         delta.mY = desiredLocation - (objectLlCorner.mY*llMultiplier + objectUrCorner.mY*urMultiplier);
      }
      dynamic_cast<GraphicObjectImp*>(*it)->move(delta);
   }
}

void GraphicLayerImp::distributeSelectedObjects(GraphicDistribution distribution)
{
   double xMultiplier = distribution==DISTRIBUTE_HORIZONTALLY;
   double yMultiplier = distribution==DISTRIBUTE_VERTICALLY;
   LocationType corner[2];
   int i;

   if (mSelectedObjects.size() < 3)
   {
      return;
   }

   // find left and right most objects
   list<GraphicObject*> objectList = mSelectedObjects;
   list<GraphicObject*>::iterator it;
   list<GraphicObject*>::iterator minIter = objectList.begin();
   list<GraphicObject*>::iterator maxIter = objectList.begin();
   double minEdgePos = 1e38;
   double maxEdgePos = -1e38;
   GraphicObject* minEdgeObject = NULL;
   GraphicObject* maxEdgeObject = NULL;
   for (it = objectList.begin(); it != objectList.end(); ++it)
   {
      corner[0] = (*it)->getLlCorner();
      corner[1] = (*it)->getUrCorner();
      alignCorners(corner[0], corner[1]);
      for (i = 0; i < 2; ++i)
      {
         double edgePos = corner[i].mX*xMultiplier+corner[i].mY*yMultiplier;
         if (edgePos < minEdgePos)
         {
            minEdgePos = edgePos;
            minIter = it;
         }
         else if (edgePos > maxEdgePos)
         {
            maxEdgePos = edgePos;
            maxIter = it;
         }
      }
   }

   minEdgeObject = *minIter;
   maxEdgeObject = *maxIter;

   if (minEdgeObject == maxEdgeObject)
   {
      return;
   }

   // sort remaining objects from left to right based on their left side
   objectList.erase(maxIter);
   objectList.erase(minIter);

   objectList.sort();

   // take distance from left edge of left most and right most objects = total distance
   corner[0] = maxEdgeObject->getLlCorner();
   corner[1] = maxEdgeObject->getUrCorner();
   alignCorners(corner[0], corner[1]);
   maxEdgePos = min(corner[0].mX, corner[1].mX) * xMultiplier + min(corner[0].mY, corner[1].mY) * yMultiplier;

   double totalDistance = maxEdgePos - minEdgePos;
   double spacing = totalDistance / (mSelectedObjects.size()-1);

   UndoGroup group(getView(), "Distribute Objects");

   //for each object in sorted list (index=0; index++)
      //move it so that its left edge is at index*spacing
   LocationType delta(0.0, 0.0);
   for (i = 1, it = objectList.begin(); it != objectList.end(); ++i, ++it)
   {
      double desiredLocation = spacing*i + minEdgePos;
      corner[0] = (*it)->getLlCorner();
      corner[1] = (*it)->getUrCorner();
      alignCorners(corner[0], corner[1]);
      delta.mX = (desiredLocation - min(corner[0].mX, corner[1].mX)) * xMultiplier;
      delta.mY = (desiredLocation - min(corner[0].mY, corner[1].mY)) * yMultiplier;
      dynamic_cast<GraphicObjectImp*>(*it)->move(delta);
   }
}

void GraphicLayerImp::setHideSelectionBox(bool hide)
{
   mbHideSelectionBox = hide;
}

bool GraphicLayerImp::getShowLabels() const
{
   return mShowLabels;
}

void GraphicLayerImp::setShowLabels(bool bShowLabels)
{
   if (bShowLabels != mShowLabels)
   {
      mShowLabels = bShowLabels;
      emit showLabelsChanged(mShowLabels);
   }
}

bool GraphicLayerImp::canContainGraphicObjectType(GraphicObjectType type)
{
   return (mAcceptableTypes.find(type) != mAcceptableTypes.end());
}

void GraphicLayerImp::addAcceptableGraphicType(GraphicObjectType type)
{
   mAcceptableTypes.insert(type);
}

void GraphicLayerImp::removeAcceptableGraphicType(GraphicObjectType type)
{
   mAcceptableTypes.erase(type);
}

void GraphicLayerImp::clearAcceptableGraphicTypes()
{
   mAcceptableTypes.clear();
}

void GraphicLayerImp::reset()
{
   // nothing to reset
}

GraphicGroup* GraphicLayerImp::getGroup() const
{
   GraphicElement* pGraph = dynamic_cast<GraphicElement*>(getDataElement());
   GraphicGroup* pGroup = NULL;
   if (pGraph != NULL)
   {
      pGroup = pGraph->getGroup();
   }

   if (pGroup != NULL && !mGroupHasLayerSet)
   {
      GraphicGroupImp* pImp = dynamic_cast<GraphicGroupImp*>(pGroup);
      VERIFY(pImp != NULL);
      pImp->setLayer(const_cast<GraphicLayer*>(dynamic_cast<const GraphicLayer*>(this)));
      mGroupHasLayerSet = true;
   }

   return pGroup;
}

void GraphicLayerImp::setCurrentGraphicObjectType(GraphicObjectType type)
{
   if (mCurrentType != type)
   {
      completeInsertion();
      mCurrentType = type;
      emit currentTypeChanged(type);
   }
}

GraphicObjectType GraphicLayerImp::getCurrentGraphicObjectType() const
{
   return mCurrentType;
}

LocationType GraphicLayerImp::correctCoordinate(const LocationType& coord) const
{
   return coord;
}

void GraphicLayerImp::completeInsertion(bool bValidObject)
{
   GraphicObject* pInsertingObject = dynamic_cast<GraphicObject*>(mpInsertingObject);
   if (mpInsertingObject != NULL)
   {
      disconnect(mpInsertingObject, SIGNAL(destroyed()), this, SLOT(clearInsertingObject()));
      if (!mpInsertingObject->isVisible())
      {
         delete mpInsertingObject;
         pInsertingObject = NULL;
      }
      else if (!bValidObject)
      {
         removeObject(dynamic_cast<GraphicObject*>(mpInsertingObject), true);
         pInsertingObject = NULL;
      }
   }

   if (mpUndoLock != NULL)
   {
      delete mpUndoLock;
      mpUndoLock = NULL;
   }

   if ((pInsertingObject != NULL) && (mpInsertingObject != NULL) && (mpInsertingObject->insertionUndoable()))
   {
      View* pView = getView();
      if (pView != NULL)
      {
         GraphicGroup* pGroup = getGroup();
         if (pGroup != NULL)
         {
            pView->addUndoAction(new AddGraphicObject(pGroup, pInsertingObject));
         }
      }
   }

   mpInsertingObject = NULL;
   notify(SIGNAL_NAME(GraphicLayer, ObjectInsertionCompleted), pInsertingObject);
}

void GraphicLayerImp::onElementModified()
{
   View* pView = getView();
   if (pView != NULL)
   {
      pView->refresh();
   }
}

QColor GraphicLayerImp::getLabelColor(const GraphicObjectImp* pObj)
{
   return QColor();
}

bool GraphicLayerImp::mayDrawAsPixels() const
{
   return false;
}

bool GraphicLayerImp::willDrawAsPixels() const
{
   return false;
}

bool GraphicLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   GraphicGroup* pGroup = getGroup();
   if (pGroup == NULL)
   {
      return false;
   }

   LocationType ll = pGroup->getLlCorner();
   LocationType ur = pGroup->getUrCorner();
   x1 = min(ll.mX, ur.mX);
   y1 = min(ll.mY, ur.mY);
   x4 = max(ll.mX, ur.mX);
   y4 = max(ll.mY, ur.mY);

   return true;
}

void GraphicLayerImp::clearInsertingObject()
{
   mpInsertingObject = NULL;
}

bool GraphicLayerImp::getLayerLocked() const
{
   return mLayerLocked;
}

void GraphicLayerImp::setLayerLocked(bool bLocked)
{
   if (bLocked != mLayerLocked)
   {
      mLayerLocked = bLocked;
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void GraphicLayerImp::drawSymbols(const string& symbolName, const vector<LocationType>& points, double screenSize,
                                  double objectRotation)
{
   if (symbolName.empty())
   {
      return;
   }

   SymbolManager* pSymMgr = SymbolManager::instance();
   VERIFYNRV(pSymMgr != NULL);

   double zoomPercent = 100;
   double rotation = 0;
   double pitch = 90;

   double xScale = getXScaleFactor();
   double yScale = getYScaleFactor();

   PerspectiveView* pPerView = dynamic_cast<PerspectiveView*>(getView());
   if (pPerView != NULL)
   {
      zoomPercent = pPerView->getZoomPercentage();
      rotation = pPerView->getRotation();
      pitch = pPerView->getPitch();
   }

   OrthographicView* pOrthView = dynamic_cast<OrthographicView*>(getView());
   if (pOrthView != NULL)
   {
      LocationType pixelSize = pOrthView->getPixelSize();
      xScale *= pixelSize.mX;
      yScale *= pixelSize.mY;
   }

   pSymMgr->drawSymbols(symbolName, points, screenSize, zoomPercent / 100, rotation, pitch, xScale, yScale,
      objectRotation);
}

bool GraphicLayerImp::insertingObjectNull() const
{
   return (mpInsertingObject == NULL);
}

void GraphicLayerImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   if (dynamic_cast<SessionExplorer*>(&subject) == NULL)
   {
      return;
   }

   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   if (getLayerLocked() == true)
   {
      return;
   }

   QObject* pParent = pMenu->getActionParent();

   vector<SessionItem*> items = pMenu->getSessionItems();

   unsigned int numItems = items.size();
   if (numItems == 1)
   {
      GraphicObject* pObject = dynamic_cast<GraphicObject*>(items.front());
      if ((pObject != NULL) && (hasObject(pObject) == true))
      {
         // Separator
         QAction* pSeparatorAction = new QAction(pParent);
         pSeparatorAction->setSeparator(true);
         pMenu->addActionBefore(pSeparatorAction, APP_GRAPHICLAYER_OBJECT_DELETE_SEPARATOR_ACTION,
            APP_SESSIONEXPLORER_RENAME_ACTION);

         // Delete
         QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "Delete", pParent);
         pDeleteAction->setAutoRepeat(false);
         pDeleteAction->setData(QVariant::fromValue(pObject));
         connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(deleteObject()));
         pMenu->addActionBefore(pDeleteAction, APP_GRAPHICLAYER_OBJECT_DELETE_ACTION,
            APP_GRAPHICLAYER_OBJECT_DELETE_SEPARATOR_ACTION);
      }
   }
   else if (numItems > 1)
   {
      vector<GraphicObject*> objects = pMenu->getSessionItems<GraphicObject>();
      if (objects.size() == numItems)
      {
         QList<QVariant> objectList;
         for (vector<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
         {
            GraphicObject* pObject = *iter;
            if (pObject != NULL)
            {
               if (hasObject(pObject) == false)
               {
                  return;
               }

               QVariant object = QVariant::fromValue(pObject);
               objectList.append(object);
            }
         }

         // Delete
         QAction* pDeleteAction = new QAction(QIcon(":/icons/Delete"), "Delete", pParent);
         pDeleteAction->setAutoRepeat(false);
         pDeleteAction->setData(QVariant(objectList));
         connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(deleteObject()));
         pMenu->addAction(pDeleteAction, APP_GRAPHICLAYER_OBJECT_DELETE_ACTION);
      }
   }
}

void GraphicLayerImp::deleteObject()
{
   QAction* pAction = dynamic_cast<QAction*>(sender());
   if (pAction == NULL)
   {
      return;
   }

   QVariant objectVariant = pAction->data();
   if (objectVariant.canConvert<GraphicObject*>() == true)
   {
      GraphicObject* pObject = objectVariant.value<GraphicObject*>();
      if (pObject != NULL)
      {
         removeObject(pObject, true);
      }
   }
   else if (objectVariant.canConvert(QVariant::List) == true)
   {
      QList<QVariant> objects = objectVariant.toList();
      if (objects.empty() == true)
      {
         return;
      }

      UndoGroup group(getView(), "Delete Graphic Objects");

      for (int i = 0; i < objects.count(); ++i)
      {
         GraphicObject* pObject = objects[i].value<GraphicObject*>();
         if (pObject != NULL)
         {
            removeObject(pObject, true);
         }
      }
   }
}

void GraphicLayerImp::layerActivated(bool activated)
{
   if (activated == false)
   {
      completeInsertion();
      deselectAllObjects();
   }
}
