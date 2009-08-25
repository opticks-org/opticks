/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "AppVerify.h"
#include "DynamicObject.h"
#include "GraphicLayer.h"
#include "GraphicObject.h"
#include "ObjectResource.h"
#include "QueryOptions.h"

namespace
{
   class DynamicObjectSource
   {
   public:
      DynamicObjectSource(DynamicObject &dynObj) : mDynObj(dynObj)
      {
      }

      template<typename T>
      const T *get(const std::string &name) const
      {
         const DataVariant &variant = mDynObj.getAttribute(name);
         const T *pT = variant.getPointerToValue<T>();
         VERIFYRV(pT == NULL || variant.getTypeName() == TypeConverter::toString<T>(), NULL);
         return pT;
      }

      template<typename T>
      T *get(const std::string &name)
      {
         return const_cast<T*>(const_cast<const DynamicObjectSource*>(this)->get<T>(name));
      }

      template<typename T>
      void set(const std::string &name, const T &value)
      {
         mDynObj.setAttribute(name, value);
      }

   private:
      DynamicObject& mDynObj;
   };
}

const std::string QueryOptions::QUERY_NAME = "Query name";
const std::string QueryOptions::FORMAT_STRING = "Format string";
const std::string QueryOptions::QUERY_STRING = "Query string";
const std::string QueryOptions::SYMBOL_NAME = "Symbol name";
const std::string QueryOptions::SYMBOL_SIZE = "Symbol size";
const std::string QueryOptions::LINE_STATE = "Line state";
const std::string QueryOptions::LINE_STYLE = "Line style";
const std::string QueryOptions::LINE_WIDTH = "Line width";
const std::string QueryOptions::LINE_COLOR = "Line color";
const std::string QueryOptions::LINE_SCALED = "Line scaled";
const std::string QueryOptions::FILL_COLOR = "Fill color";
const std::string QueryOptions::FILL_STYLE = "Fill style";
const std::string QueryOptions::HATCH_STYLE = "Hatch style";

const ColorType QueryOptions::mDefaultColor = ColorType(255, 255, 0);
const std::string QueryOptions::mDefaultName = "New Query";

QueryOptions::QueryOptions() :
   mQueryName(mDefaultName),
   mSymbolName(GraphicLayer::getSettingSymbolName()),
   mSymbolSize(GraphicLayer::getSettingSymbolSize()),
   mLineState(GraphicLayer::getSettingFill()),
   mLineStyle(GraphicLayer::getSettingLineStyle()),
   mLineWidth(GraphicLayer::getSettingLineWidth()),
   mLineColor(mDefaultColor),
   mLineScaled(GraphicLayer::getSettingLineScaled()),
   mFillColor(mDefaultColor),
   mFillStyle(EMPTY_FILL),
   mHatchStyle(GraphicLayer::getSettingHatchStyle())
{
}

QueryOptions::~QueryOptions()
{
}

void QueryOptions::fromDynamicObject(const DynamicObject *pDynObj)
{
   VERIFYNRV(pDynObj != NULL);

   pDynObj->getAttribute(QUERY_NAME).getValue(mQueryName);
   pDynObj->getAttribute(FORMAT_STRING).getValue(mFormatString);
   pDynObj->getAttribute(QUERY_STRING).getValue(mQueryString);
   pDynObj->getAttribute(SYMBOL_NAME).getValue(mSymbolName);
   pDynObj->getAttribute(SYMBOL_SIZE).getValue(mSymbolSize);
   pDynObj->getAttribute(LINE_STATE).getValue(mLineState);
   pDynObj->getAttribute(LINE_STYLE).getValue(mLineStyle);
   pDynObj->getAttribute(LINE_WIDTH).getValue(mLineWidth);
   pDynObj->getAttribute(LINE_COLOR).getValue(mLineColor);
   pDynObj->getAttribute(LINE_SCALED).getValue(mLineScaled);
   pDynObj->getAttribute(FILL_COLOR).getValue(mFillColor);
   pDynObj->getAttribute(FILL_STYLE).getValue(mFillStyle);
   pDynObj->getAttribute(HATCH_STYLE).getValue(mHatchStyle);
}

