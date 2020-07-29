/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AoiLayer.h"
#include "AoiLayerImp.h"
#include "ApplicationServices.h"
#include "assert.h"
#include "BitMask.h"
#include "ConfigurationSettingsImp.h"
#include "DesktopServicesImp.h"
#include "DimensionObject.h"
#include "FilenameImp.h"
#include "GraphicGroup.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "MultipointObject.h"
#include "ObjectFactory.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PolygonObject.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerAdapter.h"
#include "PseudocolorLayerImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RectangleObject.h"
#include "Resource.h"
#include "SpatialDataViewAdapter.h"
#include "SpatialDataWindowAdapter.h"
#include "SpatialDataWindowImp.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "ThresholdLayer.h"
#include "ThresholdLayerImp.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"

#include <QtCore/QCoreApplication>

#include <algorithm>
#include <math.h>
#include <sstream>

XERCES_CPP_NAMESPACE_USE

using namespace std;

class AoiObserver  // used by AoiNotifyPointsChangedTest 
{
public:
   AoiObserver() :
      mNumPointsChangedNotifications(0),
      mNumSubjectModifiedNotifications(0)
   {}

   virtual ~AoiObserver() {} 
   void aoiChanged(Subject &subject, const string &signal, const boost::any &data)
   {  
      if (signal == SIGNAL_NAME(AoiElement, PointsChanged))
      {
         ++mNumPointsChangedNotifications;
      }
      else if (signal == SIGNAL_NAME(Subject, Modified))
      {
         ++mNumSubjectModifiedNotifications;
      }
   }

   void clearNotifications()
   {
      mNumPointsChangedNotifications = 0;
      mNumSubjectModifiedNotifications = 0;
   }

   unsigned int getNumberOfPointsChangedNotifications() const
   {
      return mNumPointsChangedNotifications;
   }

   unsigned int getNumberOfSubjectModifiedNotifications() const
   {
      return mNumSubjectModifiedNotifications;
   }

private:
   unsigned int mNumPointsChangedNotifications;
   unsigned int mNumSubjectModifiedNotifications;

};

class AoiDerivationTestCase : public TestCase
{
public:
   AoiDerivationTestCase() : TestCase("Derivation") {}
   bool run()
   {
      bool success = true;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow *pSdwd = NULL;
      pSdwd = dynamic_cast<SpatialDataWindow*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pSdwd != NULL);

      SpatialDataView *pSdw = NULL;
      pSdw = dynamic_cast<SpatialDataView*>(pSdwd->getView());
      issea(pSdw != NULL);

      ThresholdLayer *pProperties = NULL;
      if (success)
      {
         pProperties = TestUtilities::createThresholdLayer(pRasterElement, 30, 45);
      }
      issea(pProperties != NULL);
      if (pProperties == NULL)
      {
         return false;
      }

      LayerList *pLayerList = NULL;
      pLayerList = pSdw->getLayerList();
      issea(pLayerList != NULL);

      string layerName = pProperties->getName();
      issea(layerName != "");

      ThresholdLayer *pThresholdOverlay = NULL;
      if (success)
      {
         pThresholdOverlay = dynamic_cast<ThresholdLayer*>(pLayerList->getLayer(pProperties->getLayerType(), pProperties->getDataElement(), layerName));
      }
      issea(pThresholdOverlay != NULL);

      AoiLayer *pAoiOverlay = NULL;
      if (success)
      {
         pAoiOverlay = dynamic_cast<AoiLayer*>(pSdw->deriveLayer((const Layer*)pThresholdOverlay, AOI_LAYER));
      }
      issea(pAoiOverlay != NULL);

      AoiElement *pAoi = NULL;
      if (success)
      {
         pAoi = static_cast<AoiElement*>(pAoiOverlay->getDataElement());
      }
      issea(pAoi != NULL);

      const BitMask *pMask = NULL;
      issea(pMask = pAoi->getSelectedPoints());
      int count = 0;
      if (success)
      {
         count = pMask->getCount();
      }
      issea(count == 45);

      if (success)
      {
         pSdw->deleteLayer(pAoiOverlay);
      }

      PseudocolorLayer *pPseudoOverlay = NULL;
      if (success)
      {
         pPseudoOverlay = dynamic_cast<PseudocolorLayer*>(pSdw->convertLayer(dynamic_cast<Layer*>(pThresholdOverlay), PSEUDOCOLOR));
         issea(pPseudoOverlay != NULL);
         if (pPseudoOverlay == NULL)
         {
            return false;
         }
      }
      issea(pPseudoOverlay != NULL);

      pThresholdOverlay = NULL;
      if (success)
      {
         for (int i = 0; i < 30; ++i)
         {
            stringstream name;
            name << i;
            int x = pPseudoOverlay->addInitializedClass(name.str(), i, ColorType(i, 255 - i, 127 + i), true);
            issea(x != -1);
         }
      }

      pAoiOverlay = NULL;
      if (success)
      {
         pAoiOverlay = dynamic_cast<AoiLayer*>(pSdw->deriveLayer(pPseudoOverlay, AOI_LAYER));
      }
      issea(pAoiOverlay != NULL);

