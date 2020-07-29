/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayerImp.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "assert.h"
#include "BitMaskImp.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "Font.h"
#include "GcpList.h"
#include "GcpListUndo.h"
#include "GcpLayer.h"
#include "ImageFilterDescriptor.h"
#include "LatLonLayer.h"
#include "LayerList.h"
#include "ObjectResource.h"
#include "ProductWindow.h"
#include "ProductView.h"
#include "ProductViewImp.h"
#include "PseudocolorClass.h"
#include "PseudocolorLayerImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "SpatialDataWindow.h"
#include "SpatialDataViewImp.h"
#include "TestBedTestUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "ThresholdLayer.h"
#include "TiePointLayer.h"
#include "TiePointList.h"
#include "TiePointListUndo.h"
#include "Undo.h"
#include "UndoAction.h"
#include "UndoStack.h"

#include <list>
using namespace std;

class AnnotationUndoTestCase : public TestCase
{
public:
   AnnotationUndoTestCase() : TestCase("Annotation") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      AnnotationLayerImp* pLayer = dynamic_cast<AnnotationLayerImp*>(pView->createLayer(ANNOTATION));
      issea(pLayer != NULL);
      issea(pLayerList->getNumLayers(ANNOTATION) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(ANNOTATION) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(ANNOTATION) == 1);

      pLayer = dynamic_cast<AnnotationLayerImp*>(pView->getTopMostLayer(ANNOTATION));
      issea(pLayer != NULL);

      // Add graphic objects
      {
         UndoGroup group(dynamic_cast<View*>(pView), "Add Rectangle");
         GraphicObject* pRectangleObject = pLayer->addObject(RECTANGLE_OBJECT, LocationType());
         issea(pRectangleObject != NULL);
         pRectangleObject->setBoundingBox(LocationType(0, 0), LocationType(11, 17));
         pRectangleObject->setFillColor(ColorType(11, 17, 23));
      }
      issea(pLayer->getNumObjects() == 1);

      {
         UndoGroup group(dynamic_cast<View*>(pView), "Add Ellipse");
         GraphicObject* pEllipseObject = pLayer->addObject(ELLIPSE_OBJECT, LocationType());
         issea(pEllipseObject != NULL);
         pEllipseObject->setBoundingBox(LocationType(0, 0), LocationType(23, 29));
         pEllipseObject->setLineColor(ColorType(29, 31, 37));
      }
      issea(pLayer->getNumObjects() == 2);

