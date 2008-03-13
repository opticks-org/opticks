/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayerImp.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "CgmObject.h"
#include "CgmObjectImp.h"
#include "DrawUtil.h"
#include "glCommon.h"
#include "GraphicElement.h"
#include "GraphicElementImp.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "GraphicObject.h"
#include "GraphicObjectFactory.h"
#include "GraphicObjectImp.h"
#include "LayerList.h"
#include "OrthographicView.h"
#include "PerspectiveView.h"
#include "PropertiesGraphicObject.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "ViewImp.h"
#include "ViewObjectImp.h"
#include "XercesIncludes.h"

#include <limits>
#include <math.h>

using namespace std;
XERCES_CPP_NAMESPACE_USE

GraphicObjectImp::GraphicObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                   LocationType pixelCoord) :
SessionItemImp(id, StringUtilities::toDisplayString(type) + ((pLayer != NULL)
                        ? StringUtilities::toDisplayString(dynamic_cast<GraphicGroupImp*>(
                                    dynamic_cast<GraphicElement*>(pLayer->getDataElement())->getGroup())->getNextObjectNumber())
                        : "")),
   mDisplayListDirty(true),
   mBitMaskDirty(true),
   mType(type),
   mpLayer(pLayer)
{
   addProperty("BoundingBox");
   addProperty("Rotation");

   VERIFYNR(connect(this, SIGNAL(propertyModified(GraphicProperty*)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(nameChanged(const QString&)), this, SIGNAL(modified())));
   VERIFYNR(connect(this, SIGNAL(modified()), this, SLOT(setCacheDirty())));
   VERIFYNR(connect(this, SIGNAL(modified()), this, SLOT(subjectModified())));

   setBoundingBox(pixelCoord, pixelCoord);
   addPropertiesPage(PropertiesGraphicObject::getName());
}

GraphicObjectImp::~GraphicObjectImp()
{
   vector<GraphicProperty*>::iterator it;
   for (it = mProperties.begin(); it != mProperties.end(); it++)
   {
      delete *it;
   }
}

bool GraphicObjectImp::replicateObject(const GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   GraphicObjectType eCurrentType = getGraphicObjectType();
   GraphicObjectType eNewType = pObject->getGraphicObjectType();

   if (eCurrentType != eNewType)
   {
      return false;
   }

   const GraphicObjectImp* pExistingObject = dynamic_cast<const GraphicObjectImp*>(pObject);

   if (this != pExistingObject)
   {
      string objectName = pExistingObject->getName();
      setName(objectName);

      vector<GraphicProperty*> properties = pExistingObject->getProperties();
      setProperties(properties);
      updateHandles();
   }

   return true;
}

CgmObject* GraphicObjectImp::convertToCgm()
{
   GraphicResource<CgmObjectImp> pCgm(CGM_OBJECT, mpLayer.get(), LocationType());
   if (pCgm.get() != NULL)
   {
      double dAngle = getRotation();
      GraphicObjectType eType = getGraphicObjectType();

      GraphicObjectImp* pObject = NULL;
      pObject = dynamic_cast<GraphicObjectImp*>(pCgm->addObject(eType));
      if (pObject != NULL)
      {
         bool bSuccess = false;
         bSuccess = pObject->replicateObject(dynamic_cast<GraphicObject*>(this));
         if (bSuccess == false)
         {
            pCgm->removeObject(dynamic_cast<GraphicObject*>(pObject), true);
            return NULL;
         }
      }

      pCgm->setRotation(dAngle);
      pCgm->updateBoundingBox();
   }

   return dynamic_cast<CgmObject*>(pCgm.release());
}

GraphicProperty* GraphicObjectImp::createProperty(const string& propertyName)
{
   GraphicProperty* pProperty = NULL;
   if (propertyName == "Lat/Long" || propertyName == "LatLon")
   {
      pProperty = new LatLonProperty(LatLonPoint());
   }
   else if (propertyName == "LineColor")
   {
      pProperty = new LineColorProperty(GraphicLayer::getSettingLineColor());
   }
   else if (propertyName == "LineWidth")
   {
      pProperty = new LineWidthProperty(GraphicLayer::getSettingLineWidth());
   }
   else if (propertyName == "Apex")
   {
      pProperty = new ApexProperty(GraphicLayer::getSettingApex());
   }
   else if (propertyName == "Wedge")
   {
      pProperty = new WedgeProperty(GraphicLayer::getSettingStartAngle(),
         GraphicLayer::getSettingStopAngle());
   }
   else if (propertyName == "Scale")
   {
      pProperty = new ScaleProperty(1.0);
   }
   else if (propertyName == "Rotation")
   {
      pProperty = new RotationProperty(GraphicLayer::getSettingRotation());
   }
   else if (propertyName == "FillColor")
   {
      pProperty = new FillColorProperty(GraphicLayer::getSettingFillColor());
   }
   else if (propertyName == "TextAlignment")
   {
      pProperty = new TextAlignmentProperty(Qt::AlignLeft);
   }
   else if (propertyName == "TextColor")
   {
      pProperty = new TextColorProperty(GraphicLayer::getSettingTextColor());
   }
   else if (propertyName == "LineOn")
   {
      pProperty = new LineOnProperty(GraphicLayer::getSettingBorder());
   }
   else if (propertyName == "Font")
   {
      pProperty = new FontProperty(AnnotationLayerImp::getDefaultFont());
   }
   else if (propertyName == "TextString")
   {
      pProperty = new TextStringProperty(string());
   }
   else if (propertyName == "BoundingBox")
   {
      pProperty = new BoundingBoxProperty(LocationType(), LocationType());
   }
   else if (propertyName == "LineStyle")
   {
      pProperty = new LineStyleProperty(GraphicLayer::getSettingLineStyle());
   }
   else if (propertyName == "FillStyle")
   {
      pProperty = new FillStyleProperty(GraphicLayer::getSettingFillStyle());
   }
   else if (propertyName == "HatchStyle")
   {
      pProperty = new HatchStyleProperty(GraphicLayer::getSettingHatchStyle());
   }
   else if (propertyName == "ArcRegion")
   {
      pProperty = new ArcRegionProperty(GraphicLayer::getSettingArcRegion());
   }
   else if (propertyName == "PaperSize")
   {
      pProperty = new PaperSizeProperty(LocationType(11.0, 8.5));
   }
   else if (propertyName == "Alpha")
   {
      pProperty = new AlphaProperty(GraphicLayer::getSettingAlpha());
   }
   else if (propertyName == "Filename")
   {
      pProperty = new FileNameProperty(string());
   }
   else if (propertyName == "PixelSymbol")
   {
      pProperty = new PixelSymbolProperty(SOLID);
   }
   else if (propertyName == "DrawMode")
   {
      pProperty = new DrawModeProperty(DRAW);
   }
   else if (propertyName == "GraphicSymbol")
   {
      pProperty = new GraphicSymbolProperty(GraphicLayer::getSettingSymbolName());
   }
   else if (propertyName == "GraphicSymbolSize")
   {
      pProperty = new GraphicSymbolSizeProperty(GraphicLayer::getSettingSymbolSize());
   }
   else if (propertyName == "LineScaled")
   {
      pProperty = new LineScaledProperty(GraphicLayer::getSettingLineScaled());
   }

   return pProperty;
}

GraphicProperty* GraphicObjectImp::addProperty(const string& propertyName)
{
   if (propertyName.empty() == true)
   {
      return NULL;
   }

   if (hasProperty(propertyName) != NULL)
   {
      return NULL;
   }

   GraphicProperty* pProperty = NULL;
   pProperty = createProperty(propertyName);
   if (pProperty != NULL)
   {
      bool bSuccess = false;
      bSuccess = addProperty(pProperty);
      if (bSuccess == false)
      {
         delete pProperty;
         pProperty = NULL;
      }
   }

   return pProperty;
}

bool GraphicObjectImp::addProperty(GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   string propertyName = pProperty->getName();
   if (hasProperty(propertyName) == true)
   {
      return false;
   }

   mProperties.push_back(pProperty);
   return true;
}

bool GraphicObjectImp::hasProperty(const string& name) const
{
   return getProperty(name) != NULL;
}

bool GraphicObjectImp::setProperty(const GraphicProperty* pProp)
{
   if (pProp == NULL)
   {
      return false;
   }

   string propertyName = pProp->getName();

   GraphicProperty* pOldProp = NULL;
   pOldProp = getProperty(propertyName);
   if (pOldProp == NULL)
   {
      return false;
   }

   if (propertyName == "BoundingBox")
   {
      LocationType ll = static_cast<const BoundingBoxProperty*>(pProp)->getLlCorner();
      LocationType ur = static_cast<const BoundingBoxProperty*>(pProp)->getUrCorner();

      const RasterElement *pRaster = getGeoreferenceElement();

      if (pRaster != NULL)
      {
         bool changed = false;
         const BoundingBoxProperty* pBoundingBox = (const BoundingBoxProperty*) pProp;
         if (pBoundingBox->geoCoordsMatchPixelCoords() == false &&
            (pBoundingBox->hasGeoCoords() || pBoundingBox->hasPixelCoords()))
         {
            LocationType llCorner = pBoundingBox->getLlCorner();
            LocationType urCorner = pBoundingBox->getUrCorner();
            LocationType geoLlCorner = pBoundingBox->getLlLatLong();
            LocationType geoUrCorner = pBoundingBox->getUrLatLong();

            if (pBoundingBox->hasGeoCoords())
            {
               // If geo coordinates are available, use them
               llCorner = pRaster->convertGeocoordToPixel(geoLlCorner);
               urCorner = pRaster->convertGeocoordToPixel(geoUrCorner);
            }
            else
            {
               // Otherwise, use pixel coordinates
               geoLlCorner = pRaster->convertPixelToGeocoord(llCorner);
               geoUrCorner = pRaster->convertPixelToGeocoord(urCorner);
            }

            BoundingBoxProperty newProp(llCorner, urCorner, geoLlCorner, geoUrCorner, true);
            return setProperty(&newProp);
         }
      }
   }

   if (pProp->compare(pOldProp) == false)
   {
      GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetGraphicObjectProperty(dynamic_cast<GraphicObject*>(this), pOldProp, pProp));
         }
      }

      pOldProp->set(pProp);
      emit propertyModified(pOldProp);
   }

   return true;
}

