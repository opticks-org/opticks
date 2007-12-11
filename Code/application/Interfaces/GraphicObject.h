/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICOBJECT_H
#define GRAPHICOBJECT_H

#include "ColorType.h"
#include "LocationType.h"
#include "Serializable.h"
#include "SessionItem.h"
#include "Subject.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class BitMask;
class CgmObject;
class Font;
class View;

/**
 *  Provides access to display properties for graphic objects.
 *
 *  Graphic layers are comprised of graphic objects.  The
 *  GraphicObject class provides access to set the display
 *  properties of each object.  Not all methods apply to all object
 *  types.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The Subject::signalModified() notification is sent when any property of
 *    the object is modified.
 *  - All notifications documented in Subject.
 *
 *  @see     GraphicLayer, GraphicObjectType
 */
class GraphicObject : public SessionItem, public Subject, public Serializable
{
public:
   /**
    *  Returns the type of the object.
    *
    *  @return  The object type.
    */
   virtual GraphicObjectType getGraphicObjectType() const = 0;

   /**
    *  Sets the name of the object.
    *
    *  @param   name
    *           The new name for the graphic object.
    */
   virtual void setName(const std::string& name) = 0;

   /**
    *  Converts the object to a CGM object containing CGM metadata.
    *
    *  @return  A pointer to the CGM object.  NULL is returned if an
    *           error occurs or if the object cannot be converted.
    */
   virtual CgmObject* convertToCgm() = 0;

   /**
    *  Returns a bit mask containing selected pixels representing the
    *  object's position.
    *
    *  The bit mask represents those pixels that define the object's
    *  location, with rotation taken into account.
    *
    *  @return  A pointer to a BitMask object.
    */
   virtual const BitMask* getPixels() = 0;

   /**
    *  Sets the object bounding box location.
    *
    *  @param   llCorner
    *           The new lower left corner position for the object in pixel
    *           coordinates.
    *  @param   urCorner
    *           The new upper right corner position for the object in pixel
    *           coordinates.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setBoundingBox(LocationType llCorner, LocationType urCorner) = 0;

   /**
    *  Returns the lower left corner position of the object's bounding box.
    *
    *  @return  The lower left corner position in pixel coordinates.
    */
   virtual LocationType getLlCorner() const = 0;

   /**
    *  Returns the upper right corner position of the object's bounding box.
    *
    *  @return  The upper right corner position in pixel coordinates.
    */
   virtual LocationType getUrCorner() const = 0;

   /**
    *  Sets the object rotation value.
    *
    *  The rotation value indicates the amount in degrees counterclockwise
    *  that the object is rotated.
    *
    *  @param   dAngle
    *           The new rotation angle.  Valid values range from 0 to 360.
    *           Values outside this range are automatically adjusted to fall
    *           within the range.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setRotation(double dAngle) = 0;

   /**
    *  Returns the object rotation value
    *
    *  @return  The object rotation value.  Valid values range from 0 to 360.
    */
   virtual double getRotation() const = 0;

   // Line

   /**
    *  Sets the line state for an object.
    *
    *  @param   bLine
    *           TRUE if the object should be displayed, FALSE if the object
    *           line should not be displayed.  This pertains primarily to
    *           filled objects where line state indicates whether the object
    *           border is visible.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setLineState(bool bLine) = 0;

   /**
    *  Returns the line state of a filled object.
    *
    *  @param   pSuccess
    *           A valid bool pointer will be populated with the success of the
    *           method.  FALSE is populated if an error occurs.
    *
    *  @return  TRUE if the object line is displayed, otherwise FALSE.  This pertains
    *           primarily to filled objects where line state indicates whether
    *           the object border is visible.
    */
   virtual bool getLineState(bool* pSuccess = NULL) const = 0;

