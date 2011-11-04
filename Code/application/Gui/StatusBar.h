/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QtGui/QLabel>
#include <QtGui/QStatusBar>

#include "Observer.h"
#include "TypesFile.h"
#include "GeoPoint.h"

#include <boost/any.hpp>

class Subject;
class Units;

class StatusBar : public QStatusBar, public Observer
{
   Q_OBJECT

public:
   StatusBar(QWidget* parent = 0);
   ~StatusBar();

   void optionsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void attached(Subject& subject, const std::string& signal, const Slot& slot);

   bool arePixelCoordsShown();
   bool areGeoCoordsShown();
   bool isCubeValueShown();
   bool isElevationShown();
   bool isResultValueShown();
   bool isRotationShown();

public slots:
   void setPixelCoords(int iXCoord, int iYCoord);
   void setPixelCoords(const QPoint& ptPixel);
   void clearPixelCoords();
   void showPixelCoords(bool bShow);

   void setGeoCoords(double xCoord, double yCoord, GeocoordType type, DmsFormatType format = DMS_FULL);
   void setGeoCoords(LocationType latLonCoord, GeocoordType type, DmsFormatType format = DMS_FULL);
   void clearGeoCoords();
   void showGeoCoords(bool bShow);

   void setCubeValue(const QString& layerName, double gray);
   void setCubeValue(const QString& layerName, const QString& strGray);
   void setCubeValue(const QString& layerName, double red, double green, double blue);
   void setCubeValue(const QString& layerName, const QString& strRed, const QString& strGreen, const QString& strBlue);
   void clearCubeValue();
   void showCubeValue(bool bShow);

   void setElevationValue(double value, const Units* pUnits = NULL);
   void clearElevationValue();
   void showElevationValue(bool bShow);

   void setResultValue(const QString& layerName, double value, const Units* pUnits = NULL);
   void clearResultValue();
   void showResultValue(bool bShow);

   void setRotationValue(double value);
   void clearRotationValue();
   void showRotationValue(bool bShow);

   void clearValues();

protected slots:
   void updateDisplayedWidgets();

private:
   StatusBar(const StatusBar& rhs);
   StatusBar& operator=(const StatusBar& rhs);
   QLabel* m_pResultValue_Label;
   QLabel* m_pCubeValue_Label;
   QLabel* m_pCoordinates_Label;
   QLabel* m_pGeoCoordinates_Label;
   QLabel* m_pRotation_Label;
   QLabel* m_pElevation_Label;
};

#endif
