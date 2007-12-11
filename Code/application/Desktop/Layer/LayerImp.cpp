/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LayerImp.h"
#include "AnnotationElement.h"
#include "AoiElement.h"
#include "ContextMenuAction.h"
#include "ContextMenuActions.h"
#include "AppVerify.h"
#include "DataElementImp.h"
#include "DrawUtil.h"
#include "FilenameImp.h"
#include "GcpList.h"
#include "LayerList.h"
#include "LayerUndo.h"
#include "ModelServicesImp.h"
#include "RasterElement.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "SpatialDataView.h"
#include "SpatialDataViewImp.h"
#include "StringUtilities.h"
#include "SubjectAdapter.h"
#include "TiePointList.h"
#include "UtilityServicesImp.h"
#include "View.h"
#include "ViewImp.h"
#include "xmlwriter.h"
#include "xmlreader.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

QMap<const DataElement*, int> LayerImp::mElementLayers;

LayerImp::LayerImp(const string& id, const string& layerName, DataElement* pElement) :
   SessionItemImp(id, layerName),
   mbLinking(false),
   mpView(NULL),
   mXScaleFactor(1),
   mYScaleFactor(1),
   mXOffset(0),
   mYOffset(0),
   mpDisplayedAction(NULL)
{
   mpElement.addSignal(SIGNAL_NAME(Subject, Modified), Slot(this, &LayerImp::elementModified));
   mpElement.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &LayerImp::elementDeleted));

   // Context menu actions
   mpDisplayedAction = new QAction("Displayed", this);
   mpDisplayedAction->setCheckable(true);
   mpDisplayedAction->setChecked(true);
   mpDisplayedAction->setAutoRepeat(false);
   mpDisplayedAction->setStatusTip("Toggles display of the layer");
   VERIFYNR(connect(mpDisplayedAction, SIGNAL(triggered(bool)), this, SLOT(showLayer(bool))));

   // Initialization
   addContextMenuAction(ContextMenuAction(mpDisplayedAction, APP_LAYER_DISPLAYED_ACTION));
   setDataElement(pElement);

   // Connections
   VERIFYNR(connect(this, SIGNAL(extentsModified()), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(nameChanged(const QString&)), this, SIGNAL(modified())));
}

LayerImp::~LayerImp()
{
   setDataElement(NULL);
   setView(NULL);
}

const string& LayerImp::getObjectType() const
{
   static string type("LayerImp");
   return type;
}

bool LayerImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Layer"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void LayerImp::elementDeleted(Subject &subject, const string &signal, const boost::any &v)
{
   // Can't check against mpElement, since it may have already been reset
   DataElement *pElement = dynamic_cast<DataElement*>(&subject);
   DataElementImp *pElementImp = dynamic_cast<DataElementImp*>(&subject);
   VERIFYNRV(pElement != NULL && pElementImp != NULL);

   // Remove the element from the map
   mElementLayers.remove(pElement);
}