GraphicProperty* GraphicObjectImp::getProperty(const string& name) const
{
   vector<GraphicProperty*>::const_iterator it;
   for (it = mProperties.begin(); it != mProperties.end(); ++it)
   {
      if ((*it)->getName() == name)
      {
         return *it;
      }
   }

   return NULL;
}

void GraphicObjectImp::setProperties(const vector<GraphicProperty*>& properties)
{
   vector<GraphicProperty*>::const_iterator it;
   for (it = properties.begin(); it != properties.end(); it++)
   {
      setProperty(*it);
   }
}

void GraphicObjectImp::updateGeo()
{
   BoundingBoxProperty *pProp = dynamic_cast<BoundingBoxProperty*>(getProperty("BoundingBox"));
   VERIFYNRV(pProp != NULL);
   
   LocationType llGeo = pProp->getLlLatLong();
   LocationType urGeo = pProp->getUrLatLong();

   BoundingBoxProperty newProp(llGeo, urGeo, true);
   setProperty(&newProp);
}

void GraphicObjectImp::enableGeo()
{
   BoundingBoxProperty *pProp = dynamic_cast<BoundingBoxProperty*>(getProperty("BoundingBox"));
   VERIFYNRV(pProp != NULL);
   
   BoundingBoxProperty newProp(pProp->getLlCorner(), pProp->getUrCorner());
   setProperty(&newProp);
}

const vector<LocationType>& GraphicObjectImp::getHandles() const
{
   return mHandles;
}

LocationType GraphicObjectImp::getHandle(int iHandle) const
{
   LocationType handleLocation;

   int iCount = 0;
   iCount = mHandles.size();
   if (iHandle < iCount)
   {
      handleLocation = mHandles.at(iHandle);
   }

   return handleLocation;
}

void GraphicObjectImp::moveHandle(int handle, LocationType point, bool bMaintainAspect)
{
   if (mHandles.empty())
   {
      updateHandles();
   }
   GraphicLayer *pLayer = getLayer();
   if (pLayer != NULL)
   {
      point = pLayer->correctCoordinate(point);
   }

   adjustHandles(handle, point, bMaintainAspect);
   setBoundingBox(mHandles[0], mHandles[4]);
}