      pAoi = NULL;
      if (success)
      {
         pAoi = static_cast<AoiElement*>(pAoiOverlay->getDataElement());
      }
      issea(pAoi != NULL);
      issea(pAoi->getSelectedPoints()->getCount() == 17512);

      if (pSdw != NULL)
      {
         success = tst_assert(pSdw->deleteLayer(pAoiOverlay)) && success;
         success = tst_assert(pSdw->deleteLayer(pPseudoOverlay)) && success;
      }

      return success;
   }
};

class AoiSerializeDeserializeTest : public TestCase
{
public:
   AoiSerializeDeserializeTest() : TestCase("Serialize") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 120), LocationType(40, 140));
      QCoreApplication::instance()->processEvents();

      bool ok = false;
      // serialize the AOI

      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen((tempHome + "/AOI.aoi").c_str(), "w");
      issea(pOutputFile != NULL);

      XMLWriter xwrite("AoiElement");
      ok = pAoi->toXml(&xwrite);
      issea(ok == true);
      xwrite.writeToFile(pOutputFile);
      fclose(pOutputFile);
      issea(ok == true);

      ModelServicesImp::instance()->setElementName(pAoi, "AoiSerialized");

      // create a new AOI to deserialize to
      AoiElement* pAoiCopy = NULL;
      pAoiCopy = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoiCopy != NULL);

      // deserialize the layer
      MessageLog *pLog(UtilityServicesImp::instance()->getMessageLog()->getLog("session"));
      XmlReader xml(pLog);

      FilenameImp aoiFile(tempHome + "/AOI.aoi");

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *aoiDoc(NULL);
      try
      {
         aoiDoc = xml.parse(&aoiFile);
      }
      catch(XmlBase::XmlException &)
      {
         // do nothing
      }
      DOMElement *rootelement(NULL);
      if (aoiDoc != NULL)
      {
         rootelement = aoiDoc->getDocumentElement();
      }
      else
      {
         success = false;
      }

      if (rootelement != NULL)
      {
         unsigned int formatVersion = atoi(A(rootelement->getAttribute(X("version"))));
         try
         {
            ok = pAoiCopy->fromXml(rootelement, formatVersion);
         }
         catch (...)
         {
            ok = false;
         }
         issea(ok == true);

         const BitMask *pAoiBitmask = NULL;
         pAoiBitmask = pAoi->getSelectedPoints();
         issea(pAoiBitmask != NULL);
         issea(pAoiCopy->getSelectedPoints()->compare(*pAoiBitmask));
      }
      else
      {
         success = false;
      }

      if (pProperties != NULL)
      {
         pView->deleteLayer(pProperties);
         pProperties = NULL;
      }
      if (pAoiCopy != NULL)
      {
         ModelServicesImp::instance()->destroyElement(pAoiCopy);
      }

      return success;
   }
};

class AoiSerializeDeserializeTest2 : public TestCase
{
public:
   AoiSerializeDeserializeTest2() : TestCase("MultipleRectangles") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(10, 28), LocationType(51, 54));

      pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(71, 54), LocationType(90, 86));

      pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(15, 127), LocationType(87, 142));

      bool ok = false;
      // serialize the AOI
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen((tempHome + "/secondAOI.aoi").c_str(), "w");
      issea(pOutputFile != NULL);

      XMLWriter xwrite("AoiElement");
      ok = pAoi->toXml(&xwrite);
      issea(ok == true);
      xwrite.writeToFile(pOutputFile);
      fclose(pOutputFile);
      issea(ok == true);

      ModelServicesImp::instance()->setElementName(pAoi, "AoiSerialized");
      // create a new AOI to deserialize to	  
      
      AoiElement* pAoiCopy = NULL;
      pAoiCopy = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoiCopy != NULL);

      // deserialize the layer
      MessageLog *pLog(UtilityServicesImp::instance()->getMessageLog()->getLog("session"));
      XmlReader xml(pLog);

      FilenameImp aoiFile(tempHome + "/secondAOI.aoi");

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *aoiDoc(NULL);
      try
      {
         aoiDoc = xml.parse(&aoiFile);
      }
      catch(XmlBase::XmlException &)
      {
         // do nothing
      }
      DOMElement *rootelement(NULL);
      if (aoiDoc != NULL)
      {
         rootelement = aoiDoc->getDocumentElement();
      }
      else
      {
         success = false;
      }

      if (rootelement != NULL)
      {
         unsigned int formatVersion = atoi(A(rootelement->getAttribute(X("version"))));
         try
         {
            ok = pAoiCopy->fromXml(rootelement, formatVersion);
         }
         catch (...)
         {
            ok = false;
         }
         issea(ok == true);

         int x1, y1, x2, y2;
         x1 = y1 = x2 = y2 = 0;
         const BitMask *pAoiBitmask = NULL;
         pAoiBitmask = pAoiCopy->getSelectedPoints();
         issea(pAoiBitmask != NULL);
         pAoiBitmask->getBoundingBox(x1, y1, x2, y2);
         issea(x1 == 0 && y1 == 28 && x2 == 95 && y2 == 142);

         AoiLayer* pProperties2 = NULL;
         pProperties2 = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoiCopy, "AoiDeserialized"));
         issea(pProperties2 != NULL);
         pView->refresh();
         if (success)
         {
            QCoreApplication::instance()->processEvents();
         }

         if (pProperties2 != NULL)
         {
            pView->deleteLayer(pProperties2);
            pProperties2 = NULL;
         }      
      }
      else
      {
         ModelServicesImp::instance()->destroyElement(pAoiCopy);
         success = false;
      }

      if (pProperties != NULL)
      {
         pView->deleteLayer(pProperties);
         pProperties = NULL;
      }

      return success;
   }
};

