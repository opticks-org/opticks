/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "FileResource.h"
#include "GcpList.h"
#include "GeoAlgorithms.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "ImageHandler.h"
#include "Kml.h"
#include "KMLServer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MultipointObject.h"
#include "PerspectiveView.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Undo.h"
#include "xmlreader.h"
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QPointF>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <QtCore/QUrl>
#include <QtGui/QMatrix>
#include <zip.h>

using namespace std;

namespace
{
   string sKmlNamespace = "http://earth.google.com/kml/2.1";
};

Kml::Kml(bool exportImages, Progress* pProgress) :
   mXml("kml", sKmlNamespace, Service<MessageLogMgr>()->getLog(), true),
   mExportImages(exportImages),
   mpProgress(pProgress)
{
}

Kml::~Kml()
{
}

QString Kml::toString()
{
   return QString::fromStdString(mXml.writeToString());
}

bool Kml::toFile(const std::string &filename)
{
   if (mExportImages) // output file is a kmz
   {
      zipFile pZip = zipOpen(filename.c_str(), APPEND_STATUS_CREATE);
      if (pZip == NULL)
      {
         return false;
      }
      zip_fileinfo nfo;
      nfo.dosDate = 0;
      nfo.external_fa = 0;
      nfo.internal_fa = 0;
      QDateTime now = QDateTime::currentDateTime();
      nfo.tmz_date.tm_year = now.date().year();
      nfo.tmz_date.tm_mon = now.date().month();
      nfo.tmz_date.tm_mday = now.date().day();
      nfo.tmz_date.tm_hour = now.time().hour();
      nfo.tmz_date.tm_min = now.time().minute();
      nfo.tmz_date.tm_sec = now.time().second();

      zipOpenNewFileInZip(pZip, "doc.kml", &nfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
      QString data = toString();
      zipWriteInFileInZip(pZip, data.toAscii(), data.size());
      zipCloseFileInZip(pZip);

      const QMap<QString, QByteArray> images = getImages();
      for (QMap<QString, QByteArray>::const_iterator image = images.begin(); image != images.end(); ++image)
      {
         zipOpenNewFileInZip(pZip, image.key().toAscii(), &nfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED,
            Z_DEFAULT_COMPRESSION);
         zipWriteInFileInZip(pZip, image.value().data(), image.value().size());
         zipCloseFileInZip(pZip);
      }

      zipClose(pZip, "KMZ File Created With " APP_NAME);
   }
   else
   {
      FileResource pFile(filename.c_str(), "wt");
      if (pFile == NULL)
      {
         return false;
      }
      mXml.writeToFile(pFile);
   }
   return true;
}

bool Kml::addSession()
{
   // return the top level folder
   vector<Window*> windows;
   mXml.pushAddPoint(mXml.addElement("Folder"));
   mXml.addText("Active Data Sets", mXml.addElement("name"));
   Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
   bool success = false;
   for (vector<Window*>::const_iterator window = windows.begin(); window != windows.end(); ++window)
   {
      if (addWindow(static_cast<SpatialDataWindow*>(*window)))
      {
         success = true;
      }
   }
   mXml.popAddPoint(); // Folder

   return success;
}

bool Kml::addWindow(const SpatialDataWindow* pWindow)
{
   if (pWindow == NULL)
   {
      return false;
   }
   return addView(pWindow->getSpatialDataView());
}

bool Kml::addView(const SpatialDataView* pView)
{
   if (pView == NULL)
   {
      return false;
   }
   // ensure we are georeferenced
   RasterElement* pElement = static_cast<RasterElement*>(pView->getLayerList()->getPrimaryRasterElement());
   VERIFY(pElement);
   if (!pElement->isGeoreferenced())
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("The " + pView->getDisplayName(true) +
            " view does not have georeference information and will not be exported.", 0, ERRORS);
      }
      return false;
   }
   Layer* pPrimaryLayer = pView->getLayerList()->getLayer(RASTER, pElement);
   mXml.pushAddPoint(mXml.addElement("Folder"));
   string name = pView->getDisplayName();
   if (name.empty())
   {
      name = pView->getName();
   }
   mXml.addText(name, mXml.addElement("name"));
   mXml.addText(pView->getDisplayText(), mXml.addElement("description"));

   bool success = false;
   vector<Layer*> layers;
   pView->getLayerList()->getLayers(layers);
   int totalLayers = static_cast<int>(pView->getLayerList()->getNumLayers());
   for (vector<Layer*>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
   {
      if (!pView->isLayerDisplayed(*layer))
      {
         continue;
      }
      if (addLayer(*layer, pPrimaryLayer, pView, totalLayers))
      {
         success = true;
      }
   }
   mXml.popAddPoint(); // Folder

   return success;
}