void GraphicObjectImp::rotateHandle(int handle, LocationType delta)
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   LocationType center;
   center.mX = (llCorner.mX + urCorner.mX)/2.0;
   center.mY = (llCorner.mY + urCorner.mY)/2.0;

   double ax, ay, bx, by;

   ax = mHandles[handle].mX - center.mX;
   ay = mHandles[handle].mY - center.mY;

   bx = delta.mX - center.mX;
   by = delta.mY - center.mY;

   double theta = 0.0;
   theta = (180.0 / PI) * acos(((ax * bx) + (ay * by)) /
      (sqrt((ax * ax) + (ay * ay)) * sqrt((bx * bx) + (by * by))));

   double direction = 0.0;
   direction = (ax * by) - (ay * bx);
   if(direction < 0.0)
   {
      theta = -theta;
   }

   setRotation(theta);
}

void GraphicObjectImp::updateHandles()
{
   mHandles.clear();

   if (hasCornerHandles())
   {
      LocationType llCorner, urCorner;
      llCorner = getLlCorner();
      urCorner = getUrCorner();

      LocationType point;
      point.mX = llCorner.mX;
      point.mY = llCorner.mY;
      mHandles.push_back(point);
      point.mX = (llCorner.mX + urCorner.mX) / 2.0;
      mHandles.push_back (point);
      point.mX = urCorner.mX;
      mHandles.push_back (point);
      point.mY = (llCorner.mY + urCorner.mY) / 2.0;
      mHandles.push_back (point);
      point.mY = urCorner.mY;
      mHandles.push_back (point);
      point.mX = (llCorner.mX + urCorner.mX) / 2.0;
      mHandles.push_back (point);
      point.mX = llCorner.mX;
      mHandles.push_back (point);
      point.mY = (llCorner.mY + urCorner.mY) / 2.0;
      mHandles.push_back (point);
   }
}

void GraphicObjectImp::move(LocationType delta)
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   LocationType newLlCorner = llCorner + delta, newUrCorner = urCorner + delta;

   setBoundingBox(newLlCorner, newUrCorner);
   updateHandles();
}

const BitMask* GraphicObjectImp::getPixels()
{
   LocationType llCorner;
   LocationType urCorner;
   getRotatedBoundingBox(llCorner, urCorner);

   if (llCorner.mX > urCorner.mX)
   {
      double dTemp = llCorner.mX;
      llCorner.mX = urCorner.mX;
      urCorner.mX = dTemp;
   }

   if (llCorner.mY > urCorner.mY)
   {
      double dTemp = llCorner.mY;
      llCorner.mY = urCorner.mY;
      urCorner.mY = dTemp;
   }

   LocationType minPoint(numeric_limits<double>::min(), numeric_limits<double>::min());
   LocationType maxPoint(numeric_limits<double>::max(), numeric_limits<double>::max());
   if (llCorner == minPoint || urCorner == maxPoint)
   {
      mPixelMask.clear();
      return &mPixelMask;
   }

   return getPixels(floor(llCorner.mX), floor(llCorner.mY), ceil(urCorner.mX), ceil(urCorner.mY));
}

const BitMask* GraphicObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   mPixelMask.clear();

   double dAngle = getRotation();
   LocationType point;
   double modelMatrix[16], projectionMatrix[16];
   int viewPort[4];

   for (int i = iStartRow; i <= iEndRow; i++)
   {
      for (int j = iStartColumn; j <= iEndColumn; j++)
      {
         point.mX = j + 0.5;     // Add 0.5 to check the pixel center
         point.mY = i + 0.5;

         if (dAngle != 0.0)
         {
            double dScreenX = 0.0;
            double dScreenY = 0.0;

            glGetIntegerv(GL_VIEWPORT, viewPort);
            glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
            glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

            GLdouble winZ;
            gluProject(point.mX, point.mY, 0.0, modelMatrix, projectionMatrix, viewPort,
               &dScreenX, &dScreenY, &winZ);

            rotateViewMatrix();

            glGetIntegerv(GL_VIEWPORT, viewPort);
            glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
            glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

            DrawUtil::unProjectToZero(dScreenX, dScreenY, modelMatrix, projectionMatrix,
               viewPort, &point.mX, &point.mY);

            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
         }

         bool bHit = false;
         bHit = hit(point);
         if (bHit == true)
         {
            mPixelMask.setPixel(j, i, true);
         }
      }
   }

   return (BitMask*) &mPixelMask;
}

void GraphicObjectImp::rotateViewMatrix() const
{
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   LocationType center;
   center.mX = (llCorner.mX + urCorner.mX) / 2.0;
   center.mY = (llCorner.mY + urCorner.mY) / 2.0;

   double angle = getRotation();

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glTranslated(center.mX, center.mY, 0.0);
   glRotated(angle, 0.0, 0.0, 1.0);
   glTranslated(-center.mX, -center.mY, 0.0);
}

bool GraphicObjectImp::setBoundingBox(LocationType llCorner, LocationType urCorner)
{
   if (hasProperty("BoundingBox") == false)
   {
      return false;
   }

   BoundingBoxProperty boundingBoxProp(llCorner, urCorner);
   return setProperty(&boundingBoxProp);
}

LocationType GraphicObjectImp::getLlCorner() const
{
   LocationType llCorner(-1.0, -1.0);

   BoundingBoxProperty* pBoundBox = NULL;
   pBoundBox = (BoundingBoxProperty*) getProperty("BoundingBox");
   if (pBoundBox != NULL)
   {
      llCorner = pBoundBox->getLlCorner();
   }

   return llCorner;
}

LocationType GraphicObjectImp::getUrCorner() const
{
   LocationType urCorner(-1.0, -1.0);

   BoundingBoxProperty* pBoundBox = NULL;
   pBoundBox = (BoundingBoxProperty*) getProperty("BoundingBox");
   if (pBoundBox != NULL)
   {
      urCorner = pBoundBox->getUrCorner();
   }

   return urCorner;
}

