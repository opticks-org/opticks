/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConnectionParameters.h"
#include "CoordinateTransformation.h"

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

#include <gdal/ogr_spatialref.h>

#include <string>

CoordinateTransformation::CoordinateTransformation(const ArcProxyLib::ConnectionParameters& connParams) :
   mpAppToShapeTransformation(NULL),
   mpShapeToAppTransformation(NULL)
{
   QDir dir(QString::fromStdString(connParams.getDatabase()));
   QString filename = dir.absoluteFilePath(QString::fromStdString(connParams.getFeatureClass()));
   QFileInfo shapeFileInfo(dir, filename);
   if (shapeFileInfo.exists() == true && shapeFileInfo.isFile() == true)
   {
      QFile prjFile(shapeFileInfo.canonicalPath() + "/" + shapeFileInfo.completeBaseName() + ".prj");
      if (prjFile.open(QFile::ReadOnly | QFile::Text))
      {
         QByteArray fileBytes(prjFile.readAll());
         std::string wkt = QString(fileBytes).toStdString();

         OGRSpatialReference shapeReference(wkt.c_str());
         OGRSpatialReference appReference;
         if (shapeReference.Validate() == OGRERR_NONE &&
            appReference.SetWellKnownGeogCS("WGS84") == OGRERR_NONE &&
            shapeReference.IsSame(&appReference) == FALSE)
         {
            mpAppToShapeTransformation = OGRCreateCoordinateTransformation(&appReference, &shapeReference);
            mpShapeToAppTransformation = OGRCreateCoordinateTransformation(&shapeReference, &appReference);
         }
      }
   }
}

CoordinateTransformation::~CoordinateTransformation()
{
   delete mpAppToShapeTransformation;
   delete mpShapeToAppTransformation;
}

bool CoordinateTransformation::translateAppToShape(double xIn, double yIn, double& xOut, double& yOut) const
{
   yOut = yIn;
   xOut = xIn;
   return mpAppToShapeTransformation == NULL || mpAppToShapeTransformation->Transform(1, &xOut, &yOut);
}

bool CoordinateTransformation::translateShapeToApp(double xIn, double yIn, double& xOut, double& yOut) const
{
   yOut = yIn;
   xOut = xIn;
   return mpShapeToAppTransformation == NULL || mpShapeToAppTransformation->Transform(1, &xOut, &yOut);
}
