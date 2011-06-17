/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef QUERYOPTIONS_H
#define QUERYOPTIONS_H

#include "ColorType.h"
#include "TypesFile.h"
#include <string>

class DynamicObject;
class GraphicObject;
class PlugInArgList;

class QueryOptions
{
public:
   QueryOptions();
   ~QueryOptions();

   /**
    * Convert options to a DynamicObject.
    * 
    * @return A DynamicObject containing the import options.  The caller is
    *         responsible for deleting this DynamicObject.
    */
   DynamicObject *toDynamicObject() const;
   void fromDynamicObject(const DynamicObject *pDynObj);

   void setOnGraphicObject(GraphicObject *pObj) const;

   void setQueryName(const std::string &queryName);
   const std::string &getQueryName() const;

   void setFormatString(const std::string &formatString);
   const std::string &getFormatString() const;

   void setQueryString(const std::string &queryString);
   const std::string &getQueryString() const;

   void setSymbolName(const std::string &symbolName);
   const std::string &getSymbolName() const;

   void setSymbolSize(unsigned int symbolSize);
   unsigned int getSymbolSize() const;

   void setLineState(bool lineState);
   bool getLineState() const;

   void setLineStyle(LineStyle lineStyle);
   LineStyle getLineStyle() const;

   void setLineWidth(double lineWidth);
   double getLineWidth() const;

   void setLineColor(const ColorType &lineColor);
   const ColorType &getLineColor() const;

   void setLineScaled(bool lineScaled);
   bool getLineScaled() const;

   void setFillColor(const ColorType &fillColor);
   const ColorType &getFillColor() const;

   void setFillStyle(FillStyle fillStyle);
   FillStyle getFillStyle() const;

   void setHatchStyle(SymbolType hatchStyle);
   SymbolType getHatchStyle() const;

private:
   static const std::string QUERY_NAME;
   static const std::string FORMAT_STRING;
   static const std::string QUERY_STRING;
   static const std::string SYMBOL_NAME;
   static const std::string SYMBOL_SIZE;
   static const std::string LINE_STATE;
   static const std::string LINE_STYLE;
   static const std::string LINE_WIDTH;
   static const std::string LINE_COLOR;
   static const std::string LINE_SCALED;
   static const std::string FILL_COLOR;
   static const std::string FILL_STYLE;
   static const std::string HATCH_STYLE;

   static const ColorType mDefaultColor;
   static const std::string mDefaultName;

   // feature
   std::string mQueryName;
   std::string mFormatString;
   std::string mQueryString;

   // point
   std::string mSymbolName;
   unsigned int mSymbolSize;

   // line
   bool mLineState;
   LineStyle mLineStyle;
   double mLineWidth;
   ColorType mLineColor;
   bool mLineScaled;
   
   // polygon
   ColorType mFillColor;
   FillStyle mFillStyle;
   SymbolType mHatchStyle;
};

#endif
