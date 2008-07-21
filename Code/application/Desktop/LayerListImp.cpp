/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include "LayerListImp.h"
#include "AnnotationElement.h"
#include "AnnotationLayerAdapter.h"
#include "AoiElement.h"
#include "AoiLayerAdapter.h"
#include "AppAssert.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "DataElement.h"
#include "DesktopServicesImp.h"
#include "GcpLayerAdapter.h"
#include "GcpList.h"
#include "LatLonLayerAdapter.h"
#include "LayerListAdapter.h"
#include "ModelServices.h"
#include "PseudocolorLayerAdapter.h"
#include "RasterLayerAdapter.h"
#include "RasterDataDescriptor.h"
#include "StringUtilities.h"
#include "ThresholdLayerAdapter.h"
#include "TiePointLayerAdapter.h"

#include <algorithm>
#include <sstream>
using namespace std;

LayerListImp::LayerListImp()
{
}

LayerListImp::~LayerListImp()
{
   clear();
}

const string& LayerListImp::getObjectType() const
{
   static string type("LayerListImp");
   return type;
}

bool LayerListImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LayerList"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

bool LayerListImp::setPrimaryRasterElement(RasterElement* pRasterElement)
{
   if (pRasterElement != mpRasterElement.get())
   {
      mpRasterElement.reset(pRasterElement);
      notify(SIGNAL_NAME(Subject, Modified));
      return true;
   }

   return false;
}

RasterElement* LayerListImp::getPrimaryRasterElement() const
{
   return const_cast<RasterElement*>(mpRasterElement.get());
}

QString LayerListImp::getPrimaryRasterElementName() const
{
   if (mpRasterElement.get() != NULL)
   {
      string name = mpRasterElement->getName();
      if (name.empty() == false)
      {
         return QString::fromStdString(name);
      }
   }

   return QString();
}

Layer* LayerListImp::newLayer(const LayerType& layerType, DataElement* pElement, const QString& strLayerName)
{
   QString strName = strLayerName;

   // Get the layer name from the element
   if ((strName.isEmpty() == true) && (pElement != NULL))
   {
      string elementName = pElement->getName();
      if (elementName.empty() == false)
      {
         strName = QString::fromStdString(elementName);
      }
   }

   // Ensure that the layer name is unique
   QString strLayerType;

   string layerTypeText = StringUtilities::toDisplayString(layerType);
   if (layerTypeText.empty() == false)
   {
      strLayerType = QString::fromStdString(layerTypeText);
   }

   unsigned int numLayers = getNumLayers(layerType);
   while (isLayerNameUnique(strName, layerType) == false)
   {
      strName = strLayerType + " " + QString::number(++numLayers);
   }

   if (strName.isEmpty() == true)
   {
      return NULL;
   }

   // Create the element if necessary
   if (pElement == NULL)
   {
      QString strModelType;
      if (layerType == ANNOTATION)
      {
         strModelType = "AnnotationElement";
      }
      if (layerType == AOI_LAYER)
      {
         strModelType = "AoiElement";
      }
      else if (layerType == GCP_LAYER)
      {
         strModelType = "GcpList";
      }
      else if (layerType == TIEPOINT_LAYER)
      {
         strModelType = "TiePointList";
      }
      else if (layerType == PSEUDOCOLOR)
      {
         strModelType = "RasterElement";
      }

      if (strModelType.isEmpty() == false)
      {
         string elementName = strName.toStdString();
         string elementType = strModelType.toStdString();

         Service<ModelServices> pModel;
         pElement = pModel->createElement(elementName, elementType, mpRasterElement.get());
      }

      if (pElement == NULL)
      {
         return NULL;
      }
   }

   // Do not create the layer if it already exists
   Layer* pLayer = NULL;
   pLayer = getLayer(layerType, pElement, strName);
   if (pLayer != NULL)
   {
      return NULL;
   }

   // create error msg in case data element is of wrong type
   string msg = "Can't create new ";
   msg += StringUtilities::toXmlString(layerType);
   msg += " layer using " + pElement->getObjectType();

   // Create the layer
   switch (layerType)
   {
      case ANNOTATION:
      {
         AnnotationElement* pAE = dynamic_cast<AnnotationElement*>(pElement);
         if (pAE != NULL)
         {
            pLayer = new AnnotationLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pAE);
         }
         break;
      }

      case AOI_LAYER:
      {
         AoiElement* pAE = dynamic_cast<AoiElement*>(pElement);
         if (pAE != NULL)
         {
            pLayer = new AoiLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pAE);
         }
         break;
      }

      case GCP_LAYER:
      {
         GcpList* pGL = dynamic_cast<GcpList*>(pElement);
         if (pGL != NULL)
         {
            pLayer = new GcpLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pGL);
         }
         break;
      }

      case LAT_LONG:
      {
         RasterElement* pRE = dynamic_cast<RasterElement*>(pElement);
         if (pRE != NULL)
         {
            pLayer = new LatLonLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pRE);
         }
         break;
      }

      case PSEUDOCOLOR:
      {
         RasterElement* pRE = dynamic_cast<RasterElement*>(pElement);
         if (pRE != NULL)
         {
            pLayer = new PseudocolorLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pRE);
         }
         break;
      }

      case RASTER:
      {
         RasterElement* pRE = dynamic_cast<RasterElement*>(pElement);
         if (pRE != NULL)
         {
            pLayer = new RasterLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pRE);
         }
         break;
      }

      case THRESHOLD:
      {
         RasterElement* pRE = dynamic_cast<RasterElement*>(pElement);
         if (pRE != NULL)
         {
            pLayer = new ThresholdLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pRE);
         }
         break;
      }

      case TIEPOINT_LAYER:
      {
         TiePointList* pTL = dynamic_cast<TiePointList*>(pElement);
         if (pTL != NULL)
         {
            pLayer = new TiePointLayerAdapter(SessionItemImp::generateUniqueId(), 
               strName.toStdString(), pTL);
         }
         break;
      }

      default:
         return NULL;
   }

   VERIFYRV_MSG(pLayer != NULL, NULL, msg.c_str());

   // get default scale factor
   if (mpRasterElement.get() != NULL)
   {
      double xFactor = 1;
      double yFactor = 1;

      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         yFactor = pDescriptor->getYPixelSize();
         xFactor = pDescriptor->getXPixelSize();
      }

      const RasterElement *pRasterElement = dynamic_cast<const RasterElement*>(pElement);
      if (pRasterElement != NULL && mpRasterElement.get() != pRasterElement)
      {
         pDescriptor = dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            yFactor *= pDescriptor->getYPixelSize();
            xFactor *= pDescriptor->getXPixelSize();
         }
      }

      pLayer->setXScaleFactor(xFactor);
      pLayer->setYScaleFactor(yFactor);
   }

   return pLayer;
}

