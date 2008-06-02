/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef KML_H
#define KML_H

#include "xmlwriter.h"
#include <QtCore/QMap>

class GraphicLayer;
class Layer;
class QByteArray;
class QString;
class RasterElement;
class SpatialDataView;
class SpatialDataWindow;

class Kml
{
public:
   Kml(bool exportImages=false);
   ~Kml();

   QString toString();
   bool toFile(const std::string &filename);

   bool addSession();
   bool addWindow(SpatialDataWindow *pWindow);
   bool addView(SpatialDataView *pView);
   bool addLayer(Layer *pLayer, RasterElement *pGeoElement, SpatialDataView *pView, int totalLayers);

   void generateBoundingBox(RasterElement *pGeoElement);
   void generatePolygonalLayer(GraphicLayer *pGraphicLayer, bool visible, int order, RasterElement *pGeoElement);
   void generateGroundOverlayLayer(Layer *pLayer, bool visible, int order, RasterElement *pGeoElement, int frame=-1);

   const QMap<QString,QByteArray> getImages() const;

private:
   XMLWriter mXml;
   bool mExportImages;
   QMap<QString,QByteArray> mImages;
};

#endif
