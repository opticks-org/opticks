/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FONT_H
#define FONT_H

#include <string>

class QFont;

/**
 *  Provides a font for widget text.
 *
 *  This class provides basic capabilities to set the display format of text in
 *  a widget.  The following settings can be set in the font:
 *  - Face
 *  - %Point size
 *  - Bold
 *  - Italic
 *  - Underline
 *
 *  Additional properties can be set on the font by creating a Qt font object
 *  calling setQFont().  The font's properties can also be returned as a
 *  QFont by calling getQFont() to provide flexibility for connecting to other
 *  widgets.
 */
class Font
{
public:
   /**
    *  Sets the font name.
    *
    *  @param   face
    *           The font name, which should be one of the valid system fonts.
    *           This method does nothing if an empty string is passed in.
    */
   virtual void setFace(const std::string& face) = 0;

   /**
    *  Returns the font name.
    *
    *  @return  The font name.
    */
   virtual std::string getFace() const = 0;

   /**
    *  Sets the point size for the font.
    *
    *  @param   iSize
    *           The font point size.
    */
   virtual void setPointSize(int iSize) = 0;

   /**
    *  Returns the font point size.
    *
    *  @return  The font point size.
    */
   virtual int getPointSize() const = 0;

   /**
    *  Sets the bold state for the font.
    *
    *  @param   bBold
    *           Pass in \b true to display bold text or \b false to display
    *           normal text.
    */
   virtual void setBold(bool bBold) = 0;

   /**
    *  Returns the bold state of the font.
    *
    *  @return  Returns \b true if the font displays bold text or \b false if
    *           the font displays normal text.
    */
   virtual bool getBold() const = 0;

   /**
    *  Sets the italics state for the font.
    *
    *  @param   bItalic
    *           Pass in \b true to display italicized text or \b false to
    *           display normal text.
    */
   virtual void setItalic(bool bItalic) = 0;

   /**
    *  Returns the italics state of the font.
    *
    *  @return  Returns \b true if the font displays italicized text or
    *           \b false if the font displays normal text.
    */
   virtual bool getItalic() const = 0;

   /**
    *  Sets the underline state for the font.
    *
    *  @param   bUnderline
    *           Pass in \b true to display underlined text or \b false to
    *           display normal text.
    */
   virtual void setUnderline(bool bUnderline) = 0;

   /**
    *  Returns the underline state of a text object.
    *
    *  @return  Returns \b true if the font displays underlined text or
    *           \b false if the font displays normal text.
    */
   virtual bool getUnderline() const = 0;

   /**
    *  Sets the values of this font to those of a given Qt font.
    *
    *  @param   font
    *           The Qt font object from which to set this font's values.  All
    *           values in the font are used to display text, not just the face,
    *           point size, bold, italic, and underline.
    */
   virtual void setQFont(const QFont& font) = 0;

   /**
    *  Returns the values in the font as a Qt font.
    *
    *  @return  Returns a reference to a Qt font.  Modifying the values in the
    *           QFont will also modify the values in this font.
    */
   virtual QFont& getQFont() = 0;

   /**
    *  Returns read-only access to the values in the font as a Qt font.
    *
    *  @return  Returns a const reference to a Qt font.  To modify the values
    *           in the font call the non-const version of getQFont() or call
    *           one of the specific set methods to set a certain value.
    */
   virtual const QFont& getQFont() const = 0;

protected:
   /**
    *  This object should be destroyed by calling ObjectFactory::destroyObject().
    */
   virtual ~Font() {}
};

#endif