void GraphicObjectImp::getRotatedBoundingBox(LocationType& llCorner, LocationType& urCorner) const
{
   llCorner = getLlCorner();
   urCorner = getUrCorner();

   double dAngle = getRotation();
   if (dAngle == 0.0)
   {
      return;
   }

   if (llCorner.mX > urCorner.mX)
   {
      double dTemp = llCorner.mX;
      llCorner.mX = urCorner.mX;
      urCorner.mX = dTemp;
   }

   if (llCorner.mY > urCorner.mY)
   {
      double dTemp = llCorner.mY;
      llCorner.mY = urCorner.mY;
      urCorner.mY = dTemp;
   }

   // Update the bounding box for rotation
   double dTheta = 0.0;
   dTheta = (PI / 180.0) * dAngle;

   LocationType center;
   center.mX = (llCorner.mX + urCorner.mX) / 2.0;
   center.mY = (llCorner.mY + urCorner.mY) / 2.0;

   LocationType cp;
   LocationType cpp;

   double dStartRow = llCorner.mY;
   double dEndRow = urCorner.mY;
   double dStartColumn = llCorner.mX;
   double dEndColumn = urCorner.mX;

   // Lower left
   cp.mX = llCorner.mX - center.mX;
   cp.mY = llCorner.mY - center.mY;
   cpp.mX = (cos(dTheta) * cp.mX) - (sin(dTheta) * cp.mY);
   cpp.mY = (cos(dTheta) * cp.mY) + (sin(dTheta) * cp.mX);

   dStartColumn = DrawUtil::minimum(dStartColumn, cpp.mX + center.mX);
   dStartRow = DrawUtil::minimum(dStartRow, cpp.mY + center.mY);

   // Lower right
   cp.mX = urCorner.mX - center.mX;
   cp.mY = llCorner.mY - center.mY;
   cpp.mX = (cos(dTheta) * cp.mX) - (sin(dTheta) * cp.mY);
   cpp.mY = (cos(dTheta) * cp.mY) + (sin(dTheta) * cp.mX);

   dEndColumn = DrawUtil::maximum(dEndColumn, cpp.mX + center.mX);
   dStartRow = DrawUtil::minimum(dStartRow, cpp.mY + center.mY);

   // Upper right
   cp.mX = urCorner.mX - center.mX;
   cp.mY = urCorner.mY - center.mY;
   cpp.mX = (cos(dTheta) * cp.mX) - (sin(dTheta) * cp.mY);
   cpp.mY = (cos(dTheta) * cp.mY) + (sin(dTheta) * cp.mX);

   dEndColumn = DrawUtil::maximum(dEndColumn, cpp.mX + center.mX);
   dEndRow = DrawUtil::maximum(dEndRow, cpp.mY + center.mY);

   // Upper left
   cp.mX = llCorner.mX - center.mX;
   cp.mY = urCorner.mY - center.mY;
   cpp.mX = (cos(dTheta) * cp.mX) - (sin(dTheta) * cp.mY);
   cpp.mY = (cos(dTheta) * cp.mY) + (sin(dTheta) * cp.mX);

   dStartColumn = DrawUtil::minimum(dStartColumn, cpp.mX + center.mX);
   dEndRow = DrawUtil::maximum(dEndRow, cpp.mY + center.mY);

   // Set the values
   llCorner.mX = dStartColumn;
   llCorner.mY = dStartRow;
   urCorner.mX = dEndColumn;
   urCorner.mY = dEndRow;
}

bool GraphicObjectImp::setRotation(double dAngle)
{
   if (hasProperty("Rotation") == false)
   {
      return false;
   }

   while (dAngle < 0.0)
   {
      dAngle += 360.0;
   }

   while (dAngle >= 360.0)
   {
      dAngle -= 360.0;
   }

   RotationProperty rotationProp(dAngle);
   return setProperty(&rotationProp);
}

double GraphicObjectImp::getRotation() const
{
   double dRotation = 0.0;

   RotationProperty* pRotation = NULL;
   pRotation = (RotationProperty*) getProperty("Rotation");
   if (pRotation != NULL)
   {
      dRotation = pRotation->getRotation();
   }

   return dRotation;
}

bool GraphicObjectImp::setLineState(bool bLine)
{
   if (hasProperty("LineOn") == false)
   {
      return false;
   }

   LineOnProperty lineOnProp(bLine);
   return setProperty(&lineOnProp);
}

bool GraphicObjectImp::getLineState(bool* pSuccess) const
{
   if (pSuccess != NULL)
   {
      *pSuccess = false;
   }

   bool bLine = false;

   LineOnProperty* pLineOn = NULL;
   pLineOn = (LineOnProperty*) getProperty("LineOn");
   if (pLineOn != NULL)
   {
      bLine = pLineOn->getState();
      if (pSuccess != NULL)
      {
         *pSuccess = true;
      }
   }

   return bLine;
}

bool GraphicObjectImp::setLineColor(ColorType lineColor)
{
   LineColorProperty lineColorProp(lineColor);
   return setProperty(&lineColorProp);
}

ColorType GraphicObjectImp::getLineColor() const
{
   ColorType lineColor;

   LineColorProperty* pLineColor = NULL;
   pLineColor = (LineColorProperty*) getProperty("LineColor");
   if (pLineColor != NULL)
   {
      lineColor = pLineColor->getColor();
   }

   return lineColor;
}

bool GraphicObjectImp::setLineWidth(double dWidth)
{
   if (hasProperty("LineWidth") == false)
   {
      return false;
   }

   LineWidthProperty lineWidthProp(dWidth);
   return setProperty(&lineWidthProp);
}

double GraphicObjectImp::getLineWidth() const
{
   double dWidth = -1.0;

   LineWidthProperty* pLineWidth = NULL;
   pLineWidth = (LineWidthProperty*) getProperty("LineWidth");
   if (pLineWidth != NULL)
   {
      dWidth = pLineWidth->getWidth();
   }

   return dWidth;
}

bool GraphicObjectImp::setLineStyle(LineStyle eLine)
{
   if (hasProperty("LineStyle") == false)
   {
      return false;
   }

   LineStyleProperty lineStyleProp(eLine);
   return setProperty(&lineStyleProp);
}

LineStyle GraphicObjectImp::getLineStyle() const
{
   LineStyle eStyle;

   LineStyleProperty* pLineStyle = NULL;
   pLineStyle = (LineStyleProperty*) getProperty("LineStyle");
   if (pLineStyle != NULL)
   {
      eStyle = pLineStyle->getStyle();
   }

   return eStyle;
}

bool GraphicObjectImp::setArcRegion(ArcRegion eRegion)
{
   if (hasProperty("ArcRegion") == false)
   {
      return false;
   }

   ArcRegionProperty arcRegionProp(eRegion);
   return setProperty(&arcRegionProp);
}