class AoiSerializeDeserializeBitmask : public TestCase
{
public:
   AoiSerializeDeserializeBitmask() : TestCase("Bitmask") {}
   bool run()
   {
      bool success = true;
      Service<ObjectFactory> pFactory;      

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(
         pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      BitMask *pMask = NULL;
      pMask = static_cast<BitMask*>(pFactory->createObject("BitMask"));
      issea(pMask != NULL);

      pMask->clear();
      pMask->setPixel(10, 28, true);
      pMask->setPixel(51, 54, true);
      pMask->setPixel(71, 54, true);
      pMask->setPixel(90, 86, true);
      pMask->setPixel(15, 127, true);
      pMask->setPixel(87, 142, true);

      issea(pMask->getCount() == 6);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement(
         "AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      pAoi->addPoints(pMask);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      bool ok = false;
      // serialize the AOI layer
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen((tempHome + "/bitmaskAOI.aoi").c_str(), "w");
      issea(pOutputFile != NULL);

      XMLWriter xwrite("AoiElement");
      ok = pAoi->toXml(&xwrite);
      issea(ok == true);
      xwrite.writeToFile(pOutputFile);
      fclose(pOutputFile);
      issea(ok == true);

      ModelServicesImp::instance()->setElementName(pAoi, "AoiSerialized");

      // create a new AOI to deserialize to
      AoiElement* pAoiCopy = NULL;
      pAoiCopy = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement(
         "AoiName", "AoiElement", pRasterElement));
      issea(pAoiCopy != NULL);

      // deserialize the layer
      MessageLog *pLog(UtilityServicesImp::instance()->getMessageLog()->getLog("session"));
      XmlReader xml(pLog);

      FilenameImp aoiFile(tempHome + "/bitmaskAOI.aoi");

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *aoiDoc(NULL);
      try
      {
         aoiDoc = xml.parse(&aoiFile);
      }
      catch(XmlBase::XmlException &)
      {
         // do nothing
      }
      DOMElement *rootelement(NULL);
      if (aoiDoc != NULL)
      {
         rootelement = aoiDoc->getDocumentElement();
      }
      else
      {
         success = false;
      }

      if (rootelement != NULL)
      {
         const BitMask *pAoiBitmask = NULL;
         pAoiBitmask = pAoiCopy->getSelectedPoints();
         issea(pAoiBitmask != NULL);
         issea(pAoiBitmask->getCount() == 0);
         
         unsigned int formatVersion = atoi(A(rootelement->getAttribute(X("version"))));
         try
         {
            ok = pAoiCopy->fromXml(rootelement, formatVersion);
         }
         catch (...)
         {
            ok = false;
         }
         issea(ok == true);

         pAoiBitmask = NULL;
         pAoiBitmask = pAoiCopy->getSelectedPoints();
         issea(pAoiBitmask != NULL);
         issea(pAoiBitmask->getCount() == 6);
         issea(pMask->compare(*pAoiBitmask));

         AoiLayer* pProperties2 = NULL;
         pProperties2 = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoiCopy, "AoiDeserialized"));
         issea(pProperties2 != NULL);
         pView->refresh();
         if (success)
         {
            QCoreApplication::instance()->processEvents();
         }

         if (pProperties2 != NULL)
         {
            pView->deleteLayer(pProperties2);
            pProperties2 = NULL;
         }
      }
      else
      {
         if (pAoiCopy != NULL)
         {
            ModelServicesImp::instance()->destroyElement(pAoiCopy);
         }
      }

      if (pProperties != NULL)
      {
         pView->deleteLayer(pProperties);
         pProperties = NULL;
      }

      return success;
   }
};