void LayerImp::elementModified(Subject &subject, const std::string &signal, const boost::any &v)
{
   if (&subject == mpElement.get())
   {
      onElementModified();
      emit modified();
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

void LayerImp::setName(const string& layerName)
{
   if (layerName.empty() == true)
   {
      return;
   }

   const string& oldName = getName();
   if (layerName != oldName)
   {
      // Add the undo action before setting the name since the old name string reference will be updated
      if (mpView != NULL)
      {
         mpView->addUndoAction(new SetLayerName(dynamic_cast<Layer*>(this), oldName, layerName));
      }

      SessionItemImp::setName(layerName);
      emit nameChanged(QString::fromStdString(layerName));
      notify(SIGNAL_NAME(Layer, NameChanged), boost::any(layerName));
   }
}

DataElement* LayerImp::getDataElement() const
{
   return const_cast<DataElement*>(mpElement.get());
}

void LayerImp::setView(ViewImp* pView)
{
   if (pView == mpView)
   {
      return;
   }

   if (mpView != NULL)
   {
      SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(mpView);
      if (pSpatialDataView != NULL)
      {
         disconnect(pSpatialDataView, SIGNAL(layerShown(Layer*)), this, SLOT(updateDisplayedAction(Layer*)));
         disconnect(pSpatialDataView, SIGNAL(layerHidden(Layer*)), this, SLOT(updateDisplayedAction(Layer*)));
      }
   }

   mpView = pView;

   if (mpView != NULL)
   {
      SpatialDataViewImp* pSpatialDataView = dynamic_cast<SpatialDataViewImp*>(mpView);
      if (pSpatialDataView != NULL)
      {
         connect(pSpatialDataView, SIGNAL(layerShown(Layer*)), this, SLOT(updateDisplayedAction(Layer*)));
         connect(pSpatialDataView, SIGNAL(layerHidden(Layer*)), this, SLOT(updateDisplayedAction(Layer*)));
      }
   }
}

View* LayerImp::getView() const
{
   return dynamic_cast<View*>(mpView);
}

vector<ColorType> LayerImp::getColors() const
{
   vector<ColorType> noColors;
   return noColors;
}

bool LayerImp::acceptsMouseEvents() const
{
   return false;
}

QCursor LayerImp::getMouseCursor() const
{
   return QCursor(Qt::ArrowCursor);
}

bool LayerImp::processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                 Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool LayerImp::processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool LayerImp::processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                   Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool LayerImp::processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
                                       Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool LayerImp::linkLayer(Layer* pLayer)
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if ((pLayerImp == NULL) || (pLayerImp == this))
   {
      return false;
   }

   // Do not add the linked layer if it is already linked
   if (isLayerLinked(pLayer) == true)
   {
      return false;
   }

   // Add the window to the linked list
   mLinkedLayers.push_back(pLayer);

   pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &LayerImp::removeLinkedLayer));
   return true;
}

vector<Layer*> LayerImp::getLinkedLayers() const
{
   return mLinkedLayers;
}

bool LayerImp::isLayerLinked(Layer* pLayer) const
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if ((pLayerImp == NULL) || (pLayerImp == this))
   {
      return false;
   }

   for (unsigned int i = 0; i < mLinkedLayers.size(); i++)
   {
      Layer* pLinkedLayer = mLinkedLayers[i];
      if (pLinkedLayer == pLayer)
      {
         return true;
      }
   }

   return false;
}

bool LayerImp::unlinkLayer(Layer* pLayer)
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if ((pLayerImp == NULL) || (pLayerImp == this))
   {
      return false;
   }

   vector<Layer*>::iterator iter = mLinkedLayers.begin();
   while (iter != mLinkedLayers.end())
   {
      Layer* pLinkedLayer = *iter;
      if (pLinkedLayer == pLayer)
      {
         mLinkedLayers.erase(iter);
         return true;
      }

      ++iter;
   }

   return false;
}

bool LayerImp::load(const QString& strFilename)
{
   DataElement* pElement = getDataElement();
   if (pElement == NULL)
   {
      return false;
   }

   // Deserialize the element as an XML file
   bool bSuccess = false;
   MessageLog* pLog = Service<MessageLogMgr>()->getLog();

   XmlReader xml(pLog);

   string filename = strFilename.toStdString();
   FilenameImp fn(filename);

   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDoc = xml.parse(&fn);
   if (pDoc != NULL)
   {
      DOMElement* pRootElement = pDoc->getDocumentElement();
      if (pRootElement != NULL)
      {
         string elementType = A(pRootElement->getAttribute(X("type")));
         unsigned int version = atoi(A(pRootElement->getAttribute(X("version"))));
         if (pElement->isKindOf(elementType) == true)
         {
            bSuccess = pElement->fromXml(pRootElement, version);
         }
      }
   }

   return bSuccess;
}

