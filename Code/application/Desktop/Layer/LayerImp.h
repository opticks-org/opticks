/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERIMP_H
#define LAYERIMP_H

#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtGui/QAction>

#include "AttachmentPtr.h"
#include "ColorType.h"
#include "DataElement.h"
#include "SessionItemImp.h"
#include "SubjectImp.h"
#include "TypesFile.h"
#include "XercesIncludes.h"
#include "xmlwriter.h"

#include <string>
#include <vector>

class DataElement;
class Layer;
class SessionItemDeserializer;
class SessionItemSerializer;
class View;
class ViewImp;

Q_DECLARE_METATYPE(Layer*)

class LayerImp : public QObject, public SessionItemImp, public SubjectImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   virtual ~LayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void elementModified(Subject& subject, const std::string& signal, const boost::any& v);
   virtual void onElementModified() {}
   void elementDeleted(Subject& subject, const std::string& signal, const boost::any& v);

   LayerImp& operator= (const LayerImp& rhs);

   virtual LayerType getLayerType() const = 0;

   void setName(const std::string& layerName);
   DataElement* getDataElement() const;
   bool hasUniqueElement() const;

   SIGNAL_METHOD(LayerImp, ViewModified)
   void setView(ViewImp* pView);
   View* getView() const;

   virtual std::vector<ColorType> getColors() const;

   virtual void draw() = 0;
   virtual bool getExtents(double& minX, double& minY, double& maxX, double& maxY) = 0;

   virtual bool acceptsMouseEvents() const;
   virtual QCursor getMouseCursor() const;

   virtual bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   virtual bool processMouseDoubleClick(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

   bool linkLayer(Layer* pLayer);
   std::vector<Layer*> getLinkedLayers() const;
   bool isLayerLinked(Layer* pLayer) const;
   bool unlinkLayer(Layer* pLayer);

   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);
   virtual bool toXml(XMLWriter* pXml) const;
   virtual bool fromXml(DOMNode* pDocument, unsigned int version);

   void setXScaleFactor(double xScaleFactor);
   double getXScaleFactor() const;
   void setYScaleFactor(double yScaleFactor);
   double getYScaleFactor() const;
   double getXOffset() const;
   void setXOffset(double xOffset);
   double getYOffset() const;
   void setYOffset(double yOffset);

   void translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const;
   void translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const;
   void translateScreenToData(double screenX, double screenY, double& dataX, double& dataY) const;
   void translateDataToScreen(double dataX, double dataY, double& screenX, double& screenY) const;

   void isFlipped(const LocationType& dataLowerLeft, const LocationType& dataUpperRight, bool& bHorizontalFlip,
      bool& bVerticalFlip) const;

   bool canRename() const;
   bool rename(const std::string& newName, std::string& errorMessage);

public slots:
   /**
    *  Sets the layer properties back to their default values.
    */
   virtual void reset() = 0;

signals:
   void nameChanged(const QString& strName);
   void viewModified(ViewImp* pView);
   void modified();
   void extentsModified();

protected:
   LayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   void removeLinkedLayer(Subject& subject, const std::string& signal, const boost::any& value);

protected:
   bool mbLinking;
   AttachmentPtr<DataElement> mpElement;
   virtual void layerActivated(bool activated) { }
   QAction* mpSubsetStatisticsAction;

protected slots:
   void showLayer(bool show);
   void calculateSubsetStatistics();
   void updateDisplayedAction(Layer* pLayer);
   void layerActivated(Layer* pLayer);

private:
   void setDataElement(DataElement* pElement);

private:
   ViewImp* mpView;
   std::vector<Layer*> mLinkedLayers;
   double mXScaleFactor;
   double mYScaleFactor;
   double mXOffset;
   double mYOffset;
   QAction* mpDisplayedAction;

   static QMap<const DataElement*, int> mElementLayers;
};

#define LAYERADAPTEREXTENSION_CLASSES \
   SESSIONITEMADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define LAYERADAPTER_METHODS(impClass) \
   SESSIONITEMADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   LayerType getLayerType() const \
   { \
      return impClass::getLayerType(); \
   } \
   DataElement* getDataElement() const \
   { \
      return impClass::getDataElement(); \
   } \
   bool hasUniqueElement() const \
   { \
      return impClass::hasUniqueElement(); \
   } \
   View* getView() const \
   { \
      return impClass::getView(); \
   } \
   bool linkLayer(Layer* pLayer) \
   { \
      return impClass::linkLayer(pLayer); \
   } \
   void getLinkedLayers(std::vector<Layer*>& linkedLayers) const \
   { \
      linkedLayers = impClass::getLinkedLayers(); \
   } \
   bool isLayerLinked(Layer* pLayer) const \
   { \
      return impClass::isLayerLinked(pLayer); \
   } \
   bool unlinkLayer(Layer* pLayer) \
   { \
      return impClass::unlinkLayer(pLayer); \
   } \
   bool getExtents(double& minX, double& minY, double& maxX, double& maxY) \
   { \
      return impClass::getExtents(minX, minY, maxX, maxY); \
   } \
   void setXScaleFactor(double xScaleFactor) \
   { \
      return impClass::setXScaleFactor(xScaleFactor); \
   } \
   double getXScaleFactor() const \
   { \
      return impClass::getXScaleFactor(); \
   } \
   void setYScaleFactor(double yScaleFactor) \
   { \
      return impClass::setYScaleFactor(yScaleFactor); \
   } \
   double getYScaleFactor() const \
   { \
      return impClass::getYScaleFactor(); \
   } \
   double getXOffset() const \
   { \
      return impClass::getXOffset(); \
   } \
   void setXOffset(double xOffset) \
   { \
      return impClass::setXOffset(xOffset); \
   } \
   double getYOffset() const \
   { \
      return impClass::getYOffset(); \
   } \
   void setYOffset(double yOffset) \
   { \
      return impClass::setYOffset(yOffset); \
   } \
   void translateWorldToData(double worldX, double worldY, double& dataX, double& dataY) const \
   { \
      return impClass::translateWorldToData(worldX, worldY, dataX, dataY); \
   } \
   void translateDataToWorld(double dataX, double dataY, double& worldX, double& worldY) const \
   { \
      return impClass::translateDataToWorld(dataX, dataY, worldX, worldY); \
   } \
   void translateScreenToData(double screenX, double screenY, double& dataX, double& dataY) const \
   { \
      return impClass::translateScreenToData(screenX, screenY, dataX, dataY); \
   } \
   void translateDataToScreen(double dataX, double dataY, double& screenX, double& screenY) const \
   { \
      return impClass::translateDataToScreen(dataX, dataY, screenX, screenY); \
   } \
   void isFlipped(const LocationType& dataLowerLeft, const LocationType& dataUpperRight, bool& bHorizontalFlip, \
      bool& bVerticalFlip) const \
   { \
      return impClass::isFlipped(dataLowerLeft, dataUpperRight, bHorizontalFlip, bVerticalFlip); \
   }

#endif