class AoiSerializeDeserializeLayerTest : public TestCase
{
public:
   AoiSerializeDeserializeLayerTest() : TestCase("SerializeLayer") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 120), LocationType(40, 140));
      QCoreApplication::instance()->processEvents();

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      bool ok = false;
      // serialize the AOI

      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen((tempHome + "/AOILayer.aoilayer").c_str(), "w");
      issea(pOutputFile != NULL);

      XMLWriter xwrite("AoiLayer");
      ok = pAoi->toXml(&xwrite);
      issea(ok == true);
      xwrite.writeToFile(pOutputFile);
      fclose(pOutputFile);
      issea(ok == true);

      ModelServicesImp::instance()->setElementName(pAoi, "AoiSerialized");

      // create a new AOI Layer to deserialize to
      AoiElement* pAoiCopy = NULL;
      pAoiCopy = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoiCopy != NULL);

      AoiLayer* pPropertiesCopy = NULL;
      pPropertiesCopy = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoiCopy, "AoiLayerCopy"));
      issea(pPropertiesCopy != NULL);

      // deserialize the layer
      MessageLog *pLog(UtilityServicesImp::instance()->getMessageLog()->getLog("session"));
      XmlReader xml(pLog);

      FilenameImp aoiFile(tempHome + "/AOILayer.aoilayer");

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *aoiDoc(NULL);
      try
      {
         aoiDoc = xml.parse(&aoiFile);
      }
      catch(XmlBase::XmlException &)
      {
         // do nothing
      }
      DOMElement *rootelement(NULL);
      if (aoiDoc != NULL)
      {
         rootelement = aoiDoc->getDocumentElement();
      }
      else
      {
         success = false;
      }

      if (rootelement != NULL)
      {
         unsigned int formatVersion = atoi(A(rootelement->getAttribute(X("version"))));
         try
         {
            ok = pAoiCopy->fromXml(rootelement, formatVersion);
         }
         catch (...)
         {
            ok = false;
         }
         issea(ok == true);

         const BitMask *pAoiBitmask = NULL;
         pAoiBitmask = pAoi->getSelectedPoints();
         issea(pAoiBitmask != NULL);
         issea(static_cast<AoiElement*>(pPropertiesCopy->getDataElement())->getName() == "AoiNameCopy");
         issea(static_cast<AoiElement*>(pPropertiesCopy->getDataElement())->getSelectedPoints()->compare(*pAoiBitmask));
      }

      if (pProperties != NULL)
      {
         pView->deleteLayer(pProperties);
         pProperties = NULL;
      }
      if (pPropertiesCopy != NULL)
      {
         pView->deleteLayer(pPropertiesCopy);
         pPropertiesCopy = NULL;
      }

      return success;
   }
};