   /**
    *  Sets the line color for an object.
    *
    *  @param   lineColor
    *           The new line color.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setLineColor(ColorType lineColor) = 0;

   /**
    *  Returns the line color of an object.
    *
    *  @return  The line color.  An invalid color is returned if an
    *           error occurs.
    *
    *  @see     ColorType::isValid()
    */
   virtual ColorType getLineColor() const = 0;

   /**
    *  Sets the line width for an object.
    *
    *  @param   dWidth
    *           The line width.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setLineWidth(double dWidth) = 0;

   /**
    *  Returns the line width of an object.
    *
    *  @return  The line width.  An invalid value of -1 is returned if
    *           an error occurs.
    */
   virtual double getLineWidth() const = 0;

   /**
    *  Sets the line style for an object.
    *
    *  @param   eLine
    *           The line style.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if an error occurs.
    */
   virtual bool setLineStyle(LineStyle eLine) = 0;

   /**
    *  Returns the line style of an object.
    *
    *  @return  The line style.  An invalid value of -1 is returned if
    *           an error occurs.
    */
   virtual LineStyle getLineStyle() const = 0;

   /**
    * Set the whether the lines should be scaled with the zoom level.
    *
    * @param scaled
    *        True if the line widths should be scaled, false otherwise.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   virtual bool setLineScaled(bool scaled) = 0;

   /**
    * Get the whether the lines should be scaled with the zoom level.
    *
    * @return True if the line widths should be scaled, false otherwise.
    */
   virtual bool getLineScaled() const = 0;

   // Arc

   /**
    *  Sets the arc region for an arc object.
    *
    *  @param   eRegion
    *           The new region type.
    *
    *  @return  TRUE if the region was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an arc object.
    */
   virtual bool setArcRegion(ArcRegion eRegion) = 0;

   /**
    *  Returns the arc region of an arc object.
    *
    *  @return  The region type.  An invalid value of -1 is returned if the
    *           object is not an arc object.
    */
   virtual ArcRegion getArcRegion() const = 0;

   /**
    *  Sets both the start and stop angles for an arc object.
    *
    *  A arc object is composed of a portion of an elliptical arc.  The
    *  ellipse is divided into 360 sections, and the start and stop angles
    *  represent the location on the ellipse where the arc is defined.
    *
    *  @param   dStart
    *           The start angle.  Values are automatically adjusted to be
    *           between 0 and 360.
    *  @param   dStop
    *           The stop angle.  Values are automatically adjusted to be
    *           between 0 and 360.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an arc
    *           object.
    */
   virtual bool setAngles(double dStart, double dStop) = 0;

   /**
    *  Sets the start angle for an arc object.
    *
    *  @param   dStart
    *           The start angle.  Values are automatically adjusted to be
    *           between 0 and 360.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an arc
    *           object.
    *
    *  @see     setAngles()
    */
   virtual bool setStartAngle(double dStart) = 0;

   /**
    *  Sets the stop angle for an arc object.
    *
    *  @param   dStop
    *           The stop angle.  Values are automatically adjusted to be
    *           between 0 and 360.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an arc
    *           object.
    *
    *  @see     setAngles()
    */
   virtual bool setStopAngle(double dStop) = 0;

   /**
    *  Returns the start angle of an arc object.
    *
    *  @return  The start angle.  An invalid value of -1 is returned if
    *           the object is not an arc object.
    *
    *  @see     setStartAngle()
    */
   virtual double getStartAngle() const = 0;

   /**
    *  Returns the stop angle of an arc object.
    *
    *  @return  The stop angle.  An invalid value of -1 is returned if
    *           the object is not an arc object.
    *
    *  @see     setStopAngle()
    */
   virtual double getStopAngle() const = 0;

   // Filled object

   /**
    *  Sets the fill state for a filled object.
    *
    *  This method is similar to setFillStyle() method, but simply specifies
    *  whether the object have any fill or should be empty.
    *
    *  @param   bFill
    *           TRUE if the object should be filled, FALSE if the object
    *           should have an empty fill.  Passing in TRUE fills the object
    *           according to it fill style.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a filled object.
    *
    *  @see     setFillStyle()
    */
   virtual bool setFillState(bool bFill) = 0;

