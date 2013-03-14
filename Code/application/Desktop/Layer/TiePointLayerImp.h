/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TIEPOINTLAYERIMP_H
#define TIEPOINTLAYERIMP_H

#include <QtCore/QPoint>
#include <QtGui/QColor>

#include "ColorType.h"
#include "LayerImp.h"
#include "LocationType.h"
#include "TypesFile.h"
#include "TiePointList.h"

class TiePointLayerImp : public LayerImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   TiePointLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~TiePointLayerImp();

   TiePointLayerImp& operator= (const TiePointLayerImp& tiePointLayer);

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

   LayerType getLayerType() const;

   std::vector<ColorType> getColors() const;
   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);
   bool getExtents(std::vector<LocationType>& worldCoords);

   void setColor(const QColor& colorType);
   QColor getColor() const;
   void setSymbolSize(int iSize);
   int getSymbolSize() const;
   void enableLabels(bool enabled);
   bool areLabelsEnabled() const;

   bool isMission() const;
   void setIsMission(bool isMission);

   bool acceptsMouseEvents() const;
   QCursor getMouseCursor() const;

   bool processMousePress(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseMove(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   bool processMouseRelease(const QPoint& screenCoord, Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);

public slots:
   void reset();

signals:
   void colorChanged(const QColor& clrGcp);
   void sizeChanged(int iSize);
   void labelEnabledChanged(bool bEnabled);

private:
   TiePointLayerImp(const TiePointLayerImp& rhs);

   int drawSymbols(const std::vector<TiePoint> &points, double sceneSymbolSize, const int viewableBounds[4]) const;
   void drawLabels(const std::vector<TiePoint>& points, double sceneSymbolSize, const int viewableBounds[4]) const;
   void drawSymbol(const LocationType& point, double symbolSize) const;
   LocationType getPoint(const TiePoint& tiePoint) const
   {
      LocationType point (tiePoint.mReferencePoint.mX, tiePoint.mReferencePoint.mY);
      if (mIsMission)
      {
         point += LocationType(tiePoint.mMissionOffset.mX, tiePoint.mMissionOffset.mY);
      }
      return point;
   }

   QColor mColor;
   int mSymbolSize;
   bool mLabelsEnabled;
   bool mIsMission; // else is reference image
   static unsigned int msNumLayers;
   static LocationType sAnchor;
};

#define TIEPOINTLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define TIEPOINTLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   void setColor(const ColorType& colorType) \
   { \
      QColor clrTiePoint; \
      if (colorType.isValid() == true) \
      { \
         clrTiePoint.setRgb(colorType.mRed, colorType.mGreen, colorType.mBlue); \
      } \
      impClass::setColor(clrTiePoint); \
   } \
   ColorType getColor() const \
   { \
      ColorType tiePointColor; \
      QColor clrTiePoint = impClass::getColor(); \
      if (clrTiePoint.isValid() == true) \
      { \
         tiePointColor.mRed = clrTiePoint.red(); \
         tiePointColor.mGreen = clrTiePoint.green(); \
         tiePointColor.mBlue = clrTiePoint.blue(); \
      } \
      return tiePointColor; \
   } \
   void setSymbolSize(int iSize) \
   { \
      impClass::setSymbolSize(iSize); \
   } \
   int getSymbolSize() const \
   { \
      return impClass::getSymbolSize(); \
   } \
   void enableLabels(bool enabled) \
   { \
      impClass::enableLabels(enabled); \
   } \
   bool areLabelsEnabled() const \
   { \
      return impClass::areLabelsEnabled(); \
   } \
   bool isMission() const \
   { \
      return impClass::isMission(); \
   } \
   void setIsMission(bool isMission) \
   { \
      impClass::setIsMission(isMission); \
   }

#endif