class AoiTogglePixelsTest : public TestCase
{
public:
   AoiTogglePixelsTest() : TestCase("Toggle") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("BigAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "BigAoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 70), LocationType(69, 109));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      unsigned int numNonAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 50 * 40); // num pixels = (70-20)*(110-70)
      numNonAoiPixels = numPixels - numAoiPixels;

      // now toggle the aoi pixels
      pAoi->toggleAllPoints();
      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      issea(pAoiBitMask->getCount() == 0);  // zero because they were all toggled off

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiBitmaskInvertTest : public TestCase
{
public:
   AoiBitmaskInvertTest() : TestCase("BitmaskInvert") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("SmallAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "SmallAoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 20), LocationType(29, 29));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      unsigned int numNonAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 10 * 10); // num pixels = (30-20)*(30-20)
      numNonAoiPixels = numPixels - numAoiPixels;

      // now do an invert on the BitMask
      pAoiBitMask->invert();
      numAoiPixels = pAoiBitMask->getCount(); // the pixel count should now be the inverse of what is was before
      issea(numAoiPixels == 0);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiClearPointsTest : public TestCase
{
public:
   AoiClearPointsTest() : TestCase("ClearPoints") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("SmallAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "SmallAoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 20), LocationType(29, 29));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 10 * 10); // num pixels = (30-20)*(30-20)
      pAoi->clearPoints();
      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 0);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiClearBitMaskTest : public TestCase
{
public:
   AoiClearBitMaskTest() : TestCase("ClearBitMask") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("SmallAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "SmallAoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 20), LocationType(29, 29));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 10 * 10); // num pixels = (30-20)*(30-20)
      pAoiBitMask->clear();
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 0);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiRemovePointsTest : public TestCase
{
public:
   AoiRemovePointsTest() : TestCase("RemovePoints") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("SmallAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "SmallAoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(20, 20), LocationType(29, 29));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 10 * 10); // num pixels = (30-20)*(30-20)

      // remove the four corners of the AOI rectangle
      pAoi->removePoint(LocationType(20, 20));
      pAoi->removePoint(LocationType(20, 29));
      pAoi->removePoint(LocationType(29, 20));
      pAoi->removePoint(LocationType(29, 29));
      QCoreApplication::instance()->processEvents();

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 10 * 10 - 4);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiAddPointsTest : public TestCase
{
public: 
   AoiAddPointsTest() : TestCase("AddPoints") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement= TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(40, 40), LocationType(59, 59));
      QCoreApplication::instance()->processEvents();

      unsigned int numAoiPixels = 0;
      BitMask *pAoiBitMask = NULL;

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 20 * 20); // num pixels = (60-40)*(60-40)

      // add eight new pixels to the AOI rectangle
      pAoi->addPoint(LocationType(39, 39));
      pAoi->addPoint(LocationType(39, 60));
      pAoi->addPoint(LocationType(60, 39));
      pAoi->addPoint(LocationType(60, 60));
      pAoi->addPoint(LocationType(38, 38));
      pAoi->addPoint(LocationType(38, 61));
      pAoi->addPoint(LocationType(61, 38));
      pAoi->addPoint(LocationType(61, 61));

      pAoiBitMask = const_cast<BitMask*>(pAoi->getSelectedPoints());
      numAoiPixels = pAoiBitMask->getCount();
      issea(numAoiPixels == 20 * 20 + 8);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiSymbolAndColorChangeTest : public TestCase
{
public:
   AoiSymbolAndColorChangeTest() : TestCase("SymbolAndColorChange") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("SmallAoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "SmallAoiName"));
      issea(pProperties != NULL);

      PolygonObject *pPoly = static_cast<PolygonObject*>(pAoi->getGroup()->addObject(POLYGON_OBJECT));
      issea(pPoly != NULL);
      vector<LocationType> locationTypeVector;
      locationTypeVector.push_back(LocationType(8, 20));
      locationTypeVector.push_back(LocationType(12, 24));
      locationTypeVector.push_back(LocationType(32, 80));
      locationTypeVector.push_back(LocationType(10, 70));
      pPoly->addVertices(locationTypeVector);
      QCoreApplication::instance()->processEvents();

      ColorType aoiColor;
      pProperties->setColor(ColorType(255, 0, 0));
      aoiColor = pProperties->getColor();
      issea(aoiColor == ColorType(255, 0, 0));
      pProperties->setColor(ColorType(0, 255, 0));
      aoiColor = pProperties->getColor();
      issea(aoiColor == ColorType(0, 255, 0));
      pProperties->setColor(ColorType(0, 0, 255));
      aoiColor = pProperties->getColor();
      issea(aoiColor == ColorType(0, 0, 255));

      SymbolType aoiSymbol;
      pProperties->setSymbol(X);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == X);
      pProperties->setSymbol(CROSS_HAIR);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == CROSS_HAIR);
      pProperties->setSymbol(ASTERISK);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == ASTERISK);
      pProperties->setSymbol(HORIZONTAL_LINE);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == HORIZONTAL_LINE);
      pProperties->setSymbol(VERTICAL_LINE);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == VERTICAL_LINE);
      pProperties->setSymbol(FORWARD_SLASH);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == FORWARD_SLASH);
      pProperties->setSymbol(BACK_SLASH);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BACK_SLASH);
      pProperties->setSymbol(BOX);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOX);
      pProperties->setSymbol(BOXED_X);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_X);
      pProperties->setSymbol(BOXED_CROSS_HAIR);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_CROSS_HAIR);
      pProperties->setSymbol(BOXED_ASTERISK);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_ASTERISK);
      pProperties->setSymbol(BOXED_HORIZONTAL_LINE);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_HORIZONTAL_LINE);
      pProperties->setSymbol(BOXED_VERTICAL_LINE);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_VERTICAL_LINE);
      pProperties->setSymbol(BOXED_FORWARD_SLASH);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_FORWARD_SLASH);
      pProperties->setSymbol(BOXED_BACK_SLASH);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == BOXED_BACK_SLASH);
      pProperties->setSymbol(SOLID);
      aoiSymbol = pProperties->getSymbol();
      issea(aoiSymbol == SOLID);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiSetPixelNameTest : public TestCase
{
public:
   AoiSetPixelNameTest() : TestCase("SetPixelName") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDataDescriptor != NULL);

      unsigned int numCols = 0;
      unsigned int numRows = 0;
      unsigned int numPixels = 0;
      numCols = pDataDescriptor->getColumnCount();
      issea(numCols == 97);
      numRows = pDataDescriptor->getRowCount();
      issea(numRows == 181);
      numPixels = numCols * numRows;
      issea(numPixels == 17557);
      issea(pDataDescriptor->getColumnCount() * pDataDescriptor->getRowCount() == numPixels);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AoiName", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "AoiName"));
      issea(pProperties != NULL);

      vector<LocationType> locationTypeVector;
      MultipointObject *pObj = NULL;

      pObj = static_cast<MultipointObject*>(pAoi->getGroup()->addObject(MULTIPOINT_OBJECT));
      locationTypeVector.clear();
      locationTypeVector.push_back(LocationType(30, 30));
      pObj->addVertices(locationTypeVector);
      pObj->setName("Pixel1");

      pObj = static_cast<MultipointObject*>(pAoi->getGroup()->addObject(MULTIPOINT_OBJECT));
      locationTypeVector.clear();
      locationTypeVector.push_back(LocationType(35, 35));
      pObj->addVertices(locationTypeVector);
      pObj->setName("Pixel2");

      pObj = static_cast<MultipointObject*>(pAoi->getGroup()->addObject(MULTIPOINT_OBJECT));
      locationTypeVector.clear();
      locationTypeVector.push_back(LocationType(25, 25));
      pObj->addVertices(locationTypeVector);
      pObj->setName("Pixel3");

      pObj = static_cast<MultipointObject*>(pAoi->getGroup()->addObject(MULTIPOINT_OBJECT));
      locationTypeVector.clear();
      locationTypeVector.push_back(LocationType(35, 25));
      pObj->addVertices(locationTypeVector);
      pObj->setName("Pixel4");

      pObj = static_cast<MultipointObject*>(pAoi->getGroup()->addObject(MULTIPOINT_OBJECT));
      locationTypeVector.clear();
      locationTypeVector.push_back(LocationType(25, 35));
      pObj->addVertices(locationTypeVector);
      pObj->setName("Pixel5");

      QCoreApplication::instance()->processEvents();

      bool nameWasSet = false;
      string thePixelName = "";
      const list<GraphicObject*> &objects = pAoi->getGroup()->getObjects();
      bool gotPix1 = false, gotPix2 = false, gotPix3 = false, gotPix4 = false, gotPix5 = false;
      for (list<GraphicObject*>::const_iterator iter = objects.begin();
         iter != objects.end(); ++iter)
      {
         thePixelName = (*iter)->getName();
         if ((*iter)->getLlCorner() == LocationType(30, 30))
         {
            issea(!gotPix1);
            issea(thePixelName == "Pixel1");
            gotPix1 = true;
         }
         else if ((*iter)->getLlCorner() == LocationType(35, 35))
         {
            issea(!gotPix2);
            issea(thePixelName == "Pixel2");
            gotPix2 = true;
         }
         else if ((*iter)->getLlCorner() == LocationType(25, 25))
         {
            issea(!gotPix3);
            issea(thePixelName == "Pixel3");
            gotPix3 = true;
         }
         else if ((*iter)->getLlCorner() == LocationType(35, 25))
         {
            issea(!gotPix4);
            issea(thePixelName == "Pixel4");
            gotPix4 = true;
         }
         else if ((*iter)->getLlCorner() == LocationType(25, 35))
         {
            issea(!gotPix5);
            issea(thePixelName == "Pixel5");
            gotPix5 = true;
         }
      }

      issea(gotPix1 && gotPix2 && gotPix3 && gotPix4 && gotPix5);

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};


