/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "XercesIncludes.h"

#include "MultipointObjectImp.h"
#include "glCommon.h"
#include "AppVerify.h"
#include "DrawUtil.h"
#include "GraphicElement.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "GraphicLayerUndo.h"
#include "PerspectiveView.h"
#include "RasterElement.h"
#include "SessionManager.h"
#include "View.h"

#include <limits>
#include <sstream>
#include <math.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define fabsmax(x,y) ((fabs(x) > fabs(y)) ? x : y)

#undef max
#undef min

MultipointObjectImp::MultipointObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                         LocationType pixelCoord) :
   PixelObjectImp(id, type, pLayer, pixelCoord),
   mFlipX(false),
   mFlipY(false),
   mUpdating(false)
{
   addProperty("LineColor");
   addProperty("PixelSymbol");
   addProperty("GraphicSymbol");
   addProperty("GraphicSymbolSize");
   updateBoundingBox();
}

MultipointObjectImp::~MultipointObjectImp()
{
}

void MultipointObjectImp::moveHandle(int handle, LocationType pixel, bool bMaintainAspect)
{
   pixel = getLayer()->correctCoordinate(pixel);
   
   if (handle < 8)
   {
      LocationType llCorner(getLlCorner());
      LocationType urCorner(getUrCorner());
    
      LocationType fixedPoint;
      LocationType startPoint(mHandles[handle]);
      LocationType endPoint(pixel);
      switch (handle)
      {
      case TOP_LEFT:
         fixedPoint = mHandles[BOTTOM_RIGHT];
         break;
      case TOP_CENTER:
         fixedPoint = mHandles[BOTTOM_CENTER];
         break;
      case TOP_RIGHT:
         fixedPoint = mHandles[BOTTOM_LEFT];
         break;
      case MIDDLE_RIGHT:
         fixedPoint = mHandles[MIDDLE_LEFT];
         break;
      case BOTTOM_RIGHT:
         fixedPoint = mHandles[TOP_LEFT];
         break;
      case BOTTOM_CENTER:
         fixedPoint = mHandles[TOP_CENTER];
         break;
      case BOTTOM_LEFT:
         fixedPoint = mHandles[TOP_RIGHT];
         break;
      case MIDDLE_LEFT:
         fixedPoint = mHandles[MIDDLE_RIGHT];
         break;
      default:
         break;
      };

      scaleAndTranslateAllPoints(fixedPoint, startPoint, endPoint, bMaintainAspect);

      // Update the group and polyline bounding box, handles
      updateBoundingBox();
      updateHandles();
   }
   else
   {
      // Move the handle
      if (handle >= static_cast<int>(mHandles.size()))
      {
         return;
      }

      LocationType oldPoint = mHandles[handle];
      mHandles[handle] = pixel;

      unsigned int vertexNum = handle - 8;
      if (vertexNum >= mVertices.size())
      {
         return;
      }
      mVertices[vertexNum] = pixel;

      const RasterElement* pGeo = getGeoreferenceElement();
      if (pGeo != NULL)
      {
         if (mGeoVertices.empty())
         {
            enableGeo();
         }
         else
         {
            VERIFYNRV(mVertices.size() == mGeoVertices.size());
            mGeoVertices[vertexNum] = pGeo->convertPixelToGeocoord(pixel);
         }
      }

      updateBoundingBox();
      updateHandles();
   }

   emit modified();
}

void MultipointObjectImp::updateHandles()
{
   // Add the object handles
   GraphicObjectImp::updateHandles();

   // Add the vertex
   mHandles.insert(mHandles.end(), mVertices.begin(), mVertices.end());
}

void MultipointObjectImp::updateGeo()
{
   VERIFYNRV(mVertices.size() == mGeoVertices.size());

   const RasterElement* pGeo = getGeoreferenceElement();
   VERIFYNRV(pGeo != NULL);
   mVertices = pGeo->convertGeocoordsToPixels(mGeoVertices);

   updateHandles();
   updateBoundingBox();
   emit modified();
}

void MultipointObjectImp::enableGeo()
{
   const RasterElement* pGeo = getGeoreferenceElement();
   VERIFYNRV(pGeo != NULL);
   mGeoVertices = pGeo->convertPixelsToGeocoords(mVertices);
}