   /**
    *  Returns the fill state of a filled object.
    *
    *  This method is similar to getFillStyle() method, but simply returns
    *  whether the object has any fill or is empty.
    *
    *  @param   pSuccess
    *           A valid bool pointer will be populated with the success of the
    *           method.  FALSE is populated if the object is not a filled object.
    *
    *  @return  TRUE if the object is filled, FALSE if the object fill is empty.
    *
    *  @see     getFillStyle()
    */
   virtual bool getFillState(bool* pSuccess = NULL) const = 0;

   /**
    *  Sets the background fill color for a filled object.
    *
    *  @param   fillColor
    *           The new fill color.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a filled object.
    */
   virtual bool setFillColor(ColorType fillColor) = 0;

   /**
    *  Returns the background fill color of a filled object.
    *
    *  @return  The fill color.  An invalid color is returned if the object
    *           is not a filled object.
    *
    *  @see     ColorType::isValid()
    */
   virtual ColorType getFillColor() const = 0;

   /**
    *  Sets the fill style for a filled object.
    *
    *  @param   eFill
    *           The fill style.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a filled
    *           object.
    */
   virtual bool setFillStyle(FillStyle eFill) = 0;

   /**
    *  Returns the fill style of a filled object.
    *
    *  @return  The fill style.  An invalid value of -1 is returned if
    *           the object is not a filled object.
    */
   virtual FillStyle getFillStyle() const = 0;

   /**
    *  Sets the hatch style for a filled object.
    *
    *  The hatch style corresponds with the hatch fill style.  If the 
    *  fill style is not set to hatch, setting the hatch style will
    *  still succeed, but the effect will not be seen in the object
    *  until the fill style is set to hatch.
    *
    *  @param   eHatch
    *           The hatch style.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a filled
    *           object.
    */
   virtual bool setHatchStyle(SymbolType eHatch) = 0;

   /**
    *  Returns the hatch style of a filled object.
    *
    *  @return  The hatch style.  An invalid value of -1 is returned if
    *           the object is not a filled object.
    */
   virtual SymbolType getHatchStyle() const = 0;

   // Triangle

   /**
    *  Sets the apex value for a triangle object.
    *
    *  A triangle object always has at least one size bordering on its
    *  bounding box.  The apex value specifies the position of the third
    *  point along the opposite side of the bounding box.
    *
    *  @param   dApex
    *           The apex value.  Valid values range from 0.0 to 1.0.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a triangle
    *           object.
    */
   virtual bool setApex(double dApex) = 0;

   /**
    *  Returns the apex value of a triangle object.
    *
    *  @return  The apex value.  An invalid value of -1 is returned if
    *           the object is not a triangle object.
    */
   virtual double getApex() const = 0;

   // Text

   /**
    *  Sets the text for a text object.
    *
    *  @param   objectText
    *           The text string.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a text object.
    */
   virtual bool setText(const std::string& objectText) = 0;

   /**
    *  Returns the text of a text object.
    *
    *  @return  The text string.  An empty string is returned if the object is not a
    *           text object.
    */
   virtual std::string getText() const = 0;

   /**
    *  Sets the font for object text.
    *
    *  @param   pTextFont
    *           The new object text font.  This method does nothing if \b NULL
    *           is passed in.
    *
    *  @return  Returns \b true if the font was set successfully on the object.
    *           Returns \b false if this object does not support text fonts.
    */
   virtual bool setFont(const Font* pTextFont) = 0;

   /**
    *  Returns the current object text font.
    *
    *  @return  The current object text font.  \b NULL is returned if the
    *           object does not support text fonts.
    */
   virtual const Font* getFont() const = 0;

   /**
    *  Sets the text color for a text object.
    *
    *  @param   textColor
    *           The new text color.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a text object.
    */
   virtual bool setTextColor(ColorType textColor) = 0;

