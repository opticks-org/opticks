/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LAYERLISTIMP_H
#define LAYERLISTIMP_H

#include <QtCore/QObject>

#include "AttachmentPtr.h"
#include "RasterElement.h"
#include "SubjectImp.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class Layer;
class LayerList;
class DataElement;

class LayerListImp : public QObject, public SubjectImp
{
   Q_OBJECT

public:
   LayerListImp();
   ~LayerListImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool setPrimaryRasterElement(RasterElement* pRasterElement);
   RasterElement* getPrimaryRasterElement() const;
   QString getPrimaryRasterElementName() const;

   Layer* newLayer(const LayerType& layerType, DataElement* pElement, const QString& strLayerName);
   bool addLayer(Layer* pLayer);
   bool containsLayer(Layer* pLayer) const;
   Layer* getLayer(const LayerType& layerType, const DataElement* pElement,
      const QString& strLayerName = QString()) const;
   std::vector<Layer*> getLayers(const LayerType& layerType) const;
   std::vector<Layer*> getLayers() const;
   unsigned int getNumLayers(const LayerType& layerType) const;
   unsigned int getNumLayers() const;
   bool renameLayer(Layer* pLayer, const QString& strNewName = QString()) const;
   bool isLayerNameUnique(const QString& strLayerName, const LayerType& layerType) const;
   QString getUniqueLayerName(const QString& strInitialName, const LayerType& layerType) const;
   bool deleteLayer(Layer* pLayer);
   void clear();

   bool showLayer(Layer* pLayer);
   bool hideLayer(Layer* pLayer);
   bool isLayerDisplayed(Layer* pLayer) const;
   std::vector<Layer*> getDisplayedLayers() const;
   unsigned int getNumDisplayedLayers() const;

   bool setFrontLayer(Layer* pLayer);
   bool setBackLayer(Layer* pLayer);
   bool bringLayerForward(Layer* pLayer);
   bool sendLayerBackward(Layer* pLayer);
   bool setLayerDisplayIndex(Layer* pLayer, int iIndex);
   int getLayerDisplayIndex(Layer* pLayer) const;

   Layer* getTopMostLayer(const LayerType& layerType) const;
   Layer* getTopMostLayer() const;
   DataElement* getTopMostElement(LayerType layerType) const;
   DataElement* getTopMostElement(const std::string& elementType) const;
   DataElement* getTopMostElement() const;

signals:
   void layerAdded(Layer* pLayer);
   void layerDeleted(Layer* pLayer);

private:
   AttachmentPtr<RasterElement> mpRasterElement;
   std::vector<Layer*> mLayers;
   std::vector<Layer*> mDisplayedLayers;
};

#define LAYERLISTADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define LAYERLISTADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass) \
   RasterElement* getPrimaryRasterElement() const \
   { \
      return impClass::getPrimaryRasterElement(); \
   } \
   bool containsLayer(Layer* pLayer) const \
   { \
      return impClass::containsLayer(pLayer); \
   } \
   Layer* getLayer(const LayerType& layerType, const DataElement* pElement) const \
   { \
      return impClass::getLayer(layerType, pElement); \
   } \
   Layer* getLayer(const LayerType& layerType, const DataElement* pElement, const std::string& layerName) const \
   { \
      QString strLayerName; \
      if (layerName.empty() == false) \
      { \
         strLayerName = QString::fromStdString(layerName); \
      } \
      return impClass::getLayer(layerType, pElement, strLayerName); \
   } \
   void getLayers(const LayerType& layerType, std::vector<Layer*>& layers) const \
   { \
      layers = impClass::getLayers(layerType); \
   } \
   void getLayers(std::vector<Layer*>& layers) const \
   { \
      layers = impClass::getLayers(); \
   } \
   unsigned int getNumLayers(const LayerType& layerType) const \
   { \
      return impClass::getNumLayers(layerType); \
   } \
   unsigned int getNumLayers() const \
   { \
      return impClass::getNumLayers(); \
   } \
   bool renameLayer(Layer* pLayer) const \
   { \
      return impClass::renameLayer(pLayer); \
   } \
   bool renameLayer(Layer* pLayer, const std::string& newName) const \
   { \
      QString strNewName; \
      if (newName.empty() == false) \
      { \
         strNewName = QString::fromStdString(newName); \
      } \
      return impClass::renameLayer(pLayer, strNewName); \
   }

#endif