ArcRegion GraphicObjectImp::getArcRegion() const
{
   ArcRegion eRegion;

   ArcRegionProperty* pArcRegion = NULL;
   pArcRegion = (ArcRegionProperty*) getProperty("ArcRegion");
   if (pArcRegion != NULL)
   {
      eRegion = pArcRegion->getRegion();
   }

   return eRegion;
}

bool GraphicObjectImp::setAngles(double dStart, double dStop)
{
   if (hasProperty("Wedge") == false)
   {
      return false;
   }

   WedgeProperty wedgeProp(dStart, dStop);
   return setProperty(&wedgeProp);
}

bool GraphicObjectImp::setStartAngle(double dStart)
{
   if (hasProperty("Wedge") == false)
   {
      return false;
   }

   double dStop = 0.0;
   dStop = getStopAngle();

   setAngles(dStart, dStop);
   return true;
}

bool GraphicObjectImp::setStopAngle(double dStop)
{
   if (hasProperty("Wedge") == false)
   {
      return false;
   }

   double dStart = 0.0;
   dStart = getStartAngle();

   setAngles(dStart, dStop);
   return true;
}

double GraphicObjectImp::getStartAngle() const
{
   double dStart = -1.0;

   WedgeProperty* pWedge = NULL;
   pWedge = (WedgeProperty*) getProperty("Wedge");
   if (pWedge != NULL)
   {
      dStart = pWedge->getStartAngle();
   }

   return dStart;
}

double GraphicObjectImp::getStopAngle() const
{
   double dStop = -1.0;

   WedgeProperty* pWedge = NULL;
   pWedge = (WedgeProperty*) getProperty("Wedge");
   if (pWedge != NULL)
   {
      dStop = pWedge->getStopAngle();
   }

   return dStop;
}

bool GraphicObjectImp::setFillState(bool bFill)
{
   if (hasProperty("FillStyle") == false)
   {
      return false;
   }

   FillStyle eFill = getFillStyle();

   if (bFill == false)
   {
      eFill = EMPTY_FILL;
   }
   else if (eFill == EMPTY_FILL)
   {
      eFill = SOLID_FILL;
   }

   FillStyleProperty fillStyleProp(eFill);
   return setProperty(&fillStyleProp);
}

bool GraphicObjectImp::getFillState(bool* pSuccess) const
{
   if (pSuccess != NULL)
   {
      *pSuccess = false;
   }

   bool bFill = false;

   FillStyleProperty* pFillStyle = NULL;
   pFillStyle = (FillStyleProperty*) getProperty("FillStyle");
   if (pFillStyle != NULL)
   {
      FillStyle eFill = EMPTY_FILL;
      eFill = pFillStyle->getFillStyle();
      if (eFill != EMPTY_FILL)
      {
         bFill = true;
      }

      if (pSuccess != NULL)
      {
         *pSuccess = true;
      }
   }

   return bFill;
}

bool GraphicObjectImp::setFillColor(ColorType fillColor)
{
   FillColorProperty fillColorProp(fillColor);
   return setProperty(&fillColorProp);
}

ColorType GraphicObjectImp::getFillColor() const
{
   ColorType fillColor;

   FillColorProperty* pFillColor = NULL;
   pFillColor = (FillColorProperty*) getProperty("FillColor");
   if (pFillColor != NULL)
   {
      fillColor = pFillColor->getColor();
   }

   return fillColor;
}

bool GraphicObjectImp::setFillStyle(FillStyle eFill)
{
   if (hasProperty("FillStyle") == false)
   {
      return false;
   }

   FillStyleProperty fillStyleProp(eFill);
   return setProperty(&fillStyleProp);
}

FillStyle GraphicObjectImp::getFillStyle() const
{
   FillStyle eStyle;

   FillStyleProperty* pFillStyle = NULL;
   pFillStyle = (FillStyleProperty*) getProperty("FillStyle");
   if (pFillStyle != NULL)
   {
      eStyle = pFillStyle->getFillStyle();
   }

   return eStyle;
}

bool GraphicObjectImp::setHatchStyle(SymbolType eHatch)
{
   if (hasProperty("HatchStyle") == false)
   {
      return false;
   }

   HatchStyleProperty hatchStyleProp(eHatch);
   return setProperty(&hatchStyleProp);
}

SymbolType GraphicObjectImp::getHatchStyle() const
{
   SymbolType eHatch;

   HatchStyleProperty* pHatchStyle = NULL;
   pHatchStyle = (HatchStyleProperty*) getProperty("HatchStyle");
   if (pHatchStyle != NULL)
   {
      eHatch = pHatchStyle->getHatchStyle();
   }

   return eHatch;
}

bool GraphicObjectImp::setApex(double dApex)
{
   if (hasProperty("Apex") == false)
   {
      return false;
   }

   ApexProperty apexProp(dApex);
   return setProperty(&apexProp);
}

double GraphicObjectImp::getApex() const
{
   double dApex = -1.0;

   ApexProperty* pApex = NULL;
   pApex = (ApexProperty*) getProperty("Apex");
   if (pApex != NULL)
   {
      dApex = pApex->getApex();
   }

   return dApex;
}

bool GraphicObjectImp::setText(const string& objectText)
{
   if (hasProperty("TextString") == false)
   {
      return false;
   }

   TextStringProperty textStringProp(objectText);
   return setProperty(&textStringProp);
}

string GraphicObjectImp::getText() const
{
   TextStringProperty* pText = NULL;
   pText = (TextStringProperty*) getProperty("TextString");
   if (pText != NULL)
   {
      return pText->getString();
   }
   return "";
}

bool GraphicObjectImp::setFont(const QFont& textFont)
{
   if (hasProperty("Font") == false)
   {
      return false;
   }

   FontImp font(textFont);

   FontProperty fontProp(font);
   return setProperty(&fontProp);
}

QFont GraphicObjectImp::getFont() const
{
   QFont textFont;

   FontProperty* pFont = NULL;
   pFont = (FontProperty*) getProperty("Font");
   if (pFont != NULL)
   {
      textFont = pFont->getFont().toQFont();
   }

   return textFont;
}

