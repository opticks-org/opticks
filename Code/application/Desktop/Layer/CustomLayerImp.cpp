/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CustomLayer.h"
#include "CustomLayerImp.h"
#include "SpatialDataView.h"
#include "XercesIncludes.h"

#include <vector>

#include <QtCore/QByteArray>
#include <QtCore/QDataStream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

CustomLayerImp::CustomLayerImp(const string& id, const string& layerName, DataElement* pElement) :
   LayerImp(id, layerName, pElement),
   mMouseEventsSupported(false),
   mMouseCursor(Qt::ArrowCursor)
{
   mpDrawObject.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &CustomLayerImp::drawObjectDeleted));
   mpDrawObject.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &CustomLayerImp::drawObjectModified));
}

CustomLayerImp::~CustomLayerImp()
{
   setDrawObject(NULL);
}

const string& CustomLayerImp::getObjectType() const
{
   static string type("CustomLayerImp");
   return type;
}

bool CustomLayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "CustomLayer"))
   {
      return true;
   }

   return LayerImp::isKindOf(className);
}

bool CustomLayerImp::toXml(XMLWriter* pXml) const
{
   if (!LayerImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("mouseEventsSupported", mMouseEventsSupported);
   QByteArray cursorArray;
   QDataStream cursorStream(&cursorArray, QIODevice::WriteOnly);
   pXml->pushAddPoint(pXml->addElement("mouseCursor"));
   cursorStream << mMouseCursor;
   string cursorStr = cursorArray.toBase64().data();
   pXml->addText(cursorStr);
   pXml->popAddPoint();

   return true;
}

bool CustomLayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (!LayerImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElem = static_cast<DOMElement*>(pDocument);
   setAcceptsMouseEvents(StringUtilities::fromXmlString<bool>(A(pElem->getAttribute(X("mouseEventsSupported")))));
   for (DOMNode* pChld = pElem->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("mouseCursor")))
      {
         string cursorStr = string(A(pChld->getTextContent()));
         if (cursorStr.empty() == false)
         {
            QByteArray cursorArray(QByteArray::fromBase64(QByteArray::fromRawData(
               cursorStr.c_str(), cursorStr.size())));
            QDataStream cursorStream(&cursorArray, QIODevice::ReadOnly);
            cursorStream >> mMouseCursor;
         }
      }
   }

   return true;
}

LayerType CustomLayerImp::getLayerType() const
{
   return CUSTOM_LAYER;
}

void CustomLayerImp::draw()
{
   if (mpDrawObject.get() != NULL)
   {
      mpDrawObject->draw();
   }
}

bool CustomLayerImp::getExtents(double& x1, double& y1, double& x4, double& y4)
{
   if (mpDrawObject.get() != NULL)
   {
      return mpDrawObject->getExtents(x1, y1, x4, y4);
   }

   return false;
}

bool CustomLayerImp::getExtents(vector<LocationType>& worldCoords)
{
   if (mpDrawObject.get() != NULL)
   {
      return mpDrawObject->getExtents(worldCoords);
   }

   return false;
}

bool CustomLayerImp::isKindOfLayer(const string& className)
{
   if ((className == "CustomLayerImp") || (className == "CustomLayer"))
   {
      return true;
   }

   return LayerImp::isKindOfLayer(className);
}

void CustomLayerImp::getLayerTypes(vector<string>& classList)
{
   classList.push_back("CustomLayer");
   LayerImp::getLayerTypes(classList);
}

CustomLayerImp& CustomLayerImp::operator= (const CustomLayerImp& customLayer)
{
   if (this != &customLayer)
   {
      LayerImp::operator =(customLayer);
      mMouseEventsSupported = customLayer.mMouseEventsSupported;
      mMouseCursor = customLayer.mMouseCursor;
   }

   return *this;
}

bool CustomLayerImp::acceptsMouseEvents() const
{
   return mMouseEventsSupported;
}

QCursor CustomLayerImp::getMouseCursor() const
{
   return mMouseCursor;
}

const QCursor& CustomLayerImp::getEditMouseCursor() const
{
   return mMouseCursor;
}

void CustomLayerImp::reset()
{
   if (mpDrawObject.get() != NULL)
   {
      mpDrawObject->reset();
   }
}

void CustomLayerImp::setAcceptsMouseEvents(bool accept)
{
   mMouseEventsSupported = accept;
}

void CustomLayerImp::setEditMouseCursor(const QCursor& cursor)
{
   mMouseCursor = cursor;
}

void CustomLayerImp::setDrawObject(DrawObject* pDrawObject)
{
   if (pDrawObject == mpDrawObject.get())
   {
      return;
   }

   if (mpDrawObject.get() != NULL)
   {
      delete mpDrawObject.get();
   }

   mpDrawObject.reset(pDrawObject);
   if (mpDrawObject.get() != NULL)
   {
      mpDrawObject->setLayer(dynamic_cast<CustomLayer*>(this));
   }
   notify(SIGNAL_NAME(CustomLayer, DrawObjectChanged), boost::any(mpDrawObject.get()));
   emit extentsModified();
   emit modified();
}

const DrawObject* CustomLayerImp::getDrawObject() const
{
   return mpDrawObject.get();
}

DrawObject* CustomLayerImp::getDrawObject()
{
   return mpDrawObject.get();
}

void CustomLayerImp::drawObjectDeleted(Subject& subject, const string& signal, const boost::any& v)
{
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      pView->deleteLayer(dynamic_cast<Layer*>(this));
   }
}

void CustomLayerImp::drawObjectModified(Subject& subject, const string& signal, const boost::any& v)
{
   notify(SIGNAL_NAME(Subject, Modified));
   emit modified();
}

bool CustomLayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (mpDrawObject.get() != NULL)
   {
      return mpDrawObject->processMousePress(screenCoord, button, buttons, modifiers);
   }

   return LayerImp::processMousePress(screenCoord, button, buttons, modifiers);
}

bool CustomLayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (mpDrawObject.get() != NULL)
   {
      return mpDrawObject->processMouseMove(screenCoord, button, buttons, modifiers);
   }

   return LayerImp::processMouseMove(screenCoord, button, buttons, modifiers);
}

bool CustomLayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
{
   if (mpDrawObject.get() != NULL)
   {
      return mpDrawObject->processMouseRelease(screenCoord, button, buttons, modifiers);
   }

   return LayerImp::processMouseRelease(screenCoord, button, buttons, modifiers);
}

bool CustomLayerImp::processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button,
   Qt::MouseButtons buttons,Qt::KeyboardModifiers modifiers)
{
   if (mpDrawObject.get() != NULL)
   {
     return mpDrawObject->processMouseDoubleClick(screenCoord, button, buttons, modifiers);
   }
   
   return LayerImp::processMouseDoubleClick(screenCoord, button, buttons, modifiers);
}