bool Kml::addLayer(Layer* pLayer, const Layer* pGeoLayer, const SpatialDataView* pView, int totalLayers)
{
   if (pLayer == NULL)
   {
      return false;
   }
   switch (pLayer->getLayerType())
   {
   case ANNOTATION:
   case AOI_LAYER:
   case GRAPHIC_LAYER:
      // These are polygonal layers
      generatePolygonalLayer(dynamic_cast<GraphicLayer*>(pLayer),
         pView->isLayerDisplayed(pLayer), totalLayers - pView->getLayerDisplayIndex(pLayer), pGeoLayer);
      break;
   case RASTER:
      {
         RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(pLayer);
         RasterElement* pRasterElement = pRasterLayer->getDisplayedRasterElement(GRAY);
         if (pRasterElement != NULL)
         {
            bool layerIsDisplayed = pView->isLayerDisplayed(pLayer);
            int order = totalLayers - pView->getLayerDisplayIndex(pLayer) - 1;
            if (pView->getAnimationController() == NULL)
            {
               generateGroundOverlayLayer(pLayer, layerIsDisplayed, order, pGeoLayer, -1);
            }
            else
            {
               mXml.pushAddPoint(mXml.addElement("Folder"));
               mXml.addText("Frame Data", mXml.addElement("name"));
               vector<DimensionDescriptor> frames =
                  static_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor())->getBands();
               for (vector<DimensionDescriptor>::const_iterator frame = frames.begin(); frame != frames.end(); ++frame)
               {
                  generateGroundOverlayLayer(pLayer, layerIsDisplayed, order, pGeoLayer, frame->getActiveNumber());
               }
               mXml.popAddPoint();
            }
            break;
         }
         // fall through...if there is no displayed raster element, export the flattened layer
      }
   case PSEUDOCOLOR:
   case THRESHOLD:
      // These are image layers
      generateGroundOverlayLayer(pLayer, pView->isLayerDisplayed(pLayer),
         totalLayers - pView->getLayerDisplayIndex(pLayer), pGeoLayer);
      break;
   case CONTOUR_MAP:
   case LAT_LONG:
   case TIEPOINT_LAYER:
   default:
      // These are unsupported layers
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Unable to export unsupported layer " + pLayer->getName(), 0, WARNING);
      }
      return false;
   }
   return true;
}

void Kml::generateBoundingBox(const Layer* pGeoLayer, int bbox[4])
{
   VERIFYNRV(pGeoLayer != NULL);
   double dbbox[4];
   pGeoLayer->translateScreenToData(bbox[0], bbox[1], dbbox[0], dbbox[1]);
   pGeoLayer->translateScreenToData(bbox[2], bbox[3], dbbox[2], dbbox[3]);
   if (dbbox[0] > dbbox[2])
   {
      std::swap(dbbox[0], dbbox[2]);
   }
   if (dbbox[1] > dbbox[3])
   {
      std::swap(dbbox[1], dbbox[3]);
   }

   RasterElement* pGeoElement = dynamic_cast<RasterElement*>(pGeoLayer->getDataElement());
   VERIFYNRV(pGeoElement != NULL);
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pGeoElement->getDataDescriptor());
   unsigned int elementWidth = pDesc->getColumnCount();
   unsigned int elementHeight = pDesc->getRowCount();
   mXml.pushAddPoint(mXml.addElement("LatLonAltBox"));
   vector<LocationType> corners;
   corners.push_back(LocationType(dbbox[0], dbbox[1]));
   if (bbox[2] == bbox[0] && bbox[3] == bbox[1])
   {
      corners.push_back(LocationType(elementWidth, elementHeight));
   }
   else
   {
      corners.push_back(LocationType(dbbox[2], dbbox[3]));
   }
   vector<LocationType> geoCorners = pGeoElement->convertPixelsToGeocoords(corners);
   mXml.addText(geoCorners[0].mX, mXml.addElement("north"));
   mXml.addText(geoCorners[1].mX, mXml.addElement("south"));
   if (geoCorners[0].mY > geoCorners[1].mY)
   {
      mXml.addText(geoCorners[0].mY, mXml.addElement("east"));
      mXml.addText(geoCorners[1].mY, mXml.addElement("west"));
   }
   else
   {
      mXml.addText(geoCorners[0].mY, mXml.addElement("west"));
      mXml.addText(geoCorners[1].mY, mXml.addElement("east"));
   }
   mXml.popAddPoint(); // LatLonBox
}