void MultipointObjectImp::drawVector(double zoomFactor) const
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   if (pLayer != NULL && pLayer->mayDrawAsPixels())
   {
      ColorType color = getLineColor();
      glColor3ub(color.mRed, color.mGreen, color.mBlue);
      glPointSize(1);

      glBegin(GL_POINTS);
      for (unsigned int i = 0; i < mVertices.size(); ++i)
      {
         LocationType point = mVertices.at(i);
         glVertex2f(point.mX+0.5, point.mY+0.5);
      }
      glEnd();

   }
   else
   {
      unsigned int symbolSize = getSymbolSize();
      const string& symbolName = getSymbolName();
      double objectRotation = getRotation();

      pLayer->drawSymbols(symbolName, mVertices, symbolSize, objectRotation);
   }
}

void MultipointObjectImp::drawPixels(double zoomFactor) const
{
   ColorType color = getLineColor();
   glColor3ub(color.mRed, color.mGreen, color.mBlue);
   DrawUtil::PixelDrawer drawer(getPixelSymbol());
   for (unsigned int i = 0; i < mVertices.size(); ++i)
   {
      drawer(floor(mVertices[i].mX), floor(mVertices[i].mY));
   }
}

bool MultipointObjectImp::addVertex(LocationType endPoint)
{
   vector<LocationType> oldVertices = mVertices;
   vector<LocationType> oldGeoVertices = mGeoVertices;

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   DrawUtil::updateBoundingBox(llCorner, urCorner, endPoint);

   mVertices.push_back(endPoint);

   mBoxMatchesVertices = false;
   setBoundingBox(llCorner, urCorner);
   mBoxMatchesVertices = true;

   const RasterElement* pGeo = getGeoreferenceElement();
   if (pGeo != NULL)
   {
      mGeoVertices.push_back(pGeo->convertPixelToGeocoord(endPoint));
   }

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new AddVertices(this, oldVertices, oldGeoVertices, mVertices, mGeoVertices));
      }
   }

   updateHandles();
   emit modified();
   return true;
}

bool MultipointObjectImp::addVertices(const vector<LocationType> &vertices)
{
   vector<LocationType> geoVertices;
   const RasterElement* pGeo = getGeoreferenceElement();
   if (pGeo != NULL)
   {
      geoVertices = pGeo->convertPixelsToGeocoords(vertices);
   }

   return addVertices(vertices, geoVertices);
}

bool MultipointObjectImp::addGeoVertices(const vector<LocationType> &geoVertices)
{
   const RasterElement* pGeo = getGeoreferenceElement();
   if (pGeo == NULL)
   {
      return false;
   }

   vector<LocationType> vertices = pGeo->convertGeocoordsToPixels(geoVertices);
   return addVertices(vertices, geoVertices);
}

bool MultipointObjectImp::addVertices(const vector<LocationType> &vertices, const vector<LocationType> &geoVertices)
{
   if ((vertices == mVertices) && (geoVertices == mGeoVertices))
   {
      return false;
   }

   LocationType llCorner(numeric_limits<double>::max(), numeric_limits<double>::max());
   LocationType urCorner(-1.0*numeric_limits<double>::max(), -1.0*numeric_limits<double>::max());
   LocationType currentVertex;

   vector<LocationType> oldVertices = mVertices;
   vector<LocationType> oldGeoVertices = mGeoVertices;

   if (mVertices.empty() == false)
   {
      llCorner = getLlCorner();
      urCorner = getUrCorner();
   }
   for (unsigned int i = 0; i < vertices.size(); ++i)
   {
      currentVertex = vertices.at(i);

      DrawUtil::updateBoundingBox(llCorner, urCorner, currentVertex);
      mVertices.push_back(vertices.at(i));
   }

   copy(geoVertices.begin(), geoVertices.end(), back_inserter(mGeoVertices));

   mBoxMatchesVertices = false;
   setBoundingBox(llCorner, urCorner);
   mBoxMatchesVertices = true;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      View* pView = pLayer->getView();
      if (pView != NULL)
      {
         pView->addUndoAction(new AddVertices(this, oldVertices, oldGeoVertices, mVertices, mGeoVertices));
      }
   }

   updateHandles();
   emit modified();
   return true;
}

void MultipointObjectImp::clearVertices()
{
   mVertices.clear();
   mGeoVertices.clear();

   updateBoundingBox();
   updateHandles();
   emit modified();
}

