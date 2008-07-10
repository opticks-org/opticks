/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QApplication>

#include "LatLonLayer.h"
#include "LatLonLayerImp.h"
#include "LatLonLayerUndo.h"

////////////////////
// SetLatLonColor //
////////////////////

SetLatLonColor::SetLatLonColor(LatLonLayer* pLayer, const ColorType& oldColor, const ColorType& newColor) :
   UndoAction(pLayer),
   mOldColor(oldColor),
   mNewColor(newColor)
{
   setText("Set Latitude/Longitude Color");
}

void SetLatLonColor::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mOldColor);
   }
}

void SetLatLonColor::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setColor(mNewColor);
   }
}

////////////////////
// SetLatLonStyle //
////////////////////

SetLatLonStyle::SetLatLonStyle(LatLonLayer* pLayer, LatLonStyle oldStyle, LatLonStyle newStyle) :
   UndoAction(pLayer),
   mOldStyle(oldStyle),
   mNewStyle(newStyle)
{
   setText("Set Latitude/Longitude Style");
}

void SetLatLonStyle::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStyle(mOldStyle);
   }
}

void SetLatLonStyle::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setStyle(mNewStyle);
   }
}

////////////////////
// SetLatLonWidth //
////////////////////

SetLatLonWidth::SetLatLonWidth(LatLonLayer* pLayer, int oldWidth, int newWidth) :
   UndoAction(pLayer),
   mOldWidth(oldWidth),
   mNewWidth(newWidth)
{
   setText("Set Latitude/Longitude Width");
}

void SetLatLonWidth::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setWidth(mOldWidth);
   }
}

void SetLatLonWidth::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setWidth(mNewWidth);
   }
}

//////////////////////////
// SetLatLonTickSpacing //
//////////////////////////

SetLatLonTickSpacing::SetLatLonTickSpacing(LatLonLayer* pLayer, const LocationType& oldTickSpacing,
                                           const LocationType& newTickSpacing) :
   UndoAction(pLayer),
   mOldTickSpacing(oldTickSpacing),
   mNewTickSpacing(newTickSpacing)
{
   setText("Set Latitude/Longitude Tick Spacing");
}

void SetLatLonTickSpacing::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setTickSpacing(mOldTickSpacing);
   }
}

void SetLatLonTickSpacing::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setTickSpacing(mNewTickSpacing);
   }
}

//////////////////////////////
// SetLatLonAutoTickSpacing //
//////////////////////////////

SetLatLonAutoTickSpacing::SetLatLonAutoTickSpacing(LatLonLayer* pLayer, bool bOldAutoSpacing, bool bNewAutoSpacing) :
   UndoAction(pLayer),
   mOldAutoSpacing(bOldAutoSpacing),
   mNewAutoSpacing(bNewAutoSpacing)
{
   setText("Set Latitude/Longitude Auto Tick Spacing");
}

void SetLatLonAutoTickSpacing::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setAutoTickSpacing(mOldAutoSpacing);
   }
}

void SetLatLonAutoTickSpacing::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setAutoTickSpacing(mNewAutoSpacing);
   }
}

///////////////////
// SetLatLonFont //
///////////////////

SetLatLonFont::SetLatLonFont(LatLonLayer* pLayer, const QFont& oldFont, const QFont& newFont) :
   UndoAction(pLayer),
   mOldFont(oldFont),
   mNewFont(newFont)
{
   setText("Set Latitude/Longitude Font");
}

void SetLatLonFont::executeUndo()
{
   LatLonLayerImp* pLayer = dynamic_cast<LatLonLayerImp*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setFont(mOldFont);
   }
}

void SetLatLonFont::executeRedo()
{
   LatLonLayerImp* pLayer = dynamic_cast<LatLonLayerImp*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setFont(mNewFont);
   }
}

///////////////////////////
// SetLatLonGeocoordType //
///////////////////////////

SetLatLonGeocoordType::SetLatLonGeocoordType(LatLonLayer* pLayer, GeocoordType oldGeocoordType,
                                             GeocoordType newGeocoordType) :
   UndoAction(pLayer),
   mOldGeocoordType(oldGeocoordType),
   mNewGeocoordType(newGeocoordType)
{
   setText("Set Latitude/Longitude Coordinate Type");
}

void SetLatLonGeocoordType::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setGeocoordType(mOldGeocoordType);
   }
}

void SetLatLonGeocoordType::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setGeocoordType(mNewGeocoordType);
   }
}

/////////////////////
// SetLatLonFormat //
/////////////////////

SetLatLonFormat::SetLatLonFormat(LatLonLayer* pLayer, DmsFormatType oldFormat, DmsFormatType newFormat) :
   UndoAction(pLayer),
   mOldFormat(oldFormat),
   mNewFormat(newFormat)
{
   setText("Set Latitude/Longitude Format");
}

void SetLatLonFormat::executeUndo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setLatLonFormat(mOldFormat);
   }
}

void SetLatLonFormat::executeRedo()
{
   LatLonLayer* pLayer = dynamic_cast<LatLonLayer*>(getSessionItem());
   if (pLayer != NULL)
   {
      pLayer->setLatLonFormat(mNewFormat);
   }
}

////////////////////////
// LatLonLayerMemento //
////////////////////////

LatLonLayerMemento::LatLonLayerMemento(LatLonLayer* pLayer) :
   mColor(QColor()),
   mStyle(LATLONSTYLE_SOLID),
   mWidth(1),
   mTickSpacing(LocationType()),
   mAutoSpacing(true),
   mFont(QApplication::font()),
   mGeocoordType(GEOCOORD_GENERAL),
   mFormat(DMS_FULL)
{
   LatLonLayerImp* pLayerImp = dynamic_cast<LatLonLayerImp*>(pLayer);
   if (pLayerImp != NULL)
   {
      mColor = pLayerImp->getColor();
      mStyle = pLayerImp->getStyle();
      mWidth = pLayerImp->getWidth();
      mTickSpacing = pLayerImp->getTickSpacing();
      mAutoSpacing = pLayerImp->getAutoTickSpacing();
      mFont = pLayerImp->getFont();
      mGeocoordType = pLayerImp->getGeocoordType();
      mFormat = pLayerImp->getLatLonFormat();
   }
}

void LatLonLayerMemento::toLayer(Layer* pLayer) const
{
   LatLonLayerImp* pLayerImp = dynamic_cast<LatLonLayerImp*>(pLayer);
   if (pLayerImp != NULL)
   {
      pLayerImp->setColor(mColor);
      pLayerImp->setStyle(mStyle);
      pLayerImp->setWidth(mWidth);
      pLayerImp->setTickSpacing(mTickSpacing);
      pLayerImp->setAutoTickSpacing(mAutoSpacing);
      pLayerImp->setFont(mFont);
      pLayerImp->setGeocoordType(mGeocoordType);
      pLayerImp->setLatLonFormat(mFormat);
   }
}