class AoiRenameTest : public TestCase
{
public:
   AoiRenameTest() : TestCase("Rename") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      LayerList *pList = pView->getLayerList();
      issea(pList != NULL);
      int startLayers = pList->getNumLayers(AOI_LAYER);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("AOI 1", "AoiElement", pRasterElement));
      if (pAoi == NULL)
      {
         pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->getElement("AOI 1", "AoiElement", pRasterElement));
      }
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "RenameTest"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      pRect->setBoundingBox(LocationType(50, 50), LocationType(70, 110));
      QCoreApplication::instance()->processEvents();

      // get the list of AOIs from ModelServices
      unsigned int numAois = 0;
      vector<string> aoiVector;
      string name = "";
      aoiVector = ModelServicesImp::instance()->getElementNames(pRasterElement, "AoiElement");
      numAois = aoiVector.size();
      issea(numAois == 1);
      issea(aoiVector.size() == 1);
      issea(aoiVector.at(0) == "AOI 1");

      // rename the AOI
      dynamic_cast<AoiLayerImp*>(pProperties)->setName("RenameTest2");
      ModelServicesImp::instance()->setElementName(pAoi, "RenameTest2");
      numAois = 0;
      aoiVector.clear();
      pList = NULL;
      pList = pView->getLayerList();
      issea(pList != NULL);
      issea(pList->getNumLayers(AOI_LAYER) == startLayers + 1);

      name = pList->getLayer(AOI_LAYER, pAoi)->getName();
      issea(name == "RenameTest2");

      aoiVector = ModelServicesImp::instance()->getElementNames(pRasterElement, "AoiElement");
      numAois = aoiVector.size();
      issea(numAois == 1);
      issea(aoiVector.size() == 1);
      issea(aoiVector.at(0) == "RenameTest2");

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      return success;
   }
};

class AoiRenameTest2 : public TestCase
{
public:
   AoiRenameTest2() : TestCase("Rename2") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea(pRasterElement != NULL);

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("newAOI", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "newAOI"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(50, 50), LocationType(70, 110));