bool LayerListImp::addLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   // Do not add the layer if it already exists
   if (containsLayer(pLayer) == true)
   {
      return false;
   }

   mLayers.push_back(pLayer);
   emit layerAdded(pLayer);
   notify(SIGNAL_NAME(LayerList, LayerAdded), boost::any(pLayer));
   return true;
}

bool LayerListImp::containsLayer(Layer* pLayer) const
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::const_iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   return (iter != mLayers.end());
}

Layer* LayerListImp::getLayer(const LayerType& layerType, const DataElement* pElement,
                              const QString& strLayerName) const
{
   QString strName = strLayerName;
   if ((strName.isEmpty() == true) && (pElement != NULL))
   {
      string elementName = pElement->getName();
      if (elementName.empty() == false)
      {
         strName = QString::fromStdString(elementName);
      }
   }

   for (unsigned int i = 0; i < mLayers.size(); i++)
   {
      Layer* pLayer = mLayers[i];
      if (pLayer != NULL)
      {
         QString strCurrentName = QString::fromStdString(pLayer->getName());
         DataElement* pCurrentElement = pLayer->getDataElement();
         LayerType currentType = pLayer->getLayerType();

         if ((strCurrentName == strName) && (pElement == NULL || pCurrentElement == pElement) && (currentType == layerType))
         {
            return pLayer;
         }
      }
   }

    return NULL;
}

vector<Layer*> LayerListImp::getLayers(const LayerType& layerType) const
{
   vector<Layer*> layers;
   for (unsigned int i = 0; i < mLayers.size(); i++)
   {
      Layer* pLayer = mLayers[i];
      if (pLayer != NULL)
      {
         LayerType currentType = pLayer->getLayerType();
         if (currentType == layerType)
         {
            layers.push_back(pLayer);
         }
      }
   }

    return layers;
}

vector<Layer*> LayerListImp::getLayers() const
{
   return mLayers;
}

unsigned int LayerListImp::getNumLayers(const LayerType& layerType) const
{
   vector<Layer*> layers = getLayers(layerType);
   return layers.size();
}

unsigned int LayerListImp::getNumLayers() const
{
   return mLayers.size();
}

