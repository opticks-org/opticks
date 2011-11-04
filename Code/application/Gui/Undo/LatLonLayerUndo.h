/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONLAYERUNDO_H
#define LATLONLAYERUNDO_H

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "ColorType.h"
#include "LayerUndo.h"
#include "LocationType.h"
#include "TypesFile.h"
#include "UndoAction.h"

#include <string>

class LatLonLayer;

class SetLatLonColor : public UndoAction
{
public:
   SetLatLonColor(LatLonLayer* pLayer, const ColorType& oldColor, const ColorType& newColor);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonColor(const SetLatLonColor& rhs);
   SetLatLonColor& operator=(const SetLatLonColor& rhs);
   ColorType mOldColor;
   ColorType mNewColor;
};


class SetLatLonStyle : public UndoAction
{
public:
   SetLatLonStyle(LatLonLayer* pLayer, LatLonStyle oldStyle, LatLonStyle newStyle);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonStyle(const SetLatLonStyle& rhs);
   SetLatLonStyle& operator=(const SetLatLonStyle& rhs);
   LatLonStyle mOldStyle;
   LatLonStyle mNewStyle;
};


class SetLatLonWidth : public UndoAction
{
public:
   SetLatLonWidth(LatLonLayer* pLayer, int oldWidth, int newWidth);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonWidth(const SetLatLonWidth& rhs);
   SetLatLonWidth& operator=(const SetLatLonWidth& rhs);
   int mOldWidth;
   int mNewWidth;
};


class SetLatLonTickSpacing : public UndoAction
{
public:
   SetLatLonTickSpacing(LatLonLayer* pLayer, const LocationType& oldTickSpacing, const LocationType& newTickSpacing);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonTickSpacing(const SetLatLonTickSpacing& rhs);
   SetLatLonTickSpacing& operator=(const SetLatLonTickSpacing& rhs);
   LocationType mOldTickSpacing;
   LocationType mNewTickSpacing;
};


class SetLatLonAutoTickSpacing : public UndoAction
{
public:
   SetLatLonAutoTickSpacing(LatLonLayer* pLayer, bool bOldAutoSpacing, bool bNewAutoSpacing);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonAutoTickSpacing(const SetLatLonAutoTickSpacing& rhs);
   SetLatLonAutoTickSpacing& operator=(const SetLatLonAutoTickSpacing& rhs);
   bool mOldAutoSpacing;
   bool mNewAutoSpacing;
};


class SetLatLonFont : public UndoAction
{
public:
   SetLatLonFont(LatLonLayer* pLayer, const QFont& oldFont, const QFont& newFont);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonFont(const SetLatLonFont& rhs);
   SetLatLonFont& operator=(const SetLatLonFont& rhs);
   QFont mOldFont;
   QFont mNewFont;
};


class SetLatLonGeocoordType : public UndoAction
{
public:
   SetLatLonGeocoordType(LatLonLayer* pLayer, GeocoordType oldGeocoordType, GeocoordType newGeocoordType);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonGeocoordType(const SetLatLonGeocoordType& rhs);
   SetLatLonGeocoordType& operator=(const SetLatLonGeocoordType& rhs);
   GeocoordType mOldGeocoordType;
   GeocoordType mNewGeocoordType;
};


class SetLatLonFormat : public UndoAction
{
public:
   SetLatLonFormat(LatLonLayer* pLayer, DmsFormatType oldFormat, DmsFormatType newFormat);

   void executeUndo();
   void executeRedo();

private:
   SetLatLonFormat(const SetLatLonFormat& rhs);
   SetLatLonFormat& operator=(const SetLatLonFormat& rhs);
   DmsFormatType mOldFormat;
   DmsFormatType mNewFormat;
};


class LatLonLayerMemento : public LayerMemento
{
public:
   LatLonLayerMemento(LatLonLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
   LatLonLayerMemento(const LatLonLayerMemento& rhs);
   LatLonLayerMemento& operator=(const LatLonLayerMemento& rhs);
   QColor mColor;
   LatLonStyle mStyle;
   int mWidth;
   LocationType mTickSpacing;
   bool mAutoSpacing;
   QFont mFont;
   GeocoordType mGeocoordType;
   DmsFormatType mFormat;
};

#endif