bool LayerImp::serialize(SessionItemSerializer &serializer) const
{
   XMLWriter xml(getObjectType().c_str());

   if (!toXml(&xml))
   {
      return false;
   }

   return serializer.serialize(xml);
}

bool LayerImp::deserialize(SessionItemDeserializer &deserializer)
{
   XmlReader reader(NULL, false);
   DOMElement *pRoot = deserializer.deserialize(reader, getObjectType().c_str());
   if(pRoot == NULL)
   {
      return false;
   }
   return fromXml(pRoot, XmlBase::VERSION);
}

bool LayerImp::toXml(XMLWriter* pXml) const
{
   pXml->addAttr("name", getName());
   pXml->addAttr("version", XmlBase::VERSION);
   pXml->addAttr("type", static_cast<string>(
      StringUtilities::toXmlString(getLayerType())));
   if(Service<SessionManager>()->isSessionSaving())
   {
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Should we make this a part of exported layers too? (tclarke)")
      if(!SessionItemImp::toXml(pXml))
      {
         return false;
      }

      pXml->addAttr("objectType", getObjectType());
      if (mpView != NULL)
      {
         pXml->addAttr("viewId", mpView->getId());
      }
      if (mpElement.get() != NULL)
      {
         pXml->addAttr("dataElemId", mpElement->getId());
      }
      if (mLinkedLayers.size() > 0)
      {
         for (vector<Layer*>::const_iterator it = mLinkedLayers.begin(); it != mLinkedLayers.end(); ++it)
         {
            if (*it != NULL)
            {
               pXml->addAttr("layerId", (*it)->getId(), pXml->addElement("LinkedLayer"));
            }
         }
         pXml->popAddPoint();
      }
   }
   else
   {
      pXml->addAttr("name", getName());
      string elementType(getDataElement()->getObjectType());
      string::size_type loc(string::npos);
      if((loc = elementType.find("Adapter")) == string::npos)
      {
         loc = elementType.find("Imp");
      }
      if(loc != string::npos)
      {
         elementType.erase(loc, elementType.size());
      }
      pXml->pushAddPoint(pXml->addElement(elementType.c_str()));
      if(!getDataElement()->toXml(pXml))
      {
         return false;
      }
      pXml->popAddPoint();
   }

   if (mXScaleFactor != 1.0 || mYScaleFactor != 1.0)
   {
      stringstream stream;
      stream << mXScaleFactor << " " << mYScaleFactor;
      pXml->addAttr("scale", stream.str());
   }

   if (mXOffset != 0.0 || mYOffset != 0.0)
   {
      stringstream stream;
      stream << mXOffset << " " << mYOffset;
      pXml->addAttr("offset", stream.str());
   }

   return true;
}