      AoiElement* pAoi2 = NULL;
      pAoi2 = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("newAOI2", "AoiElement", pRasterElement));
      issea(pAoi2 != NULL);

      AoiLayer* pProperties2 = NULL;
      pProperties2 = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi2, "newAOI2"));
      issea(pProperties2 != NULL);

      RectangleObject *pRect2 = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect2 != NULL);
      pRect->setBoundingBox(LocationType(10, 10), LocationType(20, 140));
      QCoreApplication::instance()->processEvents();

      // get the list of AOIs from ModelServices and the list of Layers
      unsigned int numAois = 0;
      unsigned int numLayers = 0;
      vector<string> aoiVector;
      vector<Layer*> layerVector;
      string name = "";
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea(pList != NULL);
      numLayers = pList->getNumLayers();
      issea(numLayers == 3); // raster layer, newAOI, newAOI2

      aoiVector = ModelServicesImp::instance()->getElementNames(pRasterElement, "AoiElement");
      numAois = aoiVector.size();
      issea(numAois == 2); // newAOI, newAOI2
      issea(aoiVector.size() == 2);
      issea(aoiVector.at(0) == "newAOI");
      issea(aoiVector.at(1) == "newAOI2");

      pList->getLayers(AOI_LAYER, layerVector);
      issea(layerVector.size() == 2);
      name = layerVector.at(0)->getName();
      issea(name == "newAOI");
      name = layerVector.at(1)->getName();
      issea(name == "newAOI2");
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea(name == pRasterElement->getName());

      // rename the newAOI
      dynamic_cast<AoiLayerImp*>(pProperties)->setName("renameAOI");
      ModelServicesImp::instance()->setElementName(pAoi, "renameAOI");
      numAois = 0;
      aoiVector.clear();
      pList = NULL;
      pList = pView->getLayerList();
      issea(pList != NULL);
      issea(pList->getNumLayers() == 3); // raster layer, renameAOI, newAOI2

      aoiVector = ModelServicesImp::instance()->getElementNames(pRasterElement, "AoiElement");
      numAois = aoiVector.size();
      issea(numAois == 2); // renameAOI, newAOI2
      issea(aoiVector.size() == 2);
      issea(aoiVector.at(0) == "newAOI2");
      issea(aoiVector.at(1) == "renameAOI");

      pList->getLayers(AOI_LAYER, layerVector);
      issea(layerVector.size() == 2);
      name = layerVector.at(0)->getName();
      issea(name == "renameAOI");
      name = layerVector.at(1)->getName();
      issea(name == "newAOI2");
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea(name == pRasterElement->getName());

      pView->deleteLayer(pProperties);
      pView->deleteLayer(pProperties2);
      pProperties = NULL;
      pProperties2 = NULL;

      return success;
   }
};

class AoiPolygonBoundingBoxTest : public TestCase
{
public:
   AoiPolygonBoundingBoxTest() : TestCase("Polygon") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>(DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issea(pView != NULL);

      AoiElement* pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("newPolygonAOI", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      AoiLayer* pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "newAOI"));
      issea(pProperties != NULL);

      PolygonObject *pPoly = static_cast<PolygonObject*>(pAoi->getGroup()->addObject(POLYGON_OBJECT));
      issea(pPoly != NULL);
      vector<LocationType> locationTypeVector;
      locationTypeVector.push_back(LocationType(45, 70));
      locationTypeVector.push_back(LocationType(65, 75));
      locationTypeVector.push_back(LocationType(60, 105));
      locationTypeVector.push_back(LocationType(40, 108));
      locationTypeVector.push_back(LocationType(50, 86));
      pPoly->addVertices(locationTypeVector);

      const BitMask *pMask = NULL;
      pMask = pAoi->getSelectedPoints();
      issea(pMask != NULL);

      int x1, y1, x2, y2;
      x1 = y1 = x2 = y2 = 0;
      pMask->getBoundingBox(x1, y1, x2, y2);

      issea(x1 == 40 && y1 == 70 && x2 == 65 && y2 == 108);      

      pView->deleteLayer(pProperties);
      pProperties = NULL;

      // now create a rectangular AOI and verify its bounding box is still good
      pAoi = NULL;
      pAoi = static_cast<AoiElement*>(ModelServicesImp::instance()->createElement("newRectangleAOI", "AoiElement", pRasterElement));
      issea(pAoi != NULL);

      pProperties = NULL;
      pProperties = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi, "newRectangleAOI"));
      issea(pProperties != NULL);

      RectangleObject *pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issea(pRect != NULL);
      pRect->setBoundingBox(LocationType(10, 78), LocationType(28, 128));

      const BitMask *pMaskRectangle = NULL;
      pMaskRectangle = pAoi->getSelectedPoints();
      issea(pMaskRectangle != NULL);

      x1 = y1 = x2 = y2 = 0;
      pMaskRectangle->getBoundingBox(x1, y1, x2, y2);

      issea(x1 == 10 && y1 == 78 && x2 == 28 && y2 == 128);

      if (pView != NULL)
      {
         pView->deleteLayer(pProperties);
      }

      return success;
   }
};