bool GraphicObjectImp::setTextColor(ColorType textColor)
{
   if (hasProperty("TextColor") == false)
   {
      return false;
   }

   TextColorProperty textColorProp(textColor);
   return setProperty(&textColorProp);
}

ColorType GraphicObjectImp::getTextColor() const
{
   ColorType textColor;

   TextColorProperty* pTextColor = NULL;
   pTextColor = (TextColorProperty*) getProperty("TextColor");
   if (pTextColor != NULL)
   {
      textColor = pTextColor->getColor();
   }

   return textColor;
}

bool GraphicObjectImp::setTextAlignment(int iAlignment)
{
   if (hasProperty("TextAlignment") == false)
   {
      return false;
   }

   TextAlignmentProperty textAlignProp(iAlignment);
   return setProperty(&textAlignProp);
}

int GraphicObjectImp::getTextAlignment() const
{
   int iAlignment = Qt::AlignLeft;

   TextAlignmentProperty* pTextAlignment = NULL;
   pTextAlignment = static_cast<TextAlignmentProperty*> (getProperty("TextAlignment"));
   if (pTextAlignment != NULL)
   {
      iAlignment = pTextAlignment->getAlignment();
   }

   return iAlignment;
}

bool GraphicObjectImp::setScale(double dScale)
{
   if (hasProperty("Scale") == false)
   {
      return false;
   }

   ScaleProperty scaleProp(dScale);
   return setProperty(&scaleProp);
}

double GraphicObjectImp::getScale() const
{
   double dScale = -1.0;

   ScaleProperty* pScale = NULL;
   pScale = (ScaleProperty*) getProperty("Scale");
   if (pScale != NULL)
   {
      dScale = pScale->getScale();
   }

   return dScale;
}

bool GraphicObjectImp::setLatLon(LatLonPoint latLonPoint)
{
   if (hasProperty("LatLon") == false)
   {
      return false;
   }

   LatLonProperty prop(latLonPoint);
   return setProperty(&prop);
}

LatLonPoint GraphicObjectImp::getLatLon() const
{
   LatLonPoint latLonPoint;

   LatLonProperty* pProp = (LatLonProperty*) getProperty("LatLon");
   if (pProp != NULL)
   {
      latLonPoint = pProp->getLatLon();
   }

   return latLonPoint;
}

bool GraphicObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   pXml->addAttr("version", XmlBase::VERSION);

   if (mProperties.size() == 0)
   {
      return true;
   }

   // Name
   string objectName = getName();
   pXml->addAttr("name", objectName);

   // Type
   GraphicObjectType eObject = getGraphicObjectType();
   string objectType = StringUtilities::toXmlString(eObject);
   VERIFY(objectType.empty() == false);

   pXml->addAttr("type", objectType);

   
   // Properties
   pXml->setSingleChildInstance(true);

   vector<GraphicProperty*>::const_iterator it;
   for (it = mProperties.begin(); it != mProperties.end(); it++)
   {
      if (*it == NULL)
      {
         continue;
      }

      if (!(*it)->toXml(pXml))
      {
         pXml->setSingleChildInstance(false);
         return false;
      }
   }

   pXml->setSingleChildInstance(false);
   return true;
}

bool GraphicObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*> (pDocument);
   if (pElement != NULL)
   {
      // Check the object type
      string objectType(A(pElement->getAttribute(X("type"))));
      if (objectType.empty() == true)
      {
         return false;
      }

      GraphicObjectType eObjectType = StringUtilities::fromXmlString<GraphicObjectType>(objectType);
      GraphicObjectType eCurrentType = getGraphicObjectType();
      if (eCurrentType != eObjectType)
      {
         return false;
      }

      // Set the object name
      string objectName(A(pElement->getAttribute(X("name"))));
      if (objectName.empty() == false)
      {
         setName(objectName);
      }
   }

   // Properties
   for(DOMNode *pChild = pDocument->getFirstChild();
                pChild != NULL;
                pChild = pChild->getNextSibling())
   {
      string propertyName(A(pChild->getNodeName()));

      GraphicProperty* pProperty = NULL;
      pProperty = createProperty(propertyName);
      if (pProperty != NULL)
      {
         try
         {
            if (!pProperty->fromXml(pChild, version))
            {
               delete pProperty;
               throw XmlReader::DomParseException("Can't deserialize property", pChild);
            }
         }
         catch (XmlReader::DomParseException& exc)
         {
            delete pProperty;
            throw exc;
         }

         if (hasProperty(propertyName) == false)
         {
            addProperty(propertyName);
         }

         if (setProperty(pProperty) == true)
         {
            updateHandles();
         }

         delete pProperty;
      }
   }

   return true;
}

const string& GraphicObjectImp::getObjectType() const
{
   static string type("GraphicObjectImp");
   return type;
}