bool LayerImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }
   DOMElement *pElem = static_cast<DOMElement*>(pDocument);

   if(Service<SessionManager>()->isSessionLoading())
   {
      if(!SessionItemImp::fromXml(pDocument, version))
      {
         return false;
      }
      if (pElem->hasAttribute(X("viewId")))
      {
         setView(dynamic_cast<ViewImp*>(Service<SessionManager>()->getSessionItem(A(pElem->getAttribute(X("viewId"))))));
      }

      if (pElem->hasAttribute(X("dataElemId")))
      {
         setDataElement(dynamic_cast<DataElement*>(Service<SessionManager>()->getSessionItem(A(pElem->getAttribute(X("dataElemId"))))));
      }
      for(DOMNode *pChld = pElem->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
      {
         if(XMLString::equals(pChld->getNodeName(), X("LinkedLayer")))
         {
            Layer *pLinkedLayer = dynamic_cast<Layer*>(Service<SessionManager>()->getSessionItem(
               A(static_cast<DOMElement*>(pChld)->getAttribute(X("layerId")))));
            if(pLinkedLayer != NULL)
            {
               mLinkedLayers.push_back(pLinkedLayer);
            }
         }
      }
   }
   else
   {
      setName(A(pElem->getAttribute(X("name"))));
      bool success = false;
      for(DOMNode *pChld = pDocument->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
      {
         if(pChld->getNodeType() == DOMNode::ELEMENT_NODE)
         {
            if(mpElement.get() != NULL)
            {
               if(!mpElement->fromXml(pChld, version))
               {
                  return false;
               }
               break;
            }
         }
      }
   }

   if (pElem->hasAttribute(X("scale")))
   {
      LocationType scale(1,1);
      XmlReader::StrToLocation(pElem->getAttribute(X("scale")), scale);
      setXScaleFactor(scale.mX);
      setYScaleFactor(scale.mY);
   }

   if (pElem->hasAttribute(X("offset")))
   {
      LocationType offset(0,0);
      XmlReader::StrToLocation(pElem->getAttribute(X("offset")), offset);
      setXOffset(offset.mX);
      setYOffset(offset.mY);
   }

   return true;
}

void LayerImp::removeLinkedLayer(Subject& subject, const string& signal, const boost::any& value)
{
   Layer* pLayer = dynamic_cast<Layer*>(&subject);
   if (pLayer != NULL)
   {
      unlinkLayer(pLayer);
   }
}

void LayerImp::setDataElement(DataElement* pElement)
{
   if(mpElement.get() != NULL)
   {
      // Decrement the number of layers displaying the element
      if(mElementLayers.contains(mpElement.get()))
      {
         if (--mElementLayers[mpElement.get()] == 0)
         {
            // Remove the element from the map
            mElementLayers.remove(mpElement.get());

            // Delete the element
            ModelServicesImp* pModel(ModelServicesImp::instance());
            pModel->destroyElement(mpElement.get());
         }
      }
   }

   // now set the new element with reference counting
   mpElement.reset(pElement);

   if(mpElement.get() != NULL)
   {
      // Increment the number of layers displaying the element
      if(mElementLayers.contains(mpElement.get()))
      {
         mElementLayers[mpElement.get()]++;
      }
      else
      {
         mElementLayers.insert(mpElement.get(), 1);
      }
   }
}

bool LayerImp::hasUniqueElement() const
{
   if(mpElement.get() == NULL)
   {
      return false;
   }
   QMap<const DataElement*, int>::iterator iter = mElementLayers.find(mpElement.get());
   VERIFY(iter != mElementLayers.end());

   return iter.value() == 1;
}


void LayerImp::setXScaleFactor(double xScaleFactor)
{
   if (mXScaleFactor != xScaleFactor)
   {
      mXScaleFactor = xScaleFactor;
      emit extentsModified();
      notify(SIGNAL_NAME(Layer, ExtentsModified));
   }
}

double LayerImp::getXScaleFactor() const
{
   return mXScaleFactor;
}

void LayerImp::setYScaleFactor(double yScaleFactor)
{
   if (mYScaleFactor != yScaleFactor)
   {
      mYScaleFactor = yScaleFactor;
      emit extentsModified();
      notify(SIGNAL_NAME(Layer, ExtentsModified));
   }
}

double LayerImp::getYScaleFactor() const
{
   return mYScaleFactor;
}

double LayerImp::getXOffset() const
{
   return mXOffset;
}

void LayerImp::setXOffset(double xOffset)
{
   if (mXOffset != xOffset)
   {
      mXOffset = xOffset;
      emit extentsModified();
      notify(SIGNAL_NAME(Layer, ExtentsModified));
   }
}

double LayerImp::getYOffset() const
{
   return mYOffset;
}

void LayerImp::setYOffset(double yOffset)
{
   if (mYOffset != yOffset)
   {
      mYOffset = yOffset;
      emit extentsModified();
      notify(SIGNAL_NAME(Layer, ExtentsModified));
   }
}

void LayerImp::translateWorldToData(double worldX, double worldY, double &dataX, double &dataY) const
{
   dataX = (worldX - mXOffset) / mXScaleFactor;
   dataY = (worldY - mYOffset) / mYScaleFactor;
}

void LayerImp::translateDataToWorld(double dataX, double dataY, double &worldX, double &worldY) const
{
   worldX = dataX * mXScaleFactor + mXOffset;
   worldY = dataY * mYScaleFactor + mYOffset;
}

void LayerImp::translateScreenToData(double screenX, double screenY, double &dataX, double &dataY) const
{
   double worldX = 0;
   double worldY = 0;
   View *pView = getView();
   VERIFYNRV(pView != NULL);

   pView->translateScreenToWorld(screenX, screenY, worldX, worldY);
   translateWorldToData(worldX, worldY, dataX, dataY);
}

void LayerImp::translateDataToScreen(double dataX, double dataY, double &screenX, double &screenY) const
{
   double worldX = 0;
   double worldY = 0;
   View *pView = getView();
   VERIFYNRV(pView != NULL);

   translateDataToWorld(dataX, dataY, worldX, worldY);
   pView->translateWorldToScreen(worldX, worldY, screenX, screenY);
}

void LayerImp::isFlipped(const LocationType& dataLowerLeft, const LocationType& dataUpperRight,
                         bool& bHorizontalFlip, bool& bVerticalFlip) const
{
   LocationType screenLowerLeft;
   LocationType screenLowerRight;
   LocationType screenUpperLeft;
   translateDataToScreen(dataLowerLeft.mX, dataLowerLeft.mY, screenLowerLeft.mX, screenLowerLeft.mY);
   translateDataToScreen(dataUpperRight.mX, dataLowerLeft.mY, screenLowerRight.mX, screenLowerRight.mY);
   translateDataToScreen(dataLowerLeft.mX, dataUpperRight.mY, screenUpperLeft.mX, screenUpperLeft.mY);

   bHorizontalFlip = screenLowerLeft.mX > screenLowerRight.mX;
   bVerticalFlip = screenLowerLeft.mY > screenUpperLeft.mY;
}

LayerImp& LayerImp::operator= (const LayerImp& rhs)
{
   if (&rhs != this)
   {
      // Do not call the base class operator= since its values (i.e. name) will be
      // populated by this layer and therefore do not need to be copied
      mXScaleFactor = rhs.mXScaleFactor;
      mYScaleFactor = rhs.mYScaleFactor;
      mXOffset = rhs.mXOffset;
      mYOffset = rhs.mYOffset;

      notify(SIGNAL_NAME(Subject, Modified));
   }
   return *this;
}

bool LayerImp::canRename() const
{
   return true;
}

bool LayerImp::rename(const string &newName, string &errorMessage)
{
   Layer* pLayer = dynamic_cast<Layer*>(this);
   if (pLayer == NULL)
   {
      return false;
   }

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      LayerList* pList = pView->getLayerList();
      if (pList != NULL)
      {
         return pList->renameLayer(pLayer, newName);
      }
   }
   return false;
}

void LayerImp::showLayer(bool show)
{
   Layer* pLayer = dynamic_cast<Layer*>(this);
   SpatialDataView* pSDV = dynamic_cast<SpatialDataView*>(getView());
   if (pSDV != NULL && pLayer != NULL)
   {
      if (show)
      {
         pSDV->showLayer(pLayer);
      }
      else
      {
         pSDV->hideLayer(pLayer);
      }
   }
}

void LayerImp::updateDisplayedAction(Layer* pLayer)
{
   Layer* pThisLayer = dynamic_cast<Layer*>(this);
   if (pLayer != pThisLayer)
   {
      return;
   }

   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(getView());
   if (pView != NULL)
   {
      mpDisplayedAction->setChecked(pView->isLayerDisplayed(pThisLayer));
   }
}