   /**
    *  Returns the text color of a text object.
    *
    *  @return  The text color.  An invalid color is returned if the object
    *           is not a text object.
    *
    *  @see     ColorType::isValid()
    */
   virtual ColorType getTextColor() const = 0;

   /**
    *  Sets the alignment for text within a text object.
    *
    *  @param   iAlignment
    *           The new text alignment.  Valid values are defined to be
    *           the same values as the Qt::AlignmentFlags enumeration.
    *
    *  @return  True if the value was set successfully, otherwise false.
    *           False is also returned if the object is not a text object.
    */
   virtual bool setTextAlignment(int iAlignment) = 0;

   /**
    *  Returns the text alignment in a text object.
    *
    *  @return  The text alignment.  The returned value is the same as the
    *           valid Qt::AlignmentFlags enumeration values.
    */
   virtual int getTextAlignment() const = 0;

   // Scale

   /**
    *  Sets the scale value for arrow, image and text objects.
    *
    *  Arrow, image, and text objects have a default scale value of 1.0.  The
    *  for any other value, the size of the object displayed in the view is
    *  multiplied by the scale value.
    *
    *  @param   dScale
    *           The new scale value.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not one of the valid
    *           objects.
    */
   virtual bool setScale(double dScale) = 0;

   /**
    *  Returns the scale value of arrow, image and text objects.
    *
    *  @return  The scale value.  An invalid value of -1 is returned if the
    *           object is not one of the valid objects.
    */
   virtual double getScale() const = 0;

   // Image

   /**
    *  Sets the image for a file image object from a file.
    *
    *  @param   pFilename
    *           The name of the file containing the image to display in the
    *           object.  Cannot be NULL.
    *
    *  @return  TRUE if the image was set successfully from the file, otherwise
    *           FALSE.  FALSE is also returned if the object is not a file
    *           image object.
    *
    *  @see     setObjectImage(), setAlpha()
    */
   virtual bool setImageFile(const char *pFilename) = 0;

   /**
    *  Sets the image for an image object.
    *
    *  @param   pData
    *           A pointer to the image data.  Cannot be NULL;
    *  @param   width
    *           The width of the image.
    *  @param   height
    *           The height of the image.
    *  @param   transparent
    *           Identifies the color of pixels that should be displayed
    *           transparently.  An invalid color indicated that there are no
    *           transparent pixels.  This value is separate from the object's
    *           alpha value.
    *
    *  @return  TRUE if the image was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an image object.
    *
    *  @see     setImageFile(), setAlpha()
    */
   virtual bool setObjectImage(const unsigned int* pData, int width, int height,
      ColorType transparent = ColorType()) = 0;

   /**
    *  Returns the image of an image object.
    *
    *  @param   width
    *           Populated with the width of the image.
    *  @param   height
    *           Populated with the height of the image.
    *  @param   transparent
    *           Populated with color that is used to identify transparent
    *           pixels.
    *
    *  @return  A pointer to the image data.  NULL is returned if an
    *           error occurs.
    */
   virtual const unsigned int* getObjectImage(int &width, int &height, ColorType &transparent) const = 0;

   /**
    *  Returns the alpha value of an image object.
    *
    *  @return  The alpha value.  An invalid value of -1 is returned if the
    *           object is not an image object.
    */
   virtual double getAlpha() const = 0;

   /**
    *  Sets the alpha value for an image object.
    *
    *  The alpha value defines the transparency level for the image.  An
    *  image can be converted into a watermark with a relatively low
    *  alpha value.
    *
    *  @param   alpha
    *           The new alpha value.  Valid values range from 0
    *           to 255, with 0 being transparent and 255 being opaque.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not an image object.
    */
   virtual bool setAlpha(double alpha) = 0;

   // View