class AoiNotifyPointsChangedTest : public TestCase
{
public: 
   AoiNotifyPointsChangedTest() : TestCase("NotifyPointsChanged") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement= TestUtilities::getStandardRasterElement();
      issearf(pRasterElement != NULL);

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>(
         DesktopServicesImp::instance()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issearf(pWindow != NULL);

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issearf(pView != NULL);

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issearf(pDataDescriptor != NULL);

      ModelResource<AoiElement> pAoi("AoiName", pRasterElement);
      issearf(pAoi.get() != NULL);

      AoiLayer* pAoiLayer = NULL;
      pAoiLayer = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER, pAoi.get(), "AoiName"));
      issearf(pAoiLayer != NULL);

      auto_ptr<AoiObserver> pObserver(new AoiObserver());
      issea(pObserver.get() != NULL);
      issea(pAoi->attach(SIGNAL_NAME(AoiElement, PointsChanged), Slot(pObserver.get(), &AoiObserver::aoiChanged)));
      issea(pAoi->attach(SIGNAL_NAME(Subject, Modified), Slot(pObserver.get(), &AoiObserver::aoiChanged)));

      // test notification on adding graphic objects
      pObserver->clearNotifications();
      RectangleObject* pRect = static_cast<RectangleObject*>(pAoi->getGroup()->addObject(RECTANGLE_OBJECT));
      issearf(pRect != NULL);
      pRect->setBoundingBox(LocationType(40, 40), LocationType(59, 59));
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      DimensionObject* pRow = static_cast<DimensionObject*>(pAoi->getGroup()->addObject(ROW_OBJECT));
      issearf(pRow != NULL);
      pRow->setBoundingBox(LocationType(15, 100), LocationType(15, 100));
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test that PointsChanged is not sent when properties other than bounding box are changed
      pObserver->clearNotifications();
      pRect->setFillColor(ColorType(255, 100, 100));
      issea(pObserver->getNumberOfSubjectModifiedNotifications() > 0);
      issea(pObserver->getNumberOfPointsChangedNotifications() == 0);
      pObserver->clearNotifications();
      pRow->setPixelSymbol(BOX);
      issea(pObserver->getNumberOfSubjectModifiedNotifications() > 0);
      issea(pObserver->getNumberOfPointsChangedNotifications() == 0);
      pObserver->clearNotifications();
      pRect->setLineWidth(4.0);
      issea(pObserver->getNumberOfSubjectModifiedNotifications() > 0);
      issea(pObserver->getNumberOfPointsChangedNotifications() == 0);

      // test notification on removal of graphic object
      pObserver->clearNotifications();
      pAoi->getGroup()->removeObject(pRow, true);
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on graphic object resized
      pObserver->clearNotifications();
      pRect->setBoundingBox(LocationType(20, 20), LocationType(59, 59));
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on graphic object moved
      pObserver->clearNotifications();
      AoiLayerImp* pAoiLayerImp = dynamic_cast<AoiLayerImp*>(pAoiLayer);
      if (pAoiLayerImp != NULL)
      {
         pAoiLayerImp->selectObject(pRect);
         pAoiLayerImp->moveSelectedObjects(LocationType(10, 10));
      }
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on toggling points
      pObserver->clearNotifications();
      pAoi->togglePoint(LocationType(50, 50));        // single point
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      vector<LocationType> points;
      points.push_back(LocationType(10, 10));
      points.push_back(LocationType(20, 20));
      points.push_back(LocationType(30, 30));
      pAoi->togglePoints(points);             // multiple points in vector
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      FactoryResource<BitMask> pBitMask;
      pBitMask->setRegion(70, 70, 80, 80, DRAW);
      pAoi->togglePoints(pBitMask.get());             // multiple points in BitMask
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      pAoi->toggleAllPoints();                        // all points
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on clearing points
      pObserver->clearNotifications();
      pAoi->clearPoints();
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on adding points
      pObserver->clearNotifications();
      pAoi->addPoint(LocationType(50, 50));           // single point
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      pAoi->addPoints(points);                        // multiple points in vector - note, reusing previous vector
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      pAoi->addPoints(pBitMask.get());                // multiple points in BitMask - note, reusing previous BitMask
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      // test notification on removing points
      pObserver->clearNotifications();
      pAoi->removePoint(LocationType(50, 50));        // single point
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      pAoi->removePoints(points);                     // multiple points in vector - note, reusing previous vector
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);
      pObserver->clearNotifications();
      pAoi->removePoints(pBitMask.get());             // multiple points in BitMask - note, reusing previous BitMask
      issea(pObserver->getNumberOfPointsChangedNotifications() == 1);

      return success;
   }
};

class AoiTestSuite : public TestSuiteNewSession
{
public:
   AoiTestSuite() : TestSuiteNewSession("Aoi")
   {
      addTestCase(new AoiDerivationTestCase);
      addTestCase(new AoiSerializeDeserializeTest);
      addTestCase(new AoiSerializeDeserializeTest2);
      addTestCase(new AoiSerializeDeserializeBitmask);
      addTestCase(new AoiSerializeDeserializeLayerTest);
      addTestCase(new AoiTogglePixelsTest);
      addTestCase(new AoiBitmaskInvertTest);
      addTestCase(new AoiClearPointsTest);
      addTestCase(new AoiClearBitMaskTest);
      addTestCase(new AoiRemovePointsTest);
      addTestCase(new AoiAddPointsTest);
      addTestCase(new AoiSymbolAndColorChangeTest);
      addTestCase(new AoiSetPixelNameTest);
      addTestCase(new AoiRenameTest);
      addTestCase(new AoiRenameTest2);
      addTestCase(new AoiPolygonBoundingBoxTest);
      addTestCase(new AoiNotifyPointsChangedTest);
   }
};

REGISTER_SUITE(AoiTestSuite)