const vector<LocationType> &MultipointObjectImp::getVertices() const
{
   return mVertices;
}

bool MultipointObjectImp::replicateObject(const GraphicObject* pObject)
{
   if (pObject == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::replicateObject(pObject);
   if (bSuccess == false)
   {
      return false;
   }

   if (pObject->getGraphicObjectType() == getGraphicObjectType())
   {
      vector<LocationType> oldVertices = mVertices;
      vector<LocationType> oldGeoVertices = mGeoVertices;

      const MultipointObjectImp* pMPoint = dynamic_cast<const MultipointObjectImp*>(pObject);
      mVertices = pMPoint->mVertices;
      mGeoVertices = pMPoint->mGeoVertices;

      if (getGeoreferenceElement() != NULL)
      {
         if (mGeoVertices.empty())
         {
            enableGeo();
         }
         else
         {
            updateGeo();
         }
      }

      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         View* pView = pLayer->getView();
         if (pView != NULL)
         {
            pView->addUndoAction(new AddVertices(this, oldVertices, oldGeoVertices, mVertices, mGeoVertices));
         }
      }
   }

   return bSuccess;
}

void MultipointObjectImp::updateBoundingBox()
{
   LocationType llCorner(numeric_limits<double>::max(), numeric_limits<double>::max());
   LocationType urCorner(-1.0*numeric_limits<double>::max(), -1.0*numeric_limits<double>::max());

   for (unsigned int i = 0; i < mVertices.size(); ++i)
   {
      DrawUtil::updateBoundingBox(llCorner, urCorner, mVertices.at(i));
   }

   if (mFlipX)
   {
      swap(llCorner.mX, urCorner.mX);
   }
   if (mFlipY)
   {
      swap(llCorner.mY, urCorner.mY);
   }

   mBoxMatchesVertices = false;
   setBoundingBox(llCorner, urCorner);
   mBoxMatchesVertices = true;
}

void MultipointObjectImp::scaleAndTranslateAllPoints(LocationType fixedPoint, LocationType startPoint,
                           LocationType endPoint, bool bMaintainAspect, LocationType translateFactor)
{
   double dScaleX(1.0);
   double dScaleY(1.0);

   if ( (fabs(endPoint.mX - startPoint.mX) > HANDLE_DELTA_THRESHOLD) && 
        (fabs(startPoint.mX - fixedPoint.mX) > HANDLE_DELTA_THRESHOLD) )
   {
      dScaleX = (endPoint.mX - fixedPoint.mX) / (startPoint.mX - fixedPoint.mX);
      if (dScaleX < 0)
      {
         mFlipX ^= true;
      }
   }
   if ( (fabs(endPoint.mY - startPoint.mY) > HANDLE_DELTA_THRESHOLD) && 
        (fabs(startPoint.mY - fixedPoint.mY) > HANDLE_DELTA_THRESHOLD) )
   {
      dScaleY = (endPoint.mY - fixedPoint.mY) / (startPoint.mY - fixedPoint.mY);
      if (dScaleY < 0)
      {
         mFlipY ^= true;
      }
   }

   if (bMaintainAspect == true)
   {
      dScaleX = fabsmax(dScaleX, dScaleY);
      dScaleY = fabsmax(dScaleX, dScaleY);
   }

   LocationType fullTranslate = fixedPoint + translateFactor;
   for (unsigned int i = 0; i < mVertices.size(); ++i)
   {
      LocationType vertex = mVertices.at(i) - fixedPoint;
      vertex.mX *= dScaleX;
      vertex.mY *= dScaleY;
      mVertices.at(i) = vertex + fullTranslate;
   }

   const RasterElement* pGeo = getGeoreferenceElement();
   if (pGeo != NULL)
   {
      mGeoVertices = pGeo->convertPixelsToGeocoords(mVertices);
   }

   updateHandles();
}

