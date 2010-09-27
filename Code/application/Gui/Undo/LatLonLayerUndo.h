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
   DmsFormatType mOldFormat;
   DmsFormatType mNewFormat;
};


class LatLonLayerMemento : public LayerMemento
{
public:
   LatLonLayerMemento(LatLonLayer* pLayer);
   void toLayer(Layer* pLayer) const;

private:
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