   /**
    *  Sets the displayed view in a view object.
    *
    *  @param   pView
    *           A pointer to the view to display in the view object.  NULL is
    *           a valid input, where the view object displays a message
    *           indicating that no view is available.
    *
    *  @return  TRUE if the value was set successfully, otherwise FALSE.
    *           FALSE is also returned if the object is not a view object.
    *
    *  @notify  This method will notify ViewObject::signalViewCreated() with
    *           any<View*> and/or ViewObject::signalViewDeleted() with
    *           any<View*> if the view is successfully set in the object.
    */
   virtual bool setObjectView(View* pView) = 0;

   /**
    *  Returns the view displayed in a view object.
    *
    *  @return  A pointer to the view displayed in the view object.  NULL is
    *           returned if the object is not a view object.
    */
   virtual View* getObjectView() const = 0;

   /**
    *  Adds vertices to a polyline or polygon object.
    *
    *  @param   vertices
    *           The vertices to add to the object.
    *
    *  @return  True if the vertices have been successfully added.
    */
   virtual bool addVertices(const std::vector<LocationType>& vertices) = 0;

   /**
    *  Adds geographic vertices to a polyline or polygon object.
    *
    *  @param   geoVertices
    *           The lat/long vertices to add to the object.
    *
    *  @return  True if the vertices have been successfully added.
    */
   virtual bool addGeoVertices(const std::vector<LocationType>& geoVertices) = 0;

   /**
    * Adds a new path to a Polyline or Polygon graphic.
    *
    * This method allows a single Polyline or Polygon to contain
    * multiple discontinuous paths.  For a Polygon, this allows
    * a doughnut shape.  The vertices added between calls to
    * newPath() make a single path.
    *
    * @return True if a new path was successfully added, false
    *         otherwise.  This method will fail if no vertices
    *         have been added since the last call to newPath().
    */
   virtual bool newPath() = 0;

   /**
    * Determine whether the object is a visible shape.
    *
    * This method determines whether the object is a shape, 
    * like RECTANGLE_OBJECT or a tool, like MOVE_OBJECT.
    *
    * @return True if the object is a visible shape, false if
    *         it is a non-visible tool.
    */
   virtual bool isVisible() const = 0;

   /**
    * Set the symbol to draw for each pixel when drawing individual pixels.
    *
    * @param symbol
    *        The new symbol.
    */
   virtual bool setPixelSymbol(SymbolType symbol) = 0;

   /**
    * Get the pixel drawn for individual pixels.
    *
    * @return The symbol drawn.
    */
   virtual SymbolType getPixelSymbol() const = 0;

   /**
    * Set the name of the symbol to be drawn at the points of
    * a MultipointObject.
    *
    * @param symbolName
    *        The name of the symbol to draw
    *
    * @return True if the operation succeeded, false otherwise.
    */
   virtual bool setSymbolName(const std::string &symbolName) = 0;

   /**
    * Get the name of the symbol to be drawn at the points of
    * a MultipointObject.
    *
    * @returns The name of the symbol to draw
    */
   virtual const std::string &getSymbolName() const = 0;

   /**
    * Set the size of the symbol to be drawn at the points of
    * a MultipointObject.
    *
    * @param symbolSize
    *        The size of the symbol to draw.  This is in screen pixels.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   virtual bool setSymbolSize(unsigned int symbolSize) = 0;

   /**
    * Get the size of the symbol to be drawn at the points of
    * a MultipointObject.
    *
    * @return The size of the symbol to draw.  This is in screen pixels.
    */
   virtual unsigned int getSymbolSize() const = 0;

   /** 
    * Set the draw mode when composing bitmasks with this object.
    *
    * @param mode
    *        The new draw mode.
    */
   virtual bool setDrawMode(ModeType mode) = 0;
   
   /**
    * Get the draw mode for composing bitmasks with this object.
    *
    * @return The draw mode.
    */
   virtual ModeType getDrawMode() const = 0;

protected:
   /**
    * This should be destroyed by calling GraphicLayer::removeObject.
    */
   virtual ~GraphicObject() {}
};

#endif