bool GraphicObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GraphicObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void GraphicObjectImp::adjustHandles(int handle, LocationType point, bool bMaintainAspect)
{
   /*
    * 654
    * 7 3
    * 012
    */

   // Update the point and/or the handles to maintain the aspect ratio
   if (bMaintainAspect == true)
   {
      double dWidth = mHandles[4].mX - mHandles[0].mX;
      double dHeight = mHandles[4].mY - mHandles[0].mY;

      double dSlope = 1.0;
      if ((dWidth != 0.0) && (dHeight != 0.0))
      {
         dSlope = dHeight / dWidth;
      }

      LocationType oldPoint = mHandles[handle];

      switch (handle)
      {
         case 0:
            dWidth = mHandles[4].mX - point.mX;
            dHeight = mHandles[4].mY - point.mY;

            if (fabs(dWidth) > fabs(dHeight / dSlope))
            {
               point.mY = mHandles[4].mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope));
            }
            else if (fabs(dWidth * dSlope) < fabs(dHeight))
            {
               point.mX = mHandles[4].mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope));
            }

            break;

         case 1:
            dHeight = mHandles[5].mY - point.mY;
            mHandles[0].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[2].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[3].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[4].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[6].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[7].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            break;

         case 2:
            dWidth = point.mX - mHandles[6].mX;
            dHeight = mHandles[6].mY - point.mY;

            if (fabs(dWidth) > fabs(dHeight / dSlope))
            {
               point.mY = mHandles[6].mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope));
            }
            else if (fabs(dWidth * dSlope) < fabs(dHeight))
            {
               point.mX = mHandles[6].mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope));
            }

            break;

         case 3:
            dWidth = point.mX - mHandles[7].mX;
            mHandles[0].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[1].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[2].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[4].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[5].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[6].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            break;

         case 4:
            dWidth = point.mX - mHandles[0].mX;
            dHeight = point.mY - mHandles[0].mY;

            if (fabs(dWidth) > fabs(dHeight / dSlope))
            {
               point.mY = mHandles[0].mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope));
            }
            else if (fabs(dWidth * dSlope) < fabs(dHeight))
            {
               point.mX = mHandles[0].mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope));
            }

            break;

         case 5:
            dHeight = point.mY - mHandles[1].mY;
            mHandles[0].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[2].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[3].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[4].mX = oldPoint.mX + (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[6].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            mHandles[7].mX = oldPoint.mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope) / 2);
            break;

         case 6:
            dWidth = mHandles[2].mX - point.mX;
            dHeight = point.mY - mHandles[2].mY;

            if (fabs(dWidth) > fabs(dHeight / dSlope))
            {
               point.mY = mHandles[2].mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope));
            }
            else if (fabs(dWidth * dSlope) < fabs(dHeight))
            {
               point.mX = mHandles[2].mX - (DrawUtil::sign(dWidth) * fabs(dHeight / dSlope));
            }

            break;

         case 7:
            dWidth = mHandles[3].mX - point.mX;
            mHandles[0].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[1].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[2].mY = oldPoint.mY - (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[4].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[5].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            mHandles[6].mY = oldPoint.mY + (DrawUtil::sign(dHeight) * fabs(dWidth * dSlope) / 2);
            break;
      }
   }

   switch (handle)
   {
      case 0:
         // fall through
      case 1:
         // fall through
      case 2:
         mHandles[0].mY = point.mY;
         mHandles[1].mY = point.mY;
         mHandles[2].mY = point.mY;
         mHandles[3].mY = (point.mY + mHandles[5].mY) / 2;
         mHandles[7].mY = (point.mY + mHandles[5].mY) / 2;
         break;
      default:
         break;
   }

   switch (handle)
   {
      case 4:
         // fall through
      case 5:
         // fall through
      case 6:
         mHandles[4].mY = point.mY;
         mHandles[5].mY = point.mY;
         mHandles[6].mY = point.mY;
         mHandles[3].mY = (point.mY + mHandles[1].mY) / 2;
         mHandles[7].mY = (point.mY + mHandles[1].mY) / 2;
         break;
      default:
         break;
   }

   switch (handle)
   {
      case 2:
         // fall through
      case 3:
         // fall through
      case 4:
         mHandles[2].mX = point.mX;
         mHandles[3].mX = point.mX;
         mHandles[4].mX = point.mX;
         mHandles[1].mX = (point.mX + mHandles[7].mX) / 2;
         mHandles[5].mX = (point.mX + mHandles[7].mX) / 2;
         break;
      default:
         break;
   }

   switch (handle)
   {
      case 6:
         // fall through
      case 7:
         // fall through
      case 0:
         mHandles[6].mX = point.mX;
         mHandles[7].mX = point.mX;
         mHandles[0].mX = point.mX;
         mHandles[1].mX = (point.mX + mHandles[3].mX) / 2;
         mHandles[5].mX = (point.mX + mHandles[3].mX) / 2;
         break;
      default:
         break;
   }
}

bool GraphicObjectImp::setImageFile(const char *pFilename)
{
   if ((getGraphicObjectType() == FILE_IMAGE_OBJECT) && (pFilename != NULL))
   {
      FileNameProperty fnProp(pFilename);
      return setProperty(&fnProp);
   }

   return false;
}

const char* GraphicObjectImp::getImageFile() const
{
   const char* pFilename = NULL;

   FileNameProperty* pProperty = NULL;
   pProperty = (FileNameProperty*) getProperty("Filename");
   if (pProperty != NULL)
   {
      const string& filename = pProperty->getFileName();
      if (filename.empty() == false)
      {
         pFilename = filename.c_str();
      }
   }

   return pFilename;
}

double GraphicObjectImp::getAlpha() const
{
   double dAlpha = -1.0;

   AlphaProperty* pAlpha = NULL;
   pAlpha = (AlphaProperty*) getProperty("Alpha");
   if (pAlpha != NULL)
   {
      dAlpha = pAlpha->getAlpha();
   }

   return dAlpha;
}

bool GraphicObjectImp::setAlpha(double alpha)
{
   if (hasProperty("Alpha") == false)
   {
      return false;
   }

   AlphaProperty alphaProp(alpha);
   return setProperty(&alphaProp);
}

bool GraphicObjectImp::addVertices(const vector<LocationType>& vertices)
{
   return false;
}

bool GraphicObjectImp::addGeoVertices(const vector<LocationType>& geoVertices)
{
   return false;
}

bool GraphicObjectImp::newPath()
{
   return false;
}

bool GraphicObjectImp::setObjectView(View* pView)
{
   if (getGraphicObjectType() == VIEW_OBJECT)
   {
      static_cast<ViewObjectImp*>(this)->setView(pView);
      return true;
   }

   return false;
}

View* GraphicObjectImp::getObjectView() const
{
   if (getGraphicObjectType() == VIEW_OBJECT)
   {
      return static_cast<const ViewObjectImp *>(this)->getView();
   }

   return NULL;
}

GraphicLayer* GraphicObjectImp::getLayer() const
{
   return const_cast<GraphicLayer*>(mpLayer.get());
}

void GraphicObjectImp::setLayer(GraphicLayer *pLayer)
{
   mpLayer.reset(pLayer);
}

bool GraphicObjectImp::canRename() const
{
   return true;
}

void GraphicObjectImp::drawLabel() const
{
   GraphicLayerImp *pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer != NULL)
   {
      if (pLayer->getShowLabels()) 
      {
         ViewImp* pView = dynamic_cast<ViewImp*>(getLayer()->getView());
         if (pView != NULL)
         {
            QColor color = pLayer->getLabelColor(this);
            pView->qglColor(color);

            LocationType location = getLabelPosition();

            double screenX = 0.0;
            double screenY = 0.0;
            pLayer->translateDataToScreen(location.mX, location.mY, screenX, screenY);

            pView->renderText(static_cast<int>(screenX), pView->height() - static_cast<int>(screenY),
               QString::fromStdString(getName()), getFont());
         }
      }
   }
}