bool MultipointObjectImp::setProperty(const GraphicProperty* pProperty)
{
   if (pProperty == NULL)
   {
      return false;
   }

   // if the current box matches the vertices, then the new box
   // will necessitate an update to the vertices
   if (mUpdating == false && mBoxMatchesVertices == true && pProperty->getName() == "BoundingBox")
   {
      LocationType oldLlCorner = getLlCorner();
      LocationType oldUrCorner = getUrCorner();

      const BoundingBoxProperty* pBoundProp = static_cast<const BoundingBoxProperty*>(pProperty);
      LocationType newLlCorner = pBoundProp->getLlCorner();
      LocationType newUrCorner = pBoundProp->getUrCorner();

      LocationType relUrCorner = newUrCorner - newLlCorner + oldLlCorner;
      LocationType delta = newLlCorner - oldLlCorner;
      scaleAndTranslateAllPoints(oldLlCorner, oldUrCorner, relUrCorner, false, delta);
   }

   mUpdating = true;
   bool success = GraphicObjectImp::setProperty(pProperty);
   mUpdating = false;
   return success;
}

bool MultipointObjectImp::hit(LocationType pixelCoord) const
{
   // Define the hit tolerance in scene pixels as being within the vertex pixel
   double hitTolerance = 0.5;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      PerspectiveView* pView = dynamic_cast<PerspectiveView*>(pLayer->getView());
      if (pView != NULL)
      {
         // If the zoom level is smaller than a three pixel screen tolerance,
         // use the screen tolerance equivalent instead
         const double screenTolerance = 3.0;

         double zoomFactor = pView->getZoomPercentage() / 100.0;
         if (zoomFactor < screenTolerance)
         {
            double xScale = 1.0 / zoomFactor / pLayer->getXScaleFactor();
            double yScale = 1.0 / zoomFactor / pLayer->getYScaleFactor();

            hitTolerance = min(screenTolerance * xScale, screenTolerance * yScale);
         }
      }
   }

   for (unsigned int i = 0; i < mVertices.size(); ++i)
   {
      if ((fabs(mVertices.at(i).mX - pixelCoord.mX) < hitTolerance)
         && (fabs(mVertices.at(i).mY - pixelCoord.mY) < hitTolerance))
      {
         return true;
      }
   }

   return false;
}

bool MultipointObjectImp::getExtents(vector<LocationType>& dataCoords) const
{
   if (mVertices.empty() == true)
   {
      return false;
   }

   dataCoords = mVertices;
   return true;
}

bool MultipointObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL)
   {
      return false;
   }

   bool bSuccess = GraphicObjectImp::toXml(pXml);
   if (bSuccess == true)
   {
      bSuccess = false;
      if (!mGeoVertices.empty())
      {
         DOMElement* pAllGeoElement = pXml->addElement("geoVertices");
         if (pAllGeoElement != NULL)
         {
            pXml->pushAddPoint(pAllGeoElement);

            stringstream buf;
            buf.precision(12);
            for (unsigned int i = 0; i < mGeoVertices.size(); ++i)
            {
               buf.str("");

               DOMElement* pLatLong = pXml->addElement("latlong", pAllGeoElement);
               if (pLatLong != NULL)
               {
                  buf << mGeoVertices[i].mX << ' ' << mGeoVertices[i].mY;
                  pXml->addText(buf.str().c_str(), pLatLong);
                  pAllGeoElement->appendChild(pLatLong);
               }
            }

            pXml->popAddPoint();
            bSuccess = true;
         }
         else
         {
            return false;
         }
      }

      if (mVertices.empty() == false)
      {
         DOMElement* pAllVertElement = pXml->addElement("vertices");
         if (pAllVertElement != NULL)
         {
            pXml->pushAddPoint(pAllVertElement);

            stringstream buf;
            buf.precision(12);
            for (unsigned int i = 0; i < mVertices.size(); ++i)
            {
               buf.str("");

               DOMElement* pPixel = pXml->addElement("pixel", pAllVertElement);
               if (pPixel != NULL)
               {
                  buf << mVertices[i].mX << ' ' << mVertices[i].mY;
                  pXml->addText(buf.str().c_str(), pPixel);
                  pAllVertElement->appendChild(pPixel);
               }
            }

            pXml->popAddPoint();
            bSuccess = true;
         }
         else
         {
            bSuccess = false;
         }
      }
   }

   return bSuccess;
}

