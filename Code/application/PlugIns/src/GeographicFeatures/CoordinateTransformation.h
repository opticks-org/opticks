/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COORDINATETRANSFORMATION_H
#define COORDINATETRANSFORMATION_H

namespace ArcProxyLib
{
   class ConnectionParameters;
}

class OGRCoordinateTransformation;

// Forward and inverse transformations for shapefiles with .prj projection files.
// Assumes that the application is using WGS84.
class CoordinateTransformation
{
public:
   CoordinateTransformation(const ArcProxyLib::ConnectionParameters& connParams);
   ~CoordinateTransformation();

   bool translateAppToShape(double xIn, double yIn, double& xOut, double& yOut) const;
   bool translateShapeToApp(double xIn, double yIn, double& xOut, double& yOut) const;

private:
   OGRCoordinateTransformation* mpAppToShapeTransformation;
   OGRCoordinateTransformation* mpShapeToAppTransformation;

   // Not implemented.
   CoordinateTransformation();
   CoordinateTransformation& operator=(const CoordinateTransformation&);
   CoordinateTransformation(const CoordinateTransformation&);
};

#endif