LocationType GraphicObjectImp::getLabelPosition() const
{
   string objectName = getName();
   QFontMetrics metrics(getFont());
   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();
   LocationType retval;
   retval.mX = (llCorner.mX + urCorner.mX - metrics.width(QString::fromStdString(objectName))) / 2;
   retval.mY = (llCorner.mY + urCorner.mY - metrics.height()) / 2;

   return retval;
}

LocationType GraphicObjectImp::getPixelSize() const
{
   LocationType pixelSize(1.0, 1.0);

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      PerspectiveView* pPerspectiveView = dynamic_cast<PerspectiveView*>(pLayer->getView());
      if (pPerspectiveView != NULL)
      {
         LocationType llCorner = getLlCorner();
         LocationType urCorner = getUrCorner();

         double viewSize = pPerspectiveView->getPixelSize(llCorner, urCorner);
         pixelSize.mX = viewSize * pPerspectiveView->getPixelAspect();
         pixelSize.mY = viewSize;
      }

      OrthographicView* pOrthographicView = dynamic_cast<OrthographicView*>(pLayer->getView());
      if (pOrthographicView != NULL)
      {
         pixelSize = pOrthographicView->getPixelSize();
      }
   }

   return pixelSize;
}

bool GraphicObjectImp::processMousePress(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (button == Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);

      setBoundingBox(screenCoord, screenCoord);
      used = true;
   }
   return used;
}

bool GraphicObjectImp::processMouseMove(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (buttons & Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      setBoundingBox(getLlCorner(), screenCoord);
      used = true;
   }
   return used;
}

bool GraphicObjectImp::processMouseRelease(LocationType screenCoord, 
                                  Qt::MouseButton button,
                                  Qt::MouseButtons buttons,
                                  Qt::KeyboardModifiers modifiers)
{
   bool used = false;
   if (button == Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      setBoundingBox(getLlCorner(), screenCoord);
      GraphicLayerImp *pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayerImp != NULL)
      {
         pLayerImp->completeInsertion();
      }
      used = true;
   }
   return used;
}

bool GraphicObjectImp::processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button,
                                               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool GraphicObjectImp::edit()
{
   return false;
}

GraphicObjectType GraphicObjectImp::getGraphicObjectType() const
{
   return mType;
}

bool GraphicObjectImp::isVisible() const
{
   return true;
}

void GraphicObjectImp::setCacheDirty()
{
   mDisplayListDirty = true;
   mBitMaskDirty = true;
}

bool GraphicObjectImp::setPixelSymbol(SymbolType symbol)
{
   PixelSymbolProperty symbolProp(symbol);
   return setProperty(&symbolProp);

}

SymbolType GraphicObjectImp::getPixelSymbol() const
{
   SymbolType symbol = SOLID;

   PixelSymbolProperty* pSymbol = NULL;
   pSymbol = static_cast<PixelSymbolProperty*>(getProperty("PixelSymbol"));
   if (pSymbol != NULL)
   {
      symbol = pSymbol->getPixelSymbol();
   }

   return symbol;

}

bool GraphicObjectImp::setSymbolName(const string &symbolName)
{
   GraphicSymbolProperty symbolProp(symbolName);
   return setProperty(&symbolProp);
}

const string &GraphicObjectImp::getSymbolName() const
{
   static string empty;
   
   GraphicSymbolProperty *pSymbol = dynamic_cast<GraphicSymbolProperty*>(getProperty("GraphicSymbol"));
   if (pSymbol != NULL)
   {
      return pSymbol->getSymbolName();
   }
   
   return empty;
}

bool GraphicObjectImp::setSymbolSize(unsigned int symbolSize)
{
   GraphicSymbolSizeProperty symbolProp(symbolSize);
   return setProperty(&symbolProp);
}

unsigned int GraphicObjectImp::getSymbolSize() const
{
   GraphicSymbolSizeProperty *pSymbol = dynamic_cast<GraphicSymbolSizeProperty*>(getProperty("GraphicSymbolSize"));
   if (pSymbol != NULL)
   {
      return pSymbol->getSymbolSize();
   }
   
   return 0;
}

bool GraphicObjectImp::setDrawMode(ModeType mode)
{
   DrawModeProperty modeProp(mode);
   return setProperty(&modeProp);
}

ModeType GraphicObjectImp::getDrawMode() const
{
   ModeType mode = DRAW;

   DrawModeProperty* pMode = static_cast<DrawModeProperty*>(getProperty("DrawMode"));
   if (pMode != NULL)
   {
      mode = pMode->getDrawMode();
   }

   return mode;
}

bool GraphicObjectImp::setLineScaled(bool scaled)
{
   LineScaledProperty prop(scaled);
   return setProperty(&prop);
}

bool GraphicObjectImp::getLineScaled() const
{
   bool scaled = false;
   LineScaledProperty *pScaled = static_cast<LineScaledProperty*>(getProperty("LineScaled"));
   if (pScaled != NULL)
   {
      scaled = pScaled->getScaled();
   }
   return scaled;
}

bool GraphicObjectImp::hasCornerHandles() const
{
   return true;
}

bool GraphicObjectImp::insertionUndoable() const
{
   return true;
}

void GraphicObjectImp::subjectModified()
{
   notify(SIGNAL_NAME(Subject, Modified));
}

GraphicElement *GraphicObjectImp::getElement() const
{
   GraphicLayer *pLayer = getLayer();
   if (pLayer != NULL)
   {
      return dynamic_cast<GraphicElement*>(pLayer->getDataElement());
   }
   return NULL;
}


const RasterElement *GraphicObjectImp::getGeoreferenceElement() const
{
   GraphicElementImp *pElement = dynamic_cast<GraphicElementImp*>(getElement());
   if (pElement == NULL)
   {
      return NULL;
   }

   return pElement->getGeoreferenceElement();
}

void GraphicObjectImp::setName(const string& newName)
{
   if (newName.empty() == true)
   {
      return;
   }
   const string& oldName = getName();
   if (newName != oldName)
   {
      GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new SetGraphicObjectName(dynamic_cast<GraphicObject*>(this), oldName, newName));
         }
      }
      SessionItemImp::setName(newName);
      emit nameChanged(QString::fromStdString(newName));
      notify(SIGNAL_NAME(Subject, Modified));
   }
}