void Kml::generatePolygonalLayer(const GraphicLayer* pGraphicLayer, bool visible, int order, const Layer* pGeoLayer)
{
   VERIFYNRV(pGeoLayer != NULL);
   if (pGraphicLayer == NULL)
   {
      return;
   }
   RasterElement* pGeoElement = dynamic_cast<RasterElement*>(pGeoLayer->getDataElement());
   mXml.pushAddPoint(mXml.addElement("Folder"));
   string name = pGraphicLayer->getDisplayName();
   if (name.empty())
   {
      name = pGraphicLayer->getName();
   }
   mXml.addText(name, mXml.addElement("name"));
   mXml.addText(pGraphicLayer->getDisplayText(), mXml.addElement("description"));
   mXml.addText(visible ? "1" : "0", mXml.addElement("visibility"));
   list<GraphicObject*> objects;
   pGraphicLayer->getObjects(objects);
   for (list<GraphicObject*>::const_iterator object = objects.begin(); object != objects.end(); ++object)
   {
      GraphicObject* pObject = *object;
      if (pObject == NULL)
      {
         continue;
      }
      mXml.pushAddPoint(mXml.addElement("Placemark"));
      mXml.addText(pObject->getName(), mXml.addElement("name"));
      mXml.addText(pObject->isVisible() ? "1" : "0", mXml.addElement("visibility"));
      mXml.pushAddPoint(mXml.addElement("Style"));
      mXml.pushAddPoint(mXml.addElement("PolyStyle"));
      ColorType c = pObject->getFillColor();
      QChar fc('0');
      mXml.addText(QString("%1%2%3%4").arg(c.mAlpha, 2, 16, fc).arg(c.mBlue, 2, 16, fc).arg(
         c.mGreen, 2, 16, fc).arg(c.mRed, 2, 16, fc).toAscii().data(),
         mXml.addElement("color"));
      mXml.addText(pObject->getLineState() ? "1" : "0", mXml.addElement("outline"));
      mXml.addText(pObject->getFillState() ? "1" : "0", mXml.addElement("fill"));
      mXml.popAddPoint(); // PolyStyle
      mXml.pushAddPoint(mXml.addElement("LineStyle"));
      c = pObject->getLineColor();
      mXml.addText(QString("%1%2%3%4").arg(c.mAlpha, 2, 16, fc).arg(c.mBlue, 2, 16, fc).arg(
         c.mGreen, 2, 16, fc).arg(c.mRed, 2, 16, fc).toAscii().data(),
         mXml.addElement("color"));
      mXml.addText(StringUtilities::toXmlString(pObject->getLineWidth()), mXml.addElement("width"));
      mXml.popAddPoint(); // LineStyle
      mXml.popAddPoint(); // Style
      switch (pObject->getGraphicObjectType())
      {
      case MULTIPOINT_OBJECT:
         {
            mXml.pushAddPoint(mXml.addElement("MultiGeometry"));
            vector<LocationType> vertices = static_cast<MultipointObject*>(pObject)->getVertices();
            for (vector<LocationType>::const_iterator vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
            {
               mXml.pushAddPoint(mXml.addElement("Point"));
               mXml.addText(QString::number(order).toAscii().data(), mXml.addElement("drawOrder"));
               LocationType geo = pGeoElement->convertPixelToGeocoord(*vertex);
               mXml.addText(QString("%1,%2").arg(geo.mY, 0, 'f', 10).arg(geo.mX, 0, 'f', 10).toAscii().data(),
                  mXml.addElement("coordinates"));
               mXml.popAddPoint(); // Point
            }
            mXml.popAddPoint();
            break;
         }
      case LINE_OBJECT:
      case POLYLINE_OBJECT:
      case HLINE_OBJECT:
      case VLINE_OBJECT:
      case ROW_OBJECT:
      case COLUMN_OBJECT:
         {
            mXml.pushAddPoint(mXml.addElement("LineString"));
            mXml.addText(QString::number(order).toAscii().data(), mXml.addElement("drawOrder"));
            mXml.addText("1", mXml.addElement("tessellate"));
            QStringList coords;
            vector<LocationType> vertices;
            if (pObject->getGraphicObjectType() == POLYLINE_OBJECT)
            {
               vertices = static_cast<PolylineObject*>(pObject)->getVertices();
            }
            else
            {
               LocationType ll = pObject->getLlCorner();
               LocationType ur = pObject->getUrCorner();
               vertices.push_back(ll);
               vertices.push_back(ur);
            }
            for (vector<LocationType>::const_iterator vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
            {
               LocationType geo = pGeoElement->convertPixelToGeocoord(*vertex);
               coords << QString("%1,%2").arg(geo.mY, 0, 'f', 10).arg(geo.mX, 0, 'f', 10);
            }
            mXml.addText(coords.join(" ").toAscii().data(), mXml.addElement("coordinates"));
            mXml.popAddPoint(); // LineString
            break;
         }
      case RECTANGLE_OBJECT:
      case TRIANGLE_OBJECT:
      case POLYGON_OBJECT:
         {
            mXml.pushAddPoint(mXml.addElement("Polygon"));
            mXml.addText(QString::number(order).toAscii().data(), mXml.addElement("drawOrder"));
            mXml.addText("1", mXml.addElement("tessellate"));
            mXml.pushAddPoint(mXml.addElement("LinearRing", mXml.addElement("outerBoundaryIs")));
            QStringList coords;
            vector<LocationType> vertices;
            LocationType ll = pObject->getLlCorner();
            LocationType ur = pObject->getUrCorner();
            LocationType center = (ll + ur);
            center.mX /= 2.0;
            center.mY /= 2.0;
            if (pObject->getGraphicObjectType() == RECTANGLE_OBJECT)
            {
               vertices.push_back(ll);
               vertices.push_back(LocationType(ll.mX, ur.mY));
               vertices.push_back(ur);
               vertices.push_back(LocationType(ur.mX, ll.mY));
            }
            else if (pObject->getGraphicObjectType() == TRIANGLE_OBJECT)
            {
               double apex = pObject->getApex();
               vertices.push_back(ll);
               vertices.push_back(LocationType(ur.mX, ll.mY));
               vertices.push_back(LocationType(ll.mX + apex * (ur.mX - ll.mX), ur.mY));
               vertices.push_back(ll);
            }
            else
            {
               vertices = static_cast<PolygonObject*>(pObject)->getVertices();
            }
            QPointF qcenter(center.mX, center.mY);
            QMatrix matrix;
            matrix.rotate(pObject->getRotation());
            for (vector<LocationType>::const_iterator vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
            {
               QPointF point(vertex->mX - center.mX, vertex->mY - center.mY);
               point = point * matrix;
               point += qcenter;
               LocationType geo = pGeoElement->convertPixelToGeocoord(LocationType(point.x(), point.y()));
               coords << QString("%1,%2").arg(geo.mY, 0, 'f', 10).arg(geo.mX, 0, 'f', 10);
            }
            mXml.addText(coords.join(" ").toAscii().data(), mXml.addElement("coordinates"));
            mXml.popAddPoint(); // LinearRing
            mXml.popAddPoint(); // Polygon
            break;
         }
      default: // not supported
         break;
      }
      mXml.popAddPoint(); // Placemark
   }
   mXml.popAddPoint(); // Folder
}

void Kml::generateGroundOverlayLayer(Layer* pLayer, bool visible, int order, const Layer* pGeoLayer, int frame)
{
   VERIFYNRV(pGeoLayer != NULL && pLayer != NULL);
   PerspectiveView* pView = dynamic_cast<PerspectiveView*>(pLayer->getView());
   VERIFYNRV(pView != NULL);

   // block adding current actions to the view's undo buffer
   UndoLock lock(pView);
   double originalAngle = pView->getRotation();

   // get rotation for north up
   double angle(0.0);
   RasterElement* pRaster = dynamic_cast<RasterElement*>(pGeoLayer->getDataElement());  // get the primary raster element
   VERIFYNRV(GeoAlgorithms::getAngleToNorth(pRaster, pView, angle) != false);
   pView->rotateTo(angle);

   // since north up only rotates, check for left-right reversed
   LocationType ll, ul, ur, lr;
   pView->getVisibleCorners(ll, ul, ur, lr);
   bool imageFlipped(ll.mX > ur.mX);
   if (imageFlipped)
   {
      pView->flipHorizontal();
   }
   pView->refresh();

   XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* pGroundOverlay = mXml.addElement("GroundOverlay");
   mXml.pushAddPoint(pGroundOverlay);
   string name = pLayer->getDisplayName();
   if (name.empty())
   {
      name = pLayer->getName();
   }
   if (frame >= 0)
   {
      name += " frame " + StringUtilities::toDisplayString(frame+1);
   }
   mXml.addText(name, mXml.addElement("name"));
   mXml.addText(pLayer->getDisplayText(), mXml.addElement("description"));
   mXml.addText(visible ? "1" : "0", mXml.addElement("visibility"));
   mXml.addText(QString::number(order).toAscii().data(), mXml.addElement("drawOrder"));
   QString layerId = QString::fromStdString(pLayer->getId());
   int bbox[4] = {0, 0, 0, 0};
   if (mExportImages)
   {
      QByteArray bytes(5 * 1024 * 1024, '\0');
      QBuffer buffer(&bytes);
      QString fileName = layerId;
      if (frame >= 0)
      {
         fileName += QString("/frame%1").arg(frame);
         mXml.pushAddPoint(mXml.addElement("TimeStamp"));
         QDateTime dt = QDateTime::currentDateTime();
         dt.setTime(QTime(0, 0, frame));
         mXml.addText(dt.toString(Qt::ISODate).toStdString(), mXml.addElement("when"));
         mXml.popAddPoint();
      }
      fileName += ".png";
      if (ImageHandler::getSessionItemImage(pLayer, buffer, "PNG", frame, bbox))
      {
         mImages[fileName] = bytes;
         mXml.pushAddPoint(mXml.addElement("Icon"));
         mXml.addText(fileName.toStdString(), mXml.addElement("href"));
         mXml.popAddPoint();
      }
   }
   else
   {
      layerId = QUrl::toPercentEncoding(layerId);
      QString imageUrl(QString("http://localhost:%1/images/%2.png").arg(KMLServer::getSettingKmlServerPort()).arg(
         layerId));
      if (frame >= 0)
      {
         imageUrl += QString("?frame=%1").arg(frame);
         mXml.pushAddPoint(mXml.addElement("TimeStamp"));
         QDateTime dt = QDateTime::currentDateTime();
         dt.setTime(QTime(0, 0, frame));
         mXml.addText(dt.toString(Qt::ISODate).toStdString(), mXml.addElement("when"));
         mXml.popAddPoint();
      }
      mXml.pushAddPoint(mXml.addElement("Icon"));
      mXml.addText(imageUrl.toStdString(), mXml.addElement("href"));
      mXml.popAddPoint();
   }
   generateBoundingBox(pGeoLayer, bbox);

   // add ExtendedData
   const DynamicObject* pMeta = pLayer->getDataElement()->getMetadata();
   VERIFYNRV(pMeta);
   const DataVariant& ked(pMeta->getAttributeByPath("KML/ExtendedData"));
   if (ked.isValid())
   {
      std::string kedString = ked.toXmlString();
      XmlReader reader(Service<MessageLogMgr>()->getLog(), false);
      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* kedDoc = reader.parseString(ked.toXmlString());
      if (kedDoc != NULL)
      {
         XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pKedNode = kedDoc->getDocumentElement();
         bool extraPop = false;
         if (std::string(A(pKedNode->getNodeName())) != "ExtendedData")
         {
            mXml.pushAddPoint(mXml.addElement("ExtendedData"));
            extraPop = true;
         }
         pKedNode = mXml.peekAddPoint()->getOwnerDocument()->importNode(pKedNode, true);
         mXml.peekAddPoint()->appendChild(pKedNode);
         if (extraPop)
         {
            mXml.popAddPoint();
         }
      }
   }

   mXml.popAddPoint(); // GroundOverlay

   // restore original rotation
   if (imageFlipped)
   {
      pView->flipHorizontal();
   }
   pView->rotateTo(originalAngle);
   pView->refresh();
}

const QMap<QString,QByteArray> Kml::getImages() const
{
   return mImages;
}