bool MultipointObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL)
   {
      return false;
   }

   if (GraphicObjectImp::fromXml(pDocument, version) == false)
   {
      return false;
   }

   vector<LocationType> vertices;
   vector<LocationType> geoVertices;
   DOMNode* pObjectNode = pDocument->getFirstChild();
   while (pObjectNode != NULL)
   {
      if (XMLString::equals(pObjectNode->getNodeName(), X("vertices")))
      {
         getVertices(pObjectNode->getFirstChild(), "pixel", vertices);
      }
      else if (XMLString::equals(pObjectNode->getNodeName(), X("geoVertices")))
      {
         getVertices(pObjectNode->getFirstChild(), "latlong", geoVertices);
      }

      pObjectNode = pObjectNode->getNextSibling();
   }

   if (Service<SessionManager>()->isSessionLoading())
   {
      mVertices = vertices;
      mGeoVertices = geoVertices;
      return true;
   }

   // Use geoVertices if they are available and this object is georeferenced, otherwise use vertices.
   if (getGeoreferenceElement() != NULL && geoVertices.empty() == false)
   {
      return addGeoVertices(geoVertices);
   }
   else if (vertices.empty() == false)
   {
      return addVertices(vertices);
   }

   return false;
}

void MultipointObjectImp::getVertices(const DOMNode* pVertex, const string& nodeName, vector<LocationType>& vertices)
{
   while (pVertex != NULL)
   {
      if (XMLString::equals(pVertex->getNodeName(), X(nodeName.c_str())))
      {
         DOMNode* pValue = pVertex->getFirstChild();
         if (VERIFYNR(pValue != NULL))
         {
            LocationType pixel;
            if (XmlReader::StrToLocation(pValue->getNodeValue(), pixel) == true)
            {
               vertices.push_back(pixel);
            }
         }
      }

      pVertex = pVertex->getNextSibling();
   }
}

const string& MultipointObjectImp::getObjectType() const
{
   static string type("MultipointObjectImp");
   return type;
}

bool MultipointObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "MultipointObject"))
   {
      return true;
   }

   return PixelObjectImp::isKindOf(className);
}

const BitMask* MultipointObjectImp::getPixels(int iStartColumn, int iStartRow, int iEndColumn, int iEndRow)
{
   double dAngle = getRotation();
   if (dAngle != 0.0)
   {
      return GraphicObjectImp::getPixels(iStartColumn, iStartRow, iEndColumn, iEndRow);
   }

   mPixelMask.clear();
   mPixelMask.setPixel(iStartColumn, iStartRow, true);
   mPixelMask.setPixel(iEndColumn, iEndRow, true);
   mPixelMask.setPixel(iStartColumn, iStartRow, false);
   mPixelMask.setPixel(iEndColumn, iEndRow, false);

   for (unsigned int i = 0; i < mVertices.size(); ++i)
   {
      LocationType vertex = mVertices[i];
      if ((vertex.mX >= iStartColumn) && (vertex.mX <= iEndColumn) &&
         (vertex.mY >= iStartRow) && (vertex.mY <= iEndRow))
      {
         mPixelMask.setPixel(vertex.mX, vertex.mY, true);
      }
   }

   return &mPixelMask;
}

bool MultipointObjectImp::processMousePress(LocationType screenCoord, Qt::MouseButton button,
                                            Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      addVertex(screenCoord);
      emit modified();
      return true;
   }

   return false;
}

bool MultipointObjectImp::processMouseMove(LocationType screenCoord, Qt::MouseButton button,
                                           Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if ((buttons & Qt::LeftButton) && mHandles.size() > 8)
   {
      screenCoord = getLayer()->correctCoordinate(screenCoord);
      moveHandle(mHandles.size()-1, screenCoord);
      emit modified();
      return true;
   }

   return false;
}

bool MultipointObjectImp::processMouseRelease(LocationType screenCoord, Qt::MouseButton button,
                                              Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   return false;
}

bool MultipointObjectImp::processMouseDoubleClick(LocationType screenCoord, Qt::MouseButton button,
                                                  Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   if (button == Qt::LeftButton)
   {
      GraphicLayerImp* pLayerImp = dynamic_cast<GraphicLayerImp*>(getLayer());
      if (pLayerImp != NULL)
      {
         pLayerImp->completeInsertion();
      }

      return true;
   }

   return false;
}

void MultipointObjectImp::removeVertex(unsigned int index)
{
   unsigned int numVertices = static_cast<unsigned int>(mVertices.size());
   if (index >= numVertices)
   {
      index = numVertices - 1;
   }

   mVertices.erase(mVertices.begin() + index);
   if (index < mGeoVertices.size())
   {
      mGeoVertices.erase(mGeoVertices.begin() + index);
   }

   updateBoundingBox();
   updateHandles();
   emit modified();
}