      {
         UndoGroup group(dynamic_cast<View*>(pView), "Add Line");
         GraphicObject* pLineObject = pLayer->addObject(LINE_OBJECT, LocationType());
         issea(pLineObject != NULL);
         pLineObject->setBoundingBox(LocationType(0, 0), LocationType(31, 39));
         pLineObject->setLineWidth(3);
      }
      issea(pLayer->getNumObjects() == 3);

      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 2);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 0);

      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 2);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 3);

      GraphicObject* pRectangleObject = NULL;
      GraphicObject* pEllipseObject = NULL;
      GraphicObject* pLineObject = NULL;

      list<GraphicObject*> objects = pLayer->getObjects();
      for (list<GraphicObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter)
      {
         GraphicObject* pObject = *iter;
         if (pObject != NULL)
         {
            if (pObject->getGraphicObjectType() == RECTANGLE_OBJECT)
            {
               pRectangleObject = pObject;
            }
            else if (pObject->getGraphicObjectType() == ELLIPSE_OBJECT)
            {
               pEllipseObject = pObject;
            }
            else if (pObject->getGraphicObjectType() == LINE_OBJECT)
            {
               pLineObject = pObject;
            }
         }
      }

      issea(pRectangleObject != NULL);
      issea(pEllipseObject != NULL);
      issea(pLineObject != NULL);

      // Stacking order
      issea(pLayer->getObjectStackingIndex(pRectangleObject) == 0);
      issea(pLayer->getObjectStackingIndex(pEllipseObject) == 1);
      issea(pLayer->getObjectStackingIndex(pLineObject) == 2);
      pLayer->selectObject(pEllipseObject);
      pLayer->popSelectedObjectToFront();
      issea(pLayer->getObjectStackingIndex(pRectangleObject) == 0);
      issea(pLayer->getObjectStackingIndex(pEllipseObject) == 2);
      issea(pLayer->getObjectStackingIndex(pLineObject) == 1);
      pUndoStack->undo();
      issea(pLayer->getObjectStackingIndex(pRectangleObject) == 0);
      issea(pLayer->getObjectStackingIndex(pEllipseObject) == 1);
      issea(pLayer->getObjectStackingIndex(pLineObject) == 2);
      pUndoStack->redo();
      issea(pLayer->getObjectStackingIndex(pRectangleObject) == 0);
      issea(pLayer->getObjectStackingIndex(pEllipseObject) == 2);
      issea(pLayer->getObjectStackingIndex(pLineObject) == 1);

      // Properties
      ColorType oldFillColor = pRectangleObject->getFillColor();
      ColorType newFillColor(255 - oldFillColor.mRed, 255 - oldFillColor.mGreen, 255 - oldFillColor.mBlue);

      pRectangleObject->setFillColor(newFillColor);
      issea(pRectangleObject->getFillColor() == newFillColor);
      pUndoStack->undo();
      issea(pRectangleObject->getFillColor() == oldFillColor);
      pUndoStack->redo();
      issea(pRectangleObject->getFillColor() == newFillColor);

      LineStyle oldLineStyle = pEllipseObject->getLineStyle();
      LineStyle newLineStyle = static_cast<LineStyleEnum>((oldLineStyle + 1) % 2);

      pEllipseObject->setLineStyle(newLineStyle);
      issea(pEllipseObject->getLineStyle() == newLineStyle);
      pUndoStack->undo();
      issea(pEllipseObject->getLineStyle() == oldLineStyle);
      pUndoStack->redo();
      issea(pEllipseObject->getLineStyle() == newLineStyle);

      double oldLineWidth = pLineObject->getLineWidth();
      double newLineWidth = oldLineWidth + 5.0;

      pLineObject->setLineWidth(newLineWidth);
      issea(pLineObject->getLineWidth() == newLineWidth);
      pUndoStack->undo();
      issea(pLineObject->getLineWidth() == oldLineWidth);
      pUndoStack->redo();
      issea(pLineObject->getLineWidth() == newLineWidth);

      // Group
      issea(pLayer->getNumObjects() == 3);
      pLayer->selectAllObjects();
      pLayer->groupSelection();
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 3);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 1);

      // Ungroup
      pLayer->ungroupSelection();
      issea(pLayer->getNumObjects() == 3);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 3);

      // Delete graphic objects
      pLayer->removeObject(pRectangleObject, true);
      issea(pLayer->getNumObjects() == 2);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 3);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 2);

      pLayer->removeObject(pEllipseObject, true);
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 2);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 1);

      pLayer->removeObject(pLineObject, true);
      issea(pLayer->getNumObjects() == 0);
      pUndoStack->undo();
      issea(pLayer->getNumObjects() == 1);
      pUndoStack->redo();
      issea(pLayer->getNumObjects() == 0);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(ANNOTATION, pLayer->getDataElement(), "AnotherAnnotationLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(ANNOTATION) == 2);

      pView->deleteLayer(dynamic_cast<Layer*>(pLayer));
      issea(pLayerList->getNumLayers(ANNOTATION) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(ANNOTATION) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(ANNOTATION) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(ANNOTATION) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class AoiUndoTestCase : public TestCase
{
public:
   AoiUndoTestCase() : TestCase("AOI") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      AoiLayer* pLayer = dynamic_cast<AoiLayer*>(pView->createLayer(AOI_LAYER));
      issea(pLayer != NULL);
      issea(pLayerList->getNumLayers(AOI_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(AOI_LAYER) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(AOI_LAYER) == 1);

      pLayer = dynamic_cast<AoiLayer*>(pView->getTopMostLayer(AOI_LAYER));
      issea(pLayer != NULL);

      // AOI points
      AoiElement* pAoi = static_cast<AoiElement*>(pLayer->getDataElement());
      issea(pAoi != NULL);

      // End the undo group after adding the AOI point
      {
         UndoGroup group(dynamic_cast<View*>(pView), "Add AOI Point");
         pAoi->addPoint(LocationType(5, 10));
      }

      const BitMaskImp* pOldMask = dynamic_cast<const BitMaskImp*>(pAoi->getSelectedPoints());
      issea(pOldMask != NULL);
      BitMaskImp oldMask(*pOldMask);

      // End the undo group after adding the AOI point
      {
         UndoGroup group(dynamic_cast<View*>(pView), "Add AOI Point");
         pAoi->addPoint(LocationType(60, 110));
      }

      const BitMaskImp* pNewMask = dynamic_cast<const BitMaskImp*>(pAoi->getSelectedPoints());
      issea(pNewMask != NULL);
      BitMaskImp newMask(*pNewMask);

      issea(newMask.compare(*pAoi->getSelectedPoints()) == true);
      pUndoStack->undo();
      issea(oldMask.compare(*pAoi->getSelectedPoints()) == true);
      pUndoStack->redo();
      issea(newMask.compare(*pAoi->getSelectedPoints()) == true);

      // Color
      ColorType oldColor = pLayer->getColor();
      ColorType newColor(255 - oldColor.mRed, 255 - oldColor.mGreen, 255 - oldColor.mBlue);

      pLayer->setColor(newColor);
      issea(pLayer->getColor() == newColor);
      pUndoStack->undo();
      issea(pLayer->getColor() == oldColor);
      pUndoStack->redo();
      issea(pLayer->getColor() == newColor);

      // Symbol
      SymbolType oldSymbol = pLayer->getSymbol();
      SymbolType newSymbol = static_cast<SymbolTypeEnum>((oldSymbol + 1) % 8);

      pLayer->setSymbol(newSymbol);
      issea(pLayer->getSymbol() == newSymbol);
      pUndoStack->undo();
      issea(pLayer->getSymbol() == oldSymbol);
      pUndoStack->redo();
      issea(pLayer->getSymbol() == newSymbol);

      // Clear the AOI
      pLayer->clear();
      issea(pLayer->getNumObjects() == 0);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(AOI_LAYER, pLayer->getDataElement(), "AnotherAoiLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(AOI_LAYER) == 2);

      pView->deleteLayer(pLayer);
      issea(pLayerList->getNumLayers(AOI_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(AOI_LAYER) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(AOI_LAYER) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(AOI_LAYER) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class GcpUndoTestCase : public TestCase
{
public:
   GcpUndoTestCase() : TestCase("GCP") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      GcpLayer* pLayer = dynamic_cast<GcpLayer*>(pView->createLayer(GCP_LAYER));
      issea(pLayer != NULL);
      issea(pLayerList->getNumLayers(GCP_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(GCP_LAYER) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(GCP_LAYER) == 1);

      pLayer = dynamic_cast<GcpLayer*>(pView->getTopMostLayer(GCP_LAYER));
      issea(pLayer != NULL);

      // GCPs
      GcpList* pGcpList = static_cast<GcpList*>(pLayer->getDataElement());
      issea(pGcpList != NULL);

      GcpPoint point1;
      point1.mPixel = LocationType(5, 10);
      point1.mCoordinate = LocationType(1.0, 2.0);
      point1.mRmsError = LocationType(0.1, 0.2);
      pGcpList->addPoint(point1);

      list<GcpPoint> oldPoints = pGcpList->getSelectedPoints();

      GcpPoint point2;
      point2.mPixel = LocationType(6, 11);
      point2.mCoordinate = LocationType(1.5, 2.5);
      point2.mRmsError = LocationType(0.15, 0.25);
      pGcpList->addPoint(point2);

      list<GcpPoint> newPoints = pGcpList->getSelectedPoints();

      pView->addUndoAction(new SetGcpPoints(pGcpList, oldPoints, newPoints));

      issea(pGcpList->getSelectedPoints() == newPoints);
      pUndoStack->undo();
      issea(pGcpList->getSelectedPoints() == oldPoints);
      pUndoStack->redo();
      issea(pGcpList->getSelectedPoints() == newPoints);

      // Color
      ColorType oldColor = pLayer->getColor();
      ColorType newColor(255 - oldColor.mRed, 255 - oldColor.mGreen, 255 - oldColor.mBlue);

      pLayer->setColor(newColor);
      issea(pLayer->getColor() == newColor);
      pUndoStack->undo();
      issea(pLayer->getColor() == oldColor);
      pUndoStack->redo();
      issea(pLayer->getColor() == newColor);

      // Symbol
      GcpSymbol oldSymbol = pLayer->getSymbol();
      GcpSymbol newSymbol = static_cast<GcpSymbolEnum>((oldSymbol + 1) % 2);

      pLayer->setSymbol(newSymbol);
      issea(pLayer->getSymbol() == newSymbol);
      pUndoStack->undo();
      issea(pLayer->getSymbol() == oldSymbol);
      pUndoStack->redo();
      issea(pLayer->getSymbol() == newSymbol);

      // Symbol size
      int oldSize = pLayer->getSymbolSize();
      int newSize = oldSize + 5;

      pLayer->setSymbolSize(newSize);
      issea(pLayer->getSymbolSize() == newSize);
      pUndoStack->undo();
      issea(pLayer->getSymbolSize() == oldSize);
      pUndoStack->redo();
      issea(pLayer->getSymbolSize() == newSize);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(GCP_LAYER, pLayer->getDataElement(), "AnotherGcpLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(GCP_LAYER) == 2);

      pView->deleteLayer(pLayer);
      issea(pLayerList->getNumLayers(GCP_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(GCP_LAYER) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(GCP_LAYER) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(GCP_LAYER) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class LatLonUndoTestCase : public TestCase
{
public:
   LatLonUndoTestCase() : TestCase("LatLon") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      string filename = TestUtilities::getTestDataPath() + "GeoReference/landsat6band.tif";

      SpatialDataWindow *pWindow = TestUtilities::loadDataSet(filename, "GeoTIFF Importer");
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Run georeference
      RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
      issea(pRasterElement != NULL);
      issea(TestUtilities::runGeoRef(pRasterElement));

      // Create layer
      LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(pView->createLayer(LAT_LONG, pRasterElement));
      issea(pLayer != NULL);
      issea(pLayerList->getNumLayers(LAT_LONG) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(LAT_LONG) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(LAT_LONG) == 1);

      pLayer = dynamic_cast<LatLonLayer*>(pView->getTopMostLayer(LAT_LONG));
      issea(pLayer != NULL);

      // Color
      ColorType oldColor = pLayer->getColor();
      ColorType newColor(255 - oldColor.mRed, 255 - oldColor.mGreen, 255 - oldColor.mBlue);

      pLayer->setColor(newColor);
      issea(pLayer->getColor() == newColor);
      pUndoStack->undo();
      issea(pLayer->getColor() == oldColor);
      pUndoStack->redo();
      issea(pLayer->getColor() == newColor);

      // Style
      LatLonStyle oldStyle = pLayer->getStyle();
      LatLonStyle newStyle = static_cast<LatLonStyleEnum>((oldStyle + 1) % 2);

      pLayer->setStyle(newStyle);
      issea(pLayer->getStyle() == newStyle);
      pUndoStack->undo();
      issea(pLayer->getStyle() == oldStyle);
      pUndoStack->redo();
      issea(pLayer->getStyle() == newStyle);

      // Width
      unsigned int oldWidth = pLayer->getWidth();
      unsigned int newWidth = oldWidth + 3;

      pLayer->setWidth(newWidth);
      issea(pLayer->getWidth() == newWidth);
      pUndoStack->undo();
      issea(pLayer->getWidth() == oldWidth);
      pUndoStack->redo();
      issea(pLayer->getWidth() == newWidth);

      // Tick spacing
      LocationType oldTickSpacing = pLayer->getTickSpacing();
      LocationType newTickSpacing = oldTickSpacing + LocationType(0.5, 0.5);

      pLayer->setTickSpacing(newTickSpacing);
      issea(pLayer->getTickSpacing() == newTickSpacing);
      pUndoStack->undo();
      issea(pLayer->getTickSpacing() == oldTickSpacing);
      pUndoStack->redo();
      issea(pLayer->getTickSpacing() == newTickSpacing);

      // Auto tick spacing
      bool oldAutoSpacing = pLayer->getAutoTickSpacing();
      bool newAutoSpacing = !oldAutoSpacing;

      pLayer->setAutoTickSpacing(newAutoSpacing);
      issea(pLayer->getAutoTickSpacing() == newAutoSpacing);
      pUndoStack->undo();
      issea(pLayer->getAutoTickSpacing() == oldAutoSpacing);
      pUndoStack->redo();
      issea(pLayer->getAutoTickSpacing() == newAutoSpacing);

      // Font
      QFont oldFont = pLayer->getFont().getQFont();
      QFont newFont = oldFont;
      newFont.setBold(!oldFont.bold());
      newFont.setItalic(!oldFont.italic());
      newFont.setUnderline(!oldFont.underline());

      FactoryResource<Font> font;
      font->setQFont(newFont);

      pLayer->setFont(*font.get());
      issea(pLayer->getFont().getQFont() == newFont);
      pUndoStack->undo();
      issea(pLayer->getFont().getQFont() == oldFont);
      pUndoStack->redo();
      issea(pLayer->getFont().getQFont() == newFont);

      // Geocoord type
      GeocoordType oldGeocoord = pLayer->getGeocoordType();
      GeocoordType newGeocoord = static_cast<GeocoordTypeEnum>((oldGeocoord + 1) % 2);

      pLayer->setGeocoordType(newGeocoord);
      issea(pLayer->getGeocoordType() == newGeocoord);
      pUndoStack->undo();
      issea(pLayer->getGeocoordType() == oldGeocoord);
      pUndoStack->redo();
      issea(pLayer->getGeocoordType() == newGeocoord);

      // Format
      DmsFormatType oldFormat = pLayer->getLatLonFormat();
      DmsFormatType newFormat = static_cast<DmsFormatTypeEnum>((oldFormat + 1) % 2);

      pLayer->setLatLonFormat(newFormat);
      issea(pLayer->getLatLonFormat() == newFormat);
      pUndoStack->undo();
      issea(pLayer->getLatLonFormat() == oldFormat);
      pUndoStack->redo();
      issea(pLayer->getLatLonFormat() == newFormat);

      // Delete layer
      pView->deleteLayer(pLayer);
      issea(pLayerList->getNumLayers(LAT_LONG) == 0);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(LAT_LONG) == 1);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(LAT_LONG) == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class LayerUndoTestCase : public TestCase
{
public:
   LayerUndoTestCase() : TestCase("Layer") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      Layer* pLayer = pView->createLayer(AOI_LAYER);
      issea(pLayer != NULL);

      pView->hideLayer(pLayer);

      // Show
      pView->showLayer(pLayer);
      issea(pView->isLayerDisplayed(pLayer) == true);
      pUndoStack->undo();
      issea(pView->isLayerDisplayed(pLayer) == false);
      pUndoStack->redo();
      issea(pView->isLayerDisplayed(pLayer) == true);

      // Hide
      pView->hideLayer(pLayer);
      issea(pView->isLayerDisplayed(pLayer) == false);
      pUndoStack->undo();
      issea(pView->isLayerDisplayed(pLayer) == true);
      pUndoStack->redo();
      issea(pView->isLayerDisplayed(pLayer) == false);

      pView->showLayer(pLayer);

      // Name
      string oldName = pLayer->getName();
      string newName = oldName + string(" New");

      pLayerList->renameLayer(pLayer, newName);
      issea(pLayer->getName() == newName);
      pUndoStack->undo();
      issea(pLayer->getName() == oldName);
      pUndoStack->redo();
      issea(pLayer->getName() == newName);

      // Display index
      int oldIndex = pView->getLayerDisplayIndex(pLayer);
      int newIndex = oldIndex + 1;

      pView->setLayerDisplayIndex(pLayer, newIndex);
      issea(pView->getLayerDisplayIndex(pLayer) == newIndex);
      pUndoStack->undo();
      issea(pView->getLayerDisplayIndex(pLayer) == oldIndex);
      pUndoStack->redo();
      issea(pView->getLayerDisplayIndex(pLayer) == newIndex);

      // Delete layer
      pView->deleteLayer(pLayer);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class ProductViewUndoTestCase : public TestCase
{
public:
   ProductViewUndoTestCase() : TestCase("ProductView") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      ProductWindow* pWindow = dynamic_cast<ProductWindow*>(pDesktop->createWindow("UndoProduct", PRODUCT_WINDOW));
      issea(pWindow != NULL);

      ProductViewImp* pView = dynamic_cast<ProductViewImp*>(pWindow->getProductView());
      issea(pView != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Paper size
      double oldWidth = 0.0;
      double oldHeight = 0.0;
      pView->getPaperSize(oldWidth, oldHeight);

      double newWidth = oldWidth + 2.0;
      double newHeight = oldHeight + 2.0;

      pView->setPaperSize(newWidth, newHeight);

      double width = 0.0;
      double height = 0.0;

      pView->getPaperSize(width, height);
      issea(width == newWidth);
      issea(height == newHeight);

      pUndoStack->undo();

      pView->getPaperSize(width, height);
      issea(width == oldWidth);
      issea(height == oldHeight);

      pUndoStack->redo();

      pView->getPaperSize(width, height);
      issea(width == newWidth);
      issea(height == newHeight);

      // Paper color
      QColor oldColor = pView->getPaperColor();
      QColor newColor(255 - oldColor.red(), 255 - oldColor.green(), 255 - oldColor.blue());

      pView->setPaperColor(newColor);
      issea(pView->getPaperColor() == newColor);
      pUndoStack->undo();
      issea(pView->getPaperColor() == oldColor);
      pUndoStack->redo();
      issea(pView->getPaperColor() == newColor);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class PseudocolorUndoTestCase : public TestCase
{
public:
   PseudocolorUndoTestCase() : TestCase("Pseudocolor") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      ThresholdLayer* pThresholdLayer = TestUtilities::createThresholdLayer(pRasterElement, 0, -1);
      issea(pThresholdLayer != NULL);
      if (pThresholdLayer == NULL)
      {
         return false;
      }

      PseudocolorLayerImp* pLayer = dynamic_cast<PseudocolorLayerImp*>(pView->convertLayer(pThresholdLayer,
         PSEUDOCOLOR));
      issea(pLayer != NULL);
      if (pLayer == NULL)
      {
         return false;
      }

      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 1);

      pLayer = dynamic_cast<PseudocolorLayerImp*>(pView->getTopMostLayer(PSEUDOCOLOR));
      issea(pLayer != NULL);

      // Create class
      PseudocolorClass* pClass = pLayer->addClass("PseudocolorClass", 10, QColor(255, 0, 255), true);
      issea(pClass != NULL);
      issea(pLayer->getClassCount() == 1);
      pUndoStack->undo();
      issea(pLayer->getClassCount() == 0);
      pUndoStack->redo();
      issea(pLayer->getClassCount() == 1);

      pClass = pLayer->getClass(10);
      issea(pClass != NULL);

      // Name
      QString oldName = pClass->getName();
      QString newName = "NewPseudocolorClass";

      pClass->setClassName(newName);
      issea(pClass->getName() == newName);
      pUndoStack->undo();
      issea(pClass->getName() == oldName);
      pUndoStack->redo();
      issea(pClass->getName() == newName);

      // Value
      int oldValue = pClass->getValue();
      int newValue = oldValue + 5;

      pClass->setValue(newValue);
      issea(pClass->getValue() == newValue);
      pUndoStack->undo();
      issea(pClass->getValue() == oldValue);
      pUndoStack->redo();
      issea(pClass->getValue() == newValue);

      // Color
      QColor oldColor = pClass->getColor();
      QColor newColor(255 - oldColor.red(), 255 - oldColor.green(), 255 - oldColor.blue());

      pClass->setColor(newColor);
      issea(pClass->getColor() == newColor);
      pUndoStack->undo();
      issea(pClass->getColor() == oldColor);
      pUndoStack->redo();
      issea(pClass->getColor() == newColor);

      // Displayed
      bool oldDisplayed = pClass->isDisplayed();
      bool newDisplayed = !oldDisplayed;

      pClass->setDisplayed(newDisplayed);
      issea(pClass->isDisplayed() == newDisplayed);
      pUndoStack->undo();
      issea(pClass->isDisplayed() == oldDisplayed);
      pUndoStack->redo();
      issea(pClass->isDisplayed() == newDisplayed);

      // Delete class
      pLayer->removeClass(pClass);
      issea(pLayer->getClassCount() == 0);
      pUndoStack->undo();
      issea(pLayer->getClassCount() == 1);
      pUndoStack->redo();
      issea(pLayer->getClassCount() == 0);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(PSEUDOCOLOR, pLayer->getDataElement(), "AnotherPseudocolorLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 2);

      pView->deleteLayer(dynamic_cast<Layer*>(pLayer));
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(PSEUDOCOLOR) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class RasterUndoTestCase : public TestCase
{
public:
   RasterUndoTestCase() : TestCase("Raster") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      ThresholdLayer* pThresholdLayer = TestUtilities::createThresholdLayer(pRasterElement, 0, -1);
      issea(pThresholdLayer != NULL);
      if (pThresholdLayer == NULL)
      {
         return false;
      }

      RasterLayerImp* pLayer = dynamic_cast<RasterLayerImp*>(pView->convertLayer(pThresholdLayer, RASTER));
      issea(pLayer != NULL);
      if (pLayer == NULL)
      {
         return false;
      }

      issea(pLayerList->getNumLayers(RASTER) == 2);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(RASTER) == 1);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(RASTER) == 2);

      pLayer = dynamic_cast<RasterLayerImp*>(pView->getTopMostLayer(RASTER));
      issea(pLayer != NULL);

      // Display mode
      DisplayMode oldDisplayMode = pLayer->getDisplayMode();
      DisplayMode newDisplayMode = static_cast<DisplayModeEnum>((oldDisplayMode + 1) % 2);

      pLayer->setDisplayMode(newDisplayMode);
      issea(pLayer->getDisplayMode() == newDisplayMode);
      pUndoStack->undo();
      issea(pLayer->getDisplayMode() == oldDisplayMode);
      pUndoStack->redo();
      issea(pLayer->getDisplayMode() == newDisplayMode);

      // Color map
      string oldColorMapFile = TestUtilities::getTestDataPath() + "ColorTables/HotIron.clu";
      string newColorMapFile = TestUtilities::getTestDataPath() + "ColorTables/WeatherMap.clu";

      ColorMap oldColorMap(oldColorMapFile);
      ColorMap newColorMap(newColorMapFile);

      pLayer->setColorMap(oldColorMap);
      issea(pLayer->getColorMap() == oldColorMap);
      pLayer->setColorMap(newColorMap);
      issea(pLayer->getColorMap() == newColorMap);
      pUndoStack->undo();
      issea(pLayer->getColorMap() == oldColorMap);
      pUndoStack->redo();
      issea(pLayer->getColorMap() == newColorMap);

      // Complex component
      ComplexComponent oldComplexComponent = pLayer->getComplexComponent();
      ComplexComponent newComplexComponent = static_cast<ComplexComponentEnum>((oldComplexComponent + 1) % 2);

      pLayer->setComplexComponent(newComplexComponent);
      issea(pLayer->getComplexComponent() == newComplexComponent);
      pUndoStack->undo();
      issea(pLayer->getComplexComponent() == oldComplexComponent);
      pUndoStack->redo();
      issea(pLayer->getComplexComponent() == newComplexComponent);

      // Stretch type
      StretchType oldStretchType = pLayer->getStretchType(GRAYSCALE_MODE);
      StretchType newStretchType = static_cast<StretchTypeEnum>((oldStretchType + 1) % 2);

      pLayer->setStretchType(GRAYSCALE_MODE, newStretchType);
      issea(pLayer->getStretchType(GRAYSCALE_MODE) == newStretchType);
      pUndoStack->undo();
      issea(pLayer->getStretchType(GRAYSCALE_MODE) == oldStretchType);
      pUndoStack->redo();
      issea(pLayer->getStretchType(GRAYSCALE_MODE) == newStretchType);

      // Stretch units
      RegionUnits oldStretchUnits = pLayer->getStretchUnits(GRAY);
      RegionUnits newStretchUnits = static_cast<RegionUnitsEnum>((oldStretchUnits + 1) % 2);

      pLayer->setStretchUnits(GRAY, newStretchUnits);
      issea(pLayer->getStretchUnits(GRAY) == newStretchUnits);
      pUndoStack->undo();
      issea(pLayer->getStretchUnits(GRAY) == oldStretchUnits);
      pUndoStack->redo();
      issea(pLayer->getStretchUnits(GRAY) == newStretchUnits);

      // Stretch values
      double oldLower = 0.0;
      double oldUpper = 0.0;
      pLayer->getStretchValues(GRAY, oldLower, oldUpper);

      double newLower = 100.0 - oldLower;
      double newUpper = 100.0 - oldUpper;
      double currentLower = 0.0;
      double currentUpper = 0.0;

      pLayer->setStretchValues(GRAY, newLower, newUpper);
      pLayer->getStretchValues(GRAY, currentLower, currentUpper);
      issea((currentLower == newLower) && (currentUpper == newUpper));
      pUndoStack->undo();
      pLayer->getStretchValues(GRAY, currentLower, currentUpper);
      issea((currentLower == oldLower) && (currentUpper == oldUpper));
      pUndoStack->redo();
      pLayer->getStretchValues(GRAY, currentLower, currentUpper);
      issea((currentLower == newLower) && (currentUpper == newUpper));

      // Displayed band
      RasterLayer* pCubeLayer = static_cast<RasterLayer*>(pLayerList->getLayer(RASTER, pRasterElement));
      issea(pCubeLayer != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      issea(pDescriptor != NULL);

      DimensionDescriptor oldBand = pCubeLayer->getDisplayedBand(GRAY);
      DimensionDescriptor newBand = pDescriptor->getActiveBand(oldBand.getActiveNumber() + 1);

      pCubeLayer->setDisplayedBand(GRAY, newBand);
      issea(pCubeLayer->getDisplayedBand(GRAY) == newBand);
      pUndoStack->undo();
      issea(pCubeLayer->getDisplayedBand(GRAY) == oldBand);
      pUndoStack->redo();
      issea(pCubeLayer->getDisplayedBand(GRAY) == newBand);

      // GPU image
      if (pCubeLayer->isGpuImageSupported() == true)
      {
         pCubeLayer->enableGpuImage(false);
         issea(pCubeLayer->isGpuImageEnabled() == false);
         pCubeLayer->enableGpuImage(true);
         issea(pCubeLayer->isGpuImageEnabled() == true);
         pUndoStack->undo();
         issea(pCubeLayer->isGpuImageEnabled() == false);
         pUndoStack->redo();
         issea(pCubeLayer->isGpuImageEnabled() == true);

         // Image filter
         vector<string> oldFilters;
         oldFilters.push_back("ByPass");

         vector<string> newFilters;
         newFilters.push_back("EdgeDetection");

         pCubeLayer->enableFilters(oldFilters);
         issea(pCubeLayer->getEnabledFilterNames() == oldFilters);
         pCubeLayer->enableFilters(newFilters);
         issea(pCubeLayer->getEnabledFilterNames() == newFilters);

         pUndoStack->undo();

         vector<ImageFilterDescriptor*> filters = pCubeLayer->getEnabledFilters();
         issea(filters.size() == 1);

         ImageFilterDescriptor* pFilterDescriptor = filters.front();
         issea(pFilterDescriptor != NULL);
         issea(pFilterDescriptor->getName() == "ByPass");

         pUndoStack->redo();

         filters = pCubeLayer->getEnabledFilters();
         issea(filters.size() == 1);

         pFilterDescriptor = filters.front();
         issea(pFilterDescriptor != NULL);
         issea(pFilterDescriptor->getName() == "EdgeDetection");
      }

      // Delete layer
      Layer* pNewLayer = pView->createLayer(RASTER, pLayer->getDataElement(), "AnotherRasterLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(RASTER) == 3);

      pView->deleteLayer(dynamic_cast<Layer*>(pLayer));
      issea(pLayerList->getNumLayers(RASTER) == 2);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(RASTER) == 3);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(RASTER) == 2);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(RASTER) == 1);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class ThresholdUndoTestCase : public TestCase
{
public:
   ThresholdUndoTestCase() : TestCase("Threshold") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      ThresholdLayer* pLayer = NULL;
      {
         UndoGroup group(dynamic_cast<View*>(pView), "Create Threshold Layer");
         pLayer = TestUtilities::createThresholdLayer(pRasterElement, 0, -1);
      }
      issea(pLayer != NULL);
      if (pLayer == NULL)
      {
         return false;
      }

      issea(pLayerList->getNumLayers(THRESHOLD) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(THRESHOLD) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(THRESHOLD) == 1);

      pLayer = dynamic_cast<ThresholdLayer*>(pView->getTopMostLayer(THRESHOLD));
      issea(pLayer != NULL);

      // Values
      double oldLower = pLayer->getFirstThreshold();
      double oldUpper = pLayer->getSecondThreshold();
      double newLower = oldLower + 5.0;
      double newUpper = oldUpper + 5.0;

      pLayer->setFirstThreshold(newLower);
      pLayer->setSecondThreshold(newUpper);
      issea(pLayer->getFirstThreshold() == newLower);
      issea(pLayer->getSecondThreshold() == newUpper);
      pUndoStack->undo();
      pUndoStack->undo();
      issea(pLayer->getFirstThreshold() == oldLower);
      issea(pLayer->getSecondThreshold() == oldUpper);
      pUndoStack->redo();
      pUndoStack->redo();
      issea(pLayer->getFirstThreshold() == newLower);
      issea(pLayer->getSecondThreshold() == newUpper);

      // Pass area
      PassArea oldPassArea = pLayer->getPassArea();
      PassArea newPassArea = static_cast<PassAreaEnum>((oldPassArea + 1) % 2);

      pLayer->setPassArea(newPassArea);
      issea(pLayer->getPassArea() == newPassArea);
      pUndoStack->undo();
      issea(pLayer->getPassArea() == oldPassArea);
      pUndoStack->redo();
      issea(pLayer->getPassArea() == newPassArea);

      // Units
      RegionUnits oldUnits = pLayer->getRegionUnits();
      RegionUnits newUnits = static_cast<RegionUnitsEnum>((oldUnits + 1) % 2);

      pLayer->setRegionUnits(newUnits);
      issea(pLayer->getRegionUnits() == newUnits);
      pUndoStack->undo();
      issea(pLayer->getRegionUnits() == oldUnits);
      pUndoStack->redo();
      issea(pLayer->getRegionUnits() == newUnits);

      // Color
      ColorType oldColor = pLayer->getColor();
      ColorType newColor(255 - oldColor.mRed, 255 - oldColor.mGreen, 255 - oldColor.mBlue);

      pLayer->setColor(newColor);
      issea(pLayer->getColor() == newColor);
      pUndoStack->undo();
      issea(pLayer->getColor() == oldColor);
      pUndoStack->redo();
      issea(pLayer->getColor() == newColor);

      // Symbol
      SymbolType oldSymbol = pLayer->getSymbol();
      SymbolType newSymbol = static_cast<SymbolTypeEnum>((oldSymbol + 1) % 8);

      pLayer->setSymbol(newSymbol);
      issea(pLayer->getSymbol() == newSymbol);
      pUndoStack->undo();
      issea(pLayer->getSymbol() == oldSymbol);
      pUndoStack->redo();
      issea(pLayer->getSymbol() == newSymbol);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(THRESHOLD, pLayer->getDataElement(), "AnotherThresholdLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(THRESHOLD) == 2);

      pView->deleteLayer(pLayer);
      issea(pLayerList->getNumLayers(THRESHOLD) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(THRESHOLD) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(THRESHOLD) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(THRESHOLD) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class TiePointUndoTestCase : public TestCase
{
public:
   TiePointUndoTestCase() : TestCase("TiePoint") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issea(pLayerList != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Create layer
      TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(pView->createLayer(TIEPOINT_LAYER));
      issea(pLayer != NULL);
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 0);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 1);

      pLayer = dynamic_cast<TiePointLayer*>(pView->getTopMostLayer(TIEPOINT_LAYER));
      issea(pLayer != NULL);

      // Tie points
      TiePointList* pTiePointList = static_cast<TiePointList*>(pLayer->getDataElement());
      issea(pTiePointList != NULL);

      TiePoint point1;
      point1.mReferencePoint.mX = 5;
      point1.mReferencePoint.mX = 10;
      point1.mMissionOffset.mX = 1.5;
      point1.mMissionOffset.mY = 2.5;

      vector<TiePoint> points;
      points.push_back(point1);
      pTiePointList->adoptTiePoints(points);

      vector<TiePoint> oldPoints = pTiePointList->getTiePoints();

      TiePoint point2;
      point2.mReferencePoint.mX = 6;
      point2.mReferencePoint.mX = 11;
      point2.mMissionOffset.mX = 3.5;
      point2.mMissionOffset.mY = 4.5;

      points.push_back(point2);
      pTiePointList->adoptTiePoints(points);

      vector<TiePoint> newPoints = pTiePointList->getTiePoints();

      pView->addUndoAction(new SetTiePoints(pTiePointList, oldPoints, newPoints));

      issea(pTiePointList->getTiePoints() == newPoints);
      pUndoStack->undo();
      issea(pTiePointList->getTiePoints() == oldPoints);
      pUndoStack->redo();
      issea(pTiePointList->getTiePoints() == newPoints);

      // Symbol size
      int oldSize = pLayer->getSymbolSize();
      int newSize = oldSize + 5;

      pLayer->setSymbolSize(newSize);
      issea(pLayer->getSymbolSize() == newSize);
      pUndoStack->undo();
      issea(pLayer->getSymbolSize() == oldSize);
      pUndoStack->redo();
      issea(pLayer->getSymbolSize() == newSize);

      // Color
      ColorType oldColor = pLayer->getColor();
      ColorType newColor(255 - oldColor.mRed, 255 - oldColor.mGreen, 255 - oldColor.mBlue);

      pLayer->setColor(newColor);
      issea(pLayer->getColor() == newColor);
      pUndoStack->undo();
      issea(pLayer->getColor() == oldColor);
      pUndoStack->redo();
      issea(pLayer->getColor() == newColor);

      // Labels
      bool oldLabels = pLayer->areLabelsEnabled();
      bool newLabels = !oldLabels;

      pLayer->enableLabels(newLabels);
      issea(pLayer->areLabelsEnabled() == newLabels);
      pUndoStack->undo();
      issea(pLayer->areLabelsEnabled() == oldLabels);
      pUndoStack->redo();
      issea(pLayer->areLabelsEnabled() == newLabels);

      // Delete layer
      Layer* pNewLayer = pView->createLayer(TIEPOINT_LAYER, pLayer->getDataElement(), "AnotherTiePointLayer");
      issea(pNewLayer != NULL);
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 2);

      pView->deleteLayer(pLayer);
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 1);
      pUndoStack->undo();
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 2);
      pUndoStack->redo();
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 1);

      pView->deleteLayer(pNewLayer);
      issea(pLayerList->getNumLayers(TIEPOINT_LAYER) == 0);
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class ViewUndoTestCase : public TestCase
{
public:
   ViewUndoTestCase() : TestCase("View") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Classification font
      QFont oldFont = pView->getClassificationFont();
      QFont newFont = oldFont;
      newFont.setBold(!oldFont.bold());
      newFont.setItalic(!oldFont.italic());
      newFont.setUnderline(!oldFont.underline());

      pView->setClassificationFont(newFont);
      issea(pView->getClassificationFont() == newFont);
      pUndoStack->undo();
      issea(pView->getClassificationFont() == oldFont);
      pUndoStack->redo();
      issea(pView->getClassificationFont() == newFont);

      // Classification color
      QColor oldClassificationColor = pView->getClassificationColor();
      QColor newClassificationColor(255 - oldClassificationColor.red(), 255 - oldClassificationColor.green(),
         255 - oldClassificationColor.blue());

      pView->setClassificationColor(newClassificationColor);
      issea(pView->getClassificationColor() == newClassificationColor);
      pUndoStack->undo();
      issea(pView->getClassificationColor() == oldClassificationColor);
      pUndoStack->redo();
      issea(pView->getClassificationColor() == newClassificationColor);

      // Background color
      QColor oldBackgroundColor = pView->getBackgroundColor();
      QColor newBackgroundColor(255 - oldBackgroundColor.red(), 255 - oldBackgroundColor.green(),
         255 - oldBackgroundColor.blue());

      pView->setBackgroundColor(newBackgroundColor);
      issea(pView->getBackgroundColor() == newBackgroundColor);
      pUndoStack->undo();
      issea(pView->getBackgroundColor() == oldBackgroundColor);
      pUndoStack->redo();
      issea(pView->getBackgroundColor() == newBackgroundColor);

      // Data origin
      DataOrigin oldOrigin = pView->getDataOrigin();
      DataOrigin newOrigin = static_cast<DataOriginEnum>((oldOrigin + 1) % 2);

      pView->setDataOrigin(newOrigin);
      issea(pView->getDataOrigin() == newOrigin);
      pUndoStack->undo();
      issea(pView->getDataOrigin() == oldOrigin);
      pUndoStack->redo();
      issea(pView->getDataOrigin() == newOrigin);

      // Zoom box
      LocationType oldLowerLeft;
      LocationType oldUpperLeft;
      LocationType oldUpperRight;
      LocationType oldLowerRight;
      pView->getVisibleCorners(oldLowerLeft, oldUpperLeft, oldUpperRight, oldLowerRight);

      LocationType newLowerLeft = oldLowerLeft + 20.0;
      LocationType newUpperLeft = oldUpperLeft + 20.0;
      LocationType newUpperRight = oldUpperRight + 20.0;
      LocationType newLowerRight = oldLowerRight + 20.0;

      pView->zoomToBox(newLowerLeft, newUpperRight);

      LocationType lowerLeft;
      LocationType upperLeft;
      LocationType upperRight;
      LocationType lowerRight;
      pView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

      const double compareThreshold = 1e-4;
      issea(fabs(lowerLeft.mX - newLowerLeft.mX) < compareThreshold);
      issea(fabs(lowerLeft.mY - newLowerLeft.mY) < compareThreshold);
      issea(fabs(upperLeft.mX - newUpperLeft.mX) < compareThreshold);
      issea(fabs(upperLeft.mY - newUpperLeft.mY) < compareThreshold);
      issea(fabs(upperRight.mX - newUpperRight.mX) < compareThreshold);
      issea(fabs(upperRight.mY - newUpperRight.mY) < compareThreshold);
      issea(fabs(lowerRight.mX - newLowerRight.mX) < compareThreshold);
      issea(fabs(lowerRight.mY - newLowerRight.mY) < compareThreshold);

      pUndoStack->undo();
      pView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

      issea(fabs(lowerLeft.mX - oldLowerLeft.mX) < compareThreshold);
      issea(fabs(lowerLeft.mY - oldLowerLeft.mY) < compareThreshold);
      issea(fabs(upperLeft.mX - oldUpperLeft.mX) < compareThreshold);
      issea(fabs(upperLeft.mY - oldUpperLeft.mY) < compareThreshold);
      issea(fabs(upperRight.mX - oldUpperRight.mX) < compareThreshold);
      issea(fabs(upperRight.mY - oldUpperRight.mY) < compareThreshold);
      issea(fabs(lowerRight.mX - oldLowerRight.mX) < compareThreshold);
      issea(fabs(lowerRight.mY - oldLowerRight.mY) < compareThreshold);

      pUndoStack->redo();
      pView->getVisibleCorners(lowerLeft, upperLeft, upperRight, lowerRight);

      issea(fabs(lowerLeft.mX - newLowerLeft.mX) < compareThreshold);
      issea(fabs(lowerLeft.mY - newLowerLeft.mY) < compareThreshold);
      issea(fabs(upperLeft.mX - newUpperLeft.mX) < compareThreshold);
      issea(fabs(upperLeft.mY - newUpperLeft.mY) < compareThreshold);
      issea(fabs(upperRight.mX - newUpperRight.mX) < compareThreshold);
      issea(fabs(upperRight.mY - newUpperRight.mY) < compareThreshold);
      issea(fabs(lowerRight.mX - newLowerRight.mX) < compareThreshold);
      issea(fabs(lowerRight.mY - newLowerRight.mY) < compareThreshold);

      // Zoom percent
      double oldPercent = pView->getZoomPercentage();
      double newPercent = oldPercent + 50;

      pView->zoomTo(newPercent);
      issea(fabs(pView->getZoomPercentage() - newPercent) < compareThreshold);
      pUndoStack->undo();
      issea(fabs(pView->getZoomPercentage() - oldPercent) < compareThreshold);
      pUndoStack->redo();
      issea(fabs(pView->getZoomPercentage() - newPercent) < compareThreshold);

      // Pan
      LocationType oldCenter = pView->getVisibleCenter();
      LocationType newCenter = oldCenter + 20.0;

      pView->panTo(newCenter);

      LocationType center = pView->getVisibleCenter();
      issea(fabs(center.mX - newCenter.mX) < compareThreshold);
      issea(fabs(center.mY - newCenter.mY) < compareThreshold);

      pUndoStack->undo();

      center = pView->getVisibleCenter();
      issea(fabs(center.mX - oldCenter.mX) < compareThreshold);
      issea(fabs(center.mY - oldCenter.mY) < compareThreshold);

      pUndoStack->redo();

      center = pView->getVisibleCenter();
      issea(fabs(center.mX - newCenter.mX) < compareThreshold);
      issea(fabs(center.mY - newCenter.mY) < compareThreshold);

      // Rotate
      double oldAngle = pView->getRotation();
      double newAngle = oldAngle + 60.0;

      pView->rotateTo(newAngle);
      issea(fabs(pView->getRotation() - newAngle) < compareThreshold);
      pUndoStack->undo();
      issea(fabs(pView->getRotation() - oldAngle) < compareThreshold);
      pUndoStack->redo();
      issea(fabs(pView->getRotation() - newAngle) < compareThreshold);

      // Flip
      double oldPitch = pView->getPitch();
      double newPitch = oldPitch + 30.0;

      pView->flipTo(newPitch);
      issea(fabs(pView->getPitch() - newPitch) < compareThreshold);
      pUndoStack->undo();
      issea(fabs(pView->getPitch() - oldPitch) < compareThreshold);
      pUndoStack->redo();
      issea(fabs(pView->getPitch() - newPitch) < compareThreshold);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class ImportUndoTestCase : public TestCase
{
public:
   ImportUndoTestCase() : TestCase("Import") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Ensure there are no undo actions in the stack
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class UndoLimitTestCase : public TestCase
{
public:
   UndoLimitTestCase() : TestCase("UndoLimit") {}

   bool run()
   {
      bool success = true;

      // Get the undo stack from the view
      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea(pRasterElement != NULL);

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);

      SpatialDataViewImp* pView = dynamic_cast<SpatialDataViewImp*>(pWindow->getSpatialDataView());
      issea(pView != NULL);

      UndoStack* pUndoStack = pView->getUndoStack();
      issea(pUndoStack != NULL);

      // Limit the number of actions in the undo stack
      const int newUndoLimit = 10;
      pUndoStack->setUndoLimit(newUndoLimit);
      issea(pUndoStack->undoLimit() == newUndoLimit);

      // Add actions to the stack past the limit
      for (int i = 0; i < newUndoLimit + 5; ++i)
      {
         TestUndoAction* pUndoAction = new TestUndoAction();
         if (pUndoAction != NULL)
         {
            QString actionText = "Action " + QString::number(i + 1);
            pUndoAction->setText(actionText);

            pView->addUndoAction(pUndoAction);
         }

         // Verify that the number of actions is never greater than the limit
         if (i < newUndoLimit)
         {
            issea(pUndoStack->count() == (i + 1));
         }
         else
         {
            issea(pUndoStack->count() == newUndoLimit);
         }
      }

      // Clear the stack
      pUndoStack->clear();
      issea(pUndoStack->count() == 0);

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }

private:
   class TestUndoAction : public UndoAction
   {
   public:
      TestUndoAction() : UndoAction() {}
      void executeUndo() {}
      void executeRedo() {}
   };
};

class UndoTestSuite : public TestSuiteNewSession
{
public:
   UndoTestSuite() :
      TestSuiteNewSession("Undo")
   {
      // Undo actions
      addTestCase(new AnnotationUndoTestCase);
      addTestCase(new AoiUndoTestCase);
      addTestCase(new GcpUndoTestCase);
      addTestCase(new LatLonUndoTestCase);
      addTestCase(new LayerUndoTestCase);
      addTestCase(new ProductViewUndoTestCase);
      addTestCase(new PseudocolorUndoTestCase);
      addTestCase(new RasterUndoTestCase);
      addTestCase(new ThresholdUndoTestCase);
      addTestCase(new TiePointUndoTestCase);
      addTestCase(new ViewUndoTestCase);

      // Undo behavior
      addTestCase(new ImportUndoTestCase);
      addTestCase(new UndoLimitTestCase);
   }
};

REGISTER_SUITE( UndoTestSuite )
