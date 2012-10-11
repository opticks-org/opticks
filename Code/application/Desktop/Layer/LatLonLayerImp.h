/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONLAYERIMP_H
#define LATLONLAYERIMP_H

#include <QtGui/QColor>
#include <QtGui/QFont>

#include "ColorType.h"
#include "EnumWrapper.h"
#include "FontImp.h"
#include "LayerImp.h"
#include "LocationType.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class DataElement;

class LatLonLayerImp : public LayerImp
{
   Q_OBJECT

public:
   static bool isKindOfLayer(const std::string& className);
   static void getLayerTypes(std::vector<std::string>& classList);

   LatLonLayerImp(const std::string& id, const std::string& layerName, DataElement* pElement);
   ~LatLonLayerImp();

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   void georeferenceModified(Subject &subject, const std::string &signal, const boost::any &data);

   LatLonLayerImp& operator= (const LatLonLayerImp& latLonLayer);

   LayerType getLayerType() const;

   std::vector<ColorType> getColors() const;
   void draw();
   bool getExtents(double& x1, double& y1, double& x4, double& y4);

   /**
    * Gets the color of the layer.
    *
    * @return The color of the layer.
    */
   QColor getColor() const;

   QFont getFont() const;

   /**
    * Gets the current grid style.
    *
    * @return The current style of the layer.
    */
   LatLonStyle getStyle() const;

   /**
    * Gets the current line width.
    *
    * @return The current line width of the layer.
    */
   unsigned int getWidth() const;

   /**
    * Gets the current X and Y tick spacing. The tick spacing can be auto
    * computed or specified by the user. This method returns the correct
    * value in either case.
    *
    * @return The tick spacing as a LocationType.
    */
   LocationType getTickSpacing(bool bDrawing = false) const;

   /**
    * Gets the current state of auto computed tick spacing
    *
    * @return Returns true if auto tick spacing is turned on, or false otherwise
    */
   bool getAutoTickSpacing() const;

   GeocoordType getGeocoordType() const;

   /**
    * Gets the current formatting that affects the display of latitude/longitude points
    *
    * @return Returns the current latitude/longitude formatting of the layer
    */
   DmsFormatType getLatLonFormat() const;

   bool getExtrapolation() const;

   void setBorderDirty(bool bDirty);
   void setTickSpacingDirty(bool bDirty);

   static QFont getDefaultFont();

   bool toXml(XMLWriter* pXml) const;
   bool fromXml(DOMNode* pDocument, unsigned int version);

public slots:
   /**
    * Sets the color of the layer. Does not trigger a redraw.
    *
    * @param newColor
    *     The new color for the layer.
    */
   void setColor(const QColor& newColor);

   /**
    * Sets the font to use when drawing labels.  Does not trigger a redraw.
    *
    * @param font
    *     The font to use for the text labels.
    */
   void setFont(const QFont& font);

   /**
    * Sets the current grid style. Does not trigger a redraw.
    *
    * @param newStyle
    *     An enumerated value specifying which style to apply.
    */
   void setStyle(const LatLonStyle& newStyle);

   /**
    * Sets the current line width. Does not trigger a redraw.
    *
    * @param width
    *     An width in pixels to apply to the line width.
    */
   void setWidth(unsigned int width);

   /**
    * Sets the tick spacing in X and Y to specified values. This disables
    * auto tick spacing.
    *
    * @param spacing
    *     The tick spacing to use in X and Y as a LocationType
    */
   void setTickSpacing(const LocationType& spacing);

   /**
    * Turns on or off auto computed tick spacing.
    *
    * @param compute
    *     A bool specifying whether to autocompute (true) or not (false).
    */
   void setAutoTickSpacing(bool compute);

   void setGeocoordType(const GeocoordType& eGeocoord);

   void setLatLonFormat(const DmsFormatType& newFormat);

   void setExtrapolation(bool bExtrapolate);

   void reset();

signals:
   void colorChanged(const QColor& clrLines);
   void fontChanged(const QFont& font);
   void styleChanged(const LatLonStyle& eStyle);
   void widthChanged(unsigned int width);
   void tickSpacingChanged(const LocationType& spacing);
   void autoTickSpacingChanged(bool bAutoSpacing);
   void extrapolationChanged(bool bExtrapolate);
   void coordTypeChanged(const GeocoordType& eGeocoord);
   void formatChanged(const DmsFormatType& eType);

protected:
   /**
     * Recomputes the values in mBorder if necessary and clears mBorderDirty.
     */
   void computeBorder();

   /**
     * Recomputes mComputedTickSpacing if necessary and clears mComputedTickSpacingDirty.
     */
   void computeTickSpacing(bool bDrawing = false);

