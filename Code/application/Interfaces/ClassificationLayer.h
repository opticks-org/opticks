/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CLASSIFICATIONLAYER_H
#define CLASSIFICATIONLAYER_H

#include "AnnotationLayer.h"
#include "ColorType.h"

class Font;

/**
 *  Provides access to display properties for the classification layer
 *  in a product view.
 *
 *  The classification layer is a special kind of graphic layer that
 *  contains two text objects.  The layer is used in a product view to
 *  display the overall classification markings for the product.  Only
 *  the font, color, and position of the text objects can be changed.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setClassificationFont() and
 *    setClassificationColor().
 *  - Everything else documented in AnnotationLayer.
 *
 *  @see     AnnotationLayer, ProductView
 */
class ClassificationLayer : public AnnotationLayer
{
public:
   /**
    *  Emitted when the font is changed with boost::any<const Font&> containing
    *  the new classification text font.
    */
   SIGNAL_METHOD(ClassificationLayer, FontChanged)

   /**
    *  Emitted when the color is changed with boost::any<ColorType> containing
    *  the new classification text color.
    */
   SIGNAL_METHOD(ClassificationLayer, ColorChanged)

   /**
    *  Sets the classification text font.
    *
    *  @param   classificationFont
    *           The new classification text font.
    *
    *  @notify  This method will notify signalFontChanged() with
    *           boost::any<const Font&> containing the new classification text
    *           font.
    */
   virtual void setClassificationFont(const Font& classificationFont) = 0;

   /**
    *  Returns the classification text font.
    *
    *  @return  The current classification text font.
    */
   virtual const Font& getClassificationFont() const = 0;

   /**
    *  Sets the classification text color.
    *
    *  @param   classificationColor
    *           The new classification text color.  This method does nothing if
    *           \e classificationColor is invalid.
    *
    *  @notify  This method will notify signalColorChanged() with
    *           boost::any<ColorType> containing the new classification text
    *           color.
    *
    *  @see     ColorType::isValid()
    */
   virtual void setClassificationColor(ColorType classificationColor) = 0;

   /**
    *  Returns the text color of the classification text.
    *
    *  @return  The classification text color.
    */
   virtual ColorType getClassificationColor() const = 0;

protected:
   /**
    * A plug-in cannot create this object, it can only retrieve an already existing
    * object from ProductView::getClassificationLayer.  The ProductView will manage
    * any instances of this object.
    */
   virtual ~ClassificationLayer() {}
};

#endif