bool LayerListImp::renameLayer(Layer* pLayer, const QString& strNewName) const
{
   LayerImp* pLayerImp = dynamic_cast<LayerImp*>(pLayer);
   if (pLayerImp == NULL)
   {
      return false;
   }

   // Get a unique name for the layer
   QString strName = QString::fromStdString(pLayerImp->getName());
   LayerType eType = pLayerImp->getLayerType();

   QString strLayerName = strNewName;
   if (strLayerName.isEmpty() == false)
   {
      bool bUnique = isLayerNameUnique(strLayerName, eType);
      if (bUnique == false)
      {
         return false;
      }
   }
   else
   {
      strLayerName = getUniqueLayerName(strName, eType);
      if (strLayerName.isEmpty() == true)
      {
         return false;
      }
   }

   // Rename the layer
   pLayerImp->setName(strLayerName.toStdString());

   return true;
}

bool LayerListImp::isLayerNameUnique(const QString& strLayerName, const LayerType& layerType) const
{
   if (strLayerName.isEmpty() == true)
   {
      return false;
   }

   vector<Layer*> layers = getLayers(layerType);
   for (unsigned int i = 0; i < layers.size(); i++)
   {
      Layer* pLayer = layers[i];
      if (pLayer != NULL)
      {
         QString strCurrentName = QString::fromStdString(pLayer->getName());
         if (strCurrentName == strLayerName)
         {
            return false;
         }
      }
   }

   return true;
}

QString LayerListImp::getUniqueLayerName(const QString& strInitialName, const LayerType& layerType) const
{
   QString strCaption = "Rename Layer";
   QString strLayerType;

   string layerTypeText = StringUtilities::toDisplayString(layerType);
   if (layerTypeText.empty() == false)
   {
      strLayerType = QString::fromStdString(layerTypeText);
   }

   if (strLayerType.isEmpty() == false)
   {
      strCaption.insert(7, strLayerType + " ");
   }

   QString strNewName;
   while (strNewName.isEmpty() == true)
   {
      bool bSuccess = false;
      QWidget* pParent = NULL;

      DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
      if (pDesktop != NULL)
      {
         pParent = pDesktop->getMainWidget();
      }

      strNewName = QInputDialog::getText(pParent, strCaption, "Name:", QLineEdit::Normal, strInitialName, &bSuccess);
      if (bSuccess == true)
      {
         bool bUnique = false;
         bUnique = isLayerNameUnique(strNewName, layerType);
         if (bUnique == false)
         {
            QMessageBox::warning(pParent, APP_NAME, "The '" + strNewName +
               "' name already exists.  Please select a unique name.");
            strNewName.clear();
         }
      }
      else
      {
         return QString();
      }
   }

   return strNewName;
}

bool LayerListImp::deleteLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   if (containsLayer(pLayer) == false)
   {
      return false;
   }

   // Remove the layer from the displayed layers vector
   hideLayer(pLayer);

   // Remove the layer
   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter != mLayers.end())
   {
      mLayers.erase(iter);
      emit layerDeleted(pLayer);
      notify(SIGNAL_NAME(LayerList, LayerDeleted), boost::any(pLayer));
      delete dynamic_cast<LayerImp*>(pLayer);
   }

   return true;
}

void LayerListImp::clear()
{
   vector<Layer*> layers = getLayers();
   for (unsigned int i = 0; i < layers.size(); i++)
   {
      Layer* pLayer = layers[i];
      if (pLayer != NULL)
      {
         deleteLayer(pLayer);
      }
   }
}

bool LayerListImp::showLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   if (isLayerDisplayed(pLayer) == true)
   {
      return false;
   }

   mDisplayedLayers.push_back(pLayer);
   return true;
}

bool LayerListImp::hideLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mDisplayedLayers.begin(), mDisplayedLayers.end(), pLayer);
   if (iter != mDisplayedLayers.end())
   {
      mDisplayedLayers.erase(iter);
      return true;
   }

   return false;
}

bool LayerListImp::isLayerDisplayed(Layer* pLayer) const
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::const_iterator iter = find(mDisplayedLayers.begin(), mDisplayedLayers.end(), pLayer);
   return (iter != mDisplayedLayers.end());
}

vector<Layer*> LayerListImp::getDisplayedLayers() const
{
   // Do not return the member vector since display ordering is not maintained
   vector<Layer*> displayedLayers;
   for (vector<Layer*>::const_iterator iter = mLayers.begin(); iter != mLayers.end(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL)
      {
         if (isLayerDisplayed(pLayer) == true)
         {
            displayedLayers.push_back(pLayer);
         }
      }
   }

   return displayedLayers;
}