DynamicObject *QueryOptions::toDynamicObject() const
{
   FactoryResource<DynamicObject> pDynObj;
   VERIFYRV(pDynObj.get() != NULL, NULL);

   pDynObj->setAttribute(QUERY_NAME, mQueryName);
   pDynObj->setAttribute(FORMAT_STRING, mFormatString);
   pDynObj->setAttribute(QUERY_STRING, mQueryString);
   pDynObj->setAttribute(SYMBOL_NAME, mSymbolName);
   pDynObj->setAttribute(SYMBOL_SIZE, mSymbolSize);
   pDynObj->setAttribute(LINE_STATE, mLineState);
   pDynObj->setAttribute(LINE_STYLE, mLineStyle);
   pDynObj->setAttribute(LINE_WIDTH, mLineWidth);
   pDynObj->setAttribute(LINE_COLOR, mLineColor);
   pDynObj->setAttribute(LINE_SCALED, mLineScaled);
   pDynObj->setAttribute(FILL_COLOR, mFillColor);
   pDynObj->setAttribute(FILL_STYLE, mFillStyle);
   pDynObj->setAttribute(HATCH_STYLE, mHatchStyle);

   return pDynObj.release();
}

void QueryOptions::setOnGraphicObject(GraphicObject *pObj) const
{
   VERIFYNRV(pObj != NULL);

   pObj->setSymbolName(mSymbolName);
   pObj->setSymbolSize(mSymbolSize);
   pObj->setLineState(mLineState);
   pObj->setLineStyle(mLineStyle);
   pObj->setLineWidth(mLineWidth);
   pObj->setLineColor(mLineColor);
   pObj->setLineScaled(mLineScaled);
   pObj->setFillColor(mFillColor);
   pObj->setFillStyle(mFillStyle);
   pObj->setHatchStyle(mHatchStyle);

}

void QueryOptions::setQueryName(const std::string &queryName)
{
   if (!queryName.empty())
   {
      mQueryName = queryName;
   }
}

const std::string &QueryOptions::getQueryName() const
{
   return mQueryName;
}

void QueryOptions::setFormatString(const std::string &formatString)
{
   mFormatString = formatString;
}

const std::string &QueryOptions::getFormatString() const
{
   return mFormatString;
}

void QueryOptions::setQueryString(const std::string &queryString)
{
   mQueryString = queryString;
}

const std::string &QueryOptions::getQueryString() const
{
   return mQueryString;
}

void QueryOptions::setSymbolName(const std::string &symbolName)
{
   mSymbolName = symbolName;
}

const std::string &QueryOptions::getSymbolName() const
{
   return mSymbolName;
}

void QueryOptions::setSymbolSize(unsigned int symbolSize)
{
   mSymbolSize = symbolSize;
}

unsigned int QueryOptions::getSymbolSize() const
{
   return mSymbolSize;
}

void QueryOptions::setLineState(bool lineState)
{
   mLineState = lineState;
}

bool QueryOptions::getLineState() const
{
   return mLineState;
}

void QueryOptions::setLineStyle(LineStyle lineStyle)
{
   mLineStyle = lineStyle;
}

LineStyle QueryOptions::getLineStyle() const
{
   return mLineStyle;
}

void QueryOptions::setLineWidth(double lineWidth)
{
   mLineWidth = lineWidth;
}

double QueryOptions::getLineWidth() const
{
   return mLineWidth;
}

void QueryOptions::setLineColor(const ColorType &lineColor)
{
   mLineColor = lineColor;
}

const ColorType &QueryOptions::getLineColor() const
{
   return mLineColor;
}

void QueryOptions::setLineScaled(bool lineScaled)
{
   mLineScaled = lineScaled;
}

bool QueryOptions::getLineScaled() const
{
   return mLineScaled;
}

void QueryOptions::setFillColor(const ColorType &fillColor)
{
   mFillColor = fillColor;
}

const ColorType &QueryOptions::getFillColor() const
{
   return mFillColor;
}

void QueryOptions::setFillStyle(FillStyle fillStyle)
{
   mFillStyle = fillStyle;
}

FillStyle QueryOptions::getFillStyle() const
{
   return mFillStyle;
}

void QueryOptions::setHatchStyle(SymbolType hatchStyle)
{
   mHatchStyle = hatchStyle;
}

SymbolType QueryOptions::getHatchStyle() const
{
   return mHatchStyle;
}
