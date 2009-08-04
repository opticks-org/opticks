/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICUTILITIES_H
#define GRAPHICUTILITIES_H

#include "ColorType.h"
#include "LocationType.h"
#include "TypesFile.h"

#include <list>
#include <string>

class GraphicObject;
class GraphicProperty;
class View;

namespace GraphicUtilities
{
   /**
    *  Sets a property on multiple graphic objects.
    *
    *  @param   objects
    *           The graphic objects for which to set the property.  If an
    *           object in the given list does not have the given property, the
    *           object is not modified.
    *  @param   pProperty
    *           The property to set on the graphic objects.
    *
    *  @return  Returns \c true if the property was set on at least one of the
    *           given objects; otherwise returns \c false.
    */
   bool setProperty(const std::list<GraphicObject*>& objects, GraphicProperty* pProperty);

   /**
    *  Retrieves a common property for multiple graphic objects.
    *
    *  @param   objects
    *           The graphic objects for which to get the property.
    *  @param   propertyName
    *           The name of the property to get from the graphic objects.
    *
    *  @return  Returns a property with the common value across the given
    *           objects.  If at least one object has a different value for the
    *           given property name, \c NULL is returned.  If an object in the
    *           given list does not have the property with the given name, that
    *           object is ignored and the return value is not affected.
    */
   const GraphicProperty* getProperty(const std::list<GraphicObject*>& objects, const std::string& propertyName);

   // Common
   bool setBoundingBox(const std::list<GraphicObject*>& objects, LocationType lowerLeft, LocationType upperRight);
   LocationType getLlCorner(const std::list<GraphicObject*>& objects);
   LocationType getUrCorner(const std::list<GraphicObject*>& objects);
   bool setRotation(const std::list<GraphicObject*>& objects, double angle);
   double getRotation(const std::list<GraphicObject*>& objects);

   // Arc
   bool setAngles(const std::list<GraphicObject*>& objects, double dStart, double dStop);
   double getStartAngle(const std::list<GraphicObject*>& objects);
   double getStopAngle(const std::list<GraphicObject*>& objects);
   bool setArcRegion(const std::list<GraphicObject*>& objects, ArcRegion eRegion);
   ArcRegion getArcRegion(const std::list<GraphicObject*>& objects);

   // Fill
   bool setFillState(const std::list<GraphicObject*>& objects, bool fillState);
   bool getFillState(const std::list<GraphicObject*>& objects);
   bool setFillColor(const std::list<GraphicObject*>& objects, ColorType fillColor);
   ColorType getFillColor(const std::list<GraphicObject*>& objects);
   bool setFillStyle(const std::list<GraphicObject*>& objects, FillStyle eStyle);
   FillStyle getFillStyle(const std::list<GraphicObject*>& objects);
   bool setHatchStyle(const std::list<GraphicObject*>& objects, SymbolType eHatch);
   SymbolType getHatchStyle(const std::list<GraphicObject*>& objects);

   // Image
   bool setImageFile(const std::list<GraphicObject*>& objects, const std::string& filename);
   std::string getImageFile(const std::list<GraphicObject*>& objects);
   bool setAlpha(const std::list<GraphicObject*>& objects, double alpha);
   double getAlpha(const std::list<GraphicObject*>& objects);

   // Line
   bool setLineState(const std::list<GraphicObject*>& objects, bool lineState);
   bool getLineState(const std::list<GraphicObject*>& objects);
   bool setLineStyle(const std::list<GraphicObject*>& objects, LineStyle eStyle);
   LineStyle getLineStyle(const std::list<GraphicObject*>& objects);
   bool setLineWidth(const std::list<GraphicObject*>& objects, double width);
   double getLineWidth(const std::list<GraphicObject*>& objects);
   bool setLineColor(const std::list<GraphicObject*>& objects, ColorType lineColor);
   ColorType getLineColor(const std::list<GraphicObject*>& objects);
   bool setLineScaled(const std::list<GraphicObject*>& objects, bool scaled);
   bool getLineScaled(const std::list<GraphicObject*>& objects);

   // Scale
   bool setScale(const std::list<GraphicObject*>& objects, double scale);
   double getScale(const std::list<GraphicObject*>& objects);

   // Symbol
   bool setGraphicSymbol(const std::list<GraphicObject*>& objects, const std::string& symbolName);
   std::string getGraphicSymbol(const std::list<GraphicObject*>& objects);
   bool setGraphicSymbolSize(const std::list<GraphicObject*>& objects, unsigned int size);
   unsigned int getGraphicSymbolSize(const std::list<GraphicObject*>& objects);

   // Text
   bool setText(const std::list<GraphicObject*>& objects, const std::string& text);
   std::string getText(const std::list<GraphicObject*>& objects);
   bool setTextAlignment(const std::list<GraphicObject*>& objects, int iAlignment);
   int getTextAlignment(const std::list<GraphicObject*>& objects);
   bool setFont(const std::list<GraphicObject*>& objects, const QFont& font);
   QFont getFont(const std::list<GraphicObject*>& objects);
   bool setFontName(const std::list<GraphicObject*>& objects, const std::string& fontName);
   std::string getFontName(const std::list<GraphicObject*>& objects);
   bool setFontSize(const std::list<GraphicObject*>& objects, int fontSize);
   int getFontSize(const std::list<GraphicObject*>& objects);
   bool setFontBold(const std::list<GraphicObject*>& objects, bool bBold);
   bool getFontBold(const std::list<GraphicObject*>& objects);
   bool setFontItalics(const std::list<GraphicObject*>& objects, bool bItalics);
   bool getFontItalics(const std::list<GraphicObject*>& objects);
   bool setFontUnderline(const std::list<GraphicObject*>& objects, bool bUnderline);
   bool getFontUnderline(const std::list<GraphicObject*>& objects);
   bool setTextColor(const std::list<GraphicObject*>& objects, ColorType textColor);
   ColorType getTextColor(const std::list<GraphicObject*>& objects);

   // Triangle
   bool setApex(const std::list<GraphicObject*>& objects, double dApex);
   double getApex(const std::list<GraphicObject*>& objects);

   // View
   bool setObjectView(const std::list<GraphicObject*>& objects, View* pView);
   View* getObjectView(const std::list<GraphicObject*>& objects);

   // Units
   bool setUnitSystem(const std::list<GraphicObject*>& objects, UnitSystem units);
   UnitSystem getUnitSystem(const std::list<GraphicObject*>& objects);
};

#endif