unsigned int LayerListImp::getNumDisplayedLayers() const
{
   return mDisplayedLayers.size();
}

bool LayerListImp::setFrontLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter != mLayers.end())
   {
      if (pLayer != mLayers.back())
      {
         mLayers.erase(iter);
         mLayers.push_back(pLayer);
         return true;
      }
   }

   return false;
}

bool LayerListImp::setBackLayer(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter != mLayers.end())
   {
      if (pLayer != mLayers.front())
      {
         mLayers.erase(iter);
         mLayers.insert(mLayers.begin(), pLayer);
         return true;
      }
   }

   return false;
}

bool LayerListImp::bringLayerForward(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter != mLayers.end())
   {
      if (pLayer != mLayers.back())
      {
         iter = mLayers.erase(iter);
         mLayers.insert(++iter, pLayer);
         return true;
      }
   }

   return false;
}

bool LayerListImp::sendLayerBackward(Layer* pLayer)
{
   if (pLayer == NULL)
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter != mLayers.end())
   {
      if (pLayer != mLayers.front())
      {
         iter = mLayers.erase(iter);
         mLayers.insert(--iter, pLayer);
         return true;
      }
   }

   return false;
}

bool LayerListImp::setLayerDisplayIndex(Layer* pLayer, int iIndex)
{
   if ((pLayer == NULL) || (iIndex < 0))
   {
      return false;
   }

   vector<Layer*>::iterator iter = find(mLayers.begin(), mLayers.end(), pLayer);
   if (iter == mLayers.end())
   {
      return false;
   }

   int iCurrentIndex = getLayerDisplayIndex(pLayer);
   if (iCurrentIndex == iIndex)
   {
      return true;
   }

   // Remove the layer whose index is changing
   mLayers.erase(iter);

   // Insert the layer in the displayed layers vector at the given index
   int iCount = mLayers.size();
   if (iIndex > iCount)
   {
      iIndex = iCount;
   }

   if (iIndex == 0)
   {
      mLayers.push_back(pLayer);
   }
   else
   {
      mLayers.insert(mLayers.begin() + (iCount - iIndex), pLayer);
   }

   return true;
}

int LayerListImp::getLayerDisplayIndex(Layer* pLayer) const
{
   if (pLayer == NULL)
   {
      return -1;
   }

   vector<Layer*>::const_iterator iter;
   int index = 0;

   for (iter = mLayers.begin(); iter != mLayers.end(); ++iter, index++)
   {
      Layer* pCurrentLayer = *iter;
      if (pCurrentLayer == pLayer)
      {
         return (static_cast<int>(mLayers.size()) - index - 1);
      }
   }

   return -1;
}

Layer* LayerListImp::getTopMostLayer(const LayerType& layerType) const
{
   for (vector<Layer*>::const_reverse_iterator iter = mLayers.rbegin(); iter != mLayers.rend(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL)
      {
         if (pLayer->getLayerType() == layerType)
         {
            if (isLayerDisplayed(pLayer) == true)
            {
               return pLayer;
            }
         }
      }
   }

   return NULL;
}

Layer* LayerListImp::getTopMostLayer() const
{
   for (vector<Layer*>::const_reverse_iterator iter = mLayers.rbegin(); iter != mLayers.rend(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL)
      {
         if (isLayerDisplayed(pLayer) == true)
         {
            return pLayer;
         }
      }
   }

   return NULL;
}

DataElement* LayerListImp::getTopMostElement(LayerType layerType) const
{
   Layer* pLayer = getTopMostLayer(layerType);
   if (pLayer != NULL)
   {
      DataElement* pElement = pLayer->getDataElement();
      return pElement;
   }

   return NULL;
}

DataElement* LayerListImp::getTopMostElement(const string& elementType) const
{
   if (elementType.empty() == true)
   {
      return NULL;
   }

   for (vector<Layer*>::const_reverse_iterator iter = mLayers.rbegin(); iter != mLayers.rend(); ++iter)
   {
      Layer* pLayer = *iter;
      if (pLayer != NULL)
      {
         if (isLayerDisplayed(pLayer) == true)
         {
            DataElement* pElement = pLayer->getDataElement();
            if (pElement != NULL)
            {
               if (pElement->isKindOf(elementType) == true)
               {
                  return pElement;
               }
            }
         }
      }
   }

   return NULL;
}

DataElement* LayerListImp::getTopMostElement() const
{
   Layer* pLayer = getTopMostLayer();
   if (pLayer != NULL)
   {
      DataElement* pElement = pLayer->getDataElement();
      return pElement;
   }

   return NULL;
}