   void setBoundingBox(const std::vector<LocationType> &boundingBox);
   const FontImp& getFontImp() const;

private:
   LatLonLayerImp(const LatLonLayerImp& rhs);

   GeocoordType mGeocoordType;
   bool mbExtrapolate;                 // the current extrapolation setting
   DmsFormatType mFormat;
   LatLonStyle mStyle;                 // the current drawing style
   QColor mColor;                      // the current drawing color
   unsigned int mWidth;                // the current drawing width
   LocationType mCubeSize;             // the size of the cube in pixels
   LocationType mMaxCoord;             // the maximum geocoord value found on the pixel border
   LocationType mMinCoord;             // the minimum geocoord value found on the pixel border

   bool mComputeTickSpacing;           // state of auto tick spacing
   LocationType mTickSpacing;          // currently set tick spacing
   LocationType mUserTickSpacing;      // tick spacing passed in to setTickSpacing()
   LocationType mComputedTickSpacing;  // currently computed tick spacing
   bool mComputedTickSpacingDirty;     // whether computed tick spacing needs updating
   bool mBorderDirty;                  // whether the border needs updating
   std::vector<LocationType> mBoundingBox;

   FontImp mFont;

   enum BorderTypeEnum {LEFT_BORDER, RIGHT_BORDER, BOTTOM_BORDER, TOP_BORDER};

   /**
    * @EnumWrapper LatLonLayerImp::BorderTypeEnum.
    */
   typedef EnumWrapper<BorderTypeEnum> BorderType;
   void drawLabel(const LocationType& location, const LocationType& textOffset, 
      LocationType geoCoord, bool lat, const BorderType& borderType, const double modelMatrix[16],
      const double projectionMatrix[16], const int viewPort[4], bool bProduct);
   LocationType convertPointToLatLon(const GeocoordType& type, const LocationType& point);
};

#define LATLONLAYERADAPTEREXTENSION_CLASSES \
   LAYERADAPTEREXTENSION_CLASSES

#define LATLONLAYERADAPTER_METHODS(impClass) \
   LAYERADAPTER_METHODS(impClass) \
   void setColor(const ColorType& latLonColor) \
   { \
      QColor clrLatLon; \
      if (latLonColor.isValid() == true) \
      { \
         clrLatLon.setRgb(latLonColor.mRed, latLonColor.mGreen, latLonColor.mBlue); \
      } \
      impClass::setColor(clrLatLon); \
   } \
   ColorType getColor() const \
   { \
      ColorType latLonColor; \
      QColor clrLatLon = impClass::getColor(); \
      if (clrLatLon.isValid() == true) \
      { \
         latLonColor.mRed = clrLatLon.red(); \
         latLonColor.mGreen = clrLatLon.green(); \
         latLonColor.mBlue = clrLatLon.blue(); \
      } \
      return latLonColor; \
   } \
   void setStyle(const LatLonStyle& eStyle) \
   { \
      impClass::setStyle(eStyle); \
   } \
   LatLonStyle getStyle() const \
   { \
      return impClass::getStyle(); \
   } \
   void setWidth(unsigned int uiWidth) \
   { \
      impClass::setWidth(uiWidth); \
   } \
   unsigned int getWidth() const \
   { \
      return impClass::getWidth(); \
   } \
   void setTickSpacing(const LocationType& spacing) \
   { \
      impClass::setTickSpacing(spacing); \
   } \
   LocationType getTickSpacing() const \
   { \
      return impClass::getTickSpacing(); \
   } \
   void setAutoTickSpacing(bool bAutoSpacing) \
   { \
      impClass::setAutoTickSpacing(bAutoSpacing); \
   } \
   bool getAutoTickSpacing() const \
   { \
      return impClass::getAutoTickSpacing(); \
   } \
   void setFont(const Font& font) \
   { \
      impClass::setFont(font.getQFont()); \
   } \
   const Font& getFont() const \
   { \
      return impClass::getFontImp(); \
   } \
   void setGeocoordType(const GeocoordType& system) \
   { \
      impClass::setGeocoordType(system); \
   } \
   GeocoordType getGeocoordType() const \
   { \
      return impClass::getGeocoordType(); \
   } \
   void setLatLonFormat(const DmsFormatType& eFormat) \
   { \
      impClass::setLatLonFormat(eFormat); \
   } \
   DmsFormatType getLatLonFormat() const \
   { \
      return impClass::getLatLonFormat(); \
   } \
   void setExtrapolation(bool bExtrapolate) \
   { \
      impClass::setExtrapolation(bExtrapolate); \
   } \
   bool getExtrapolation() const \
   { \
      return impClass::getExtrapolation(); \
   }

#endif
