/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMCOLORBUTTON_H
#define CUSTOMCOLORBUTTON_H

#include "ColorType.h"

#include <QtGui/QColor>
#include <QtGui/QToolButton>

class ColorMenu;

/**
 *  A push button to get a selected color from the user.
 *
 *  The custom color button allows users to select a color and displays a
 *  pixmap of the currently selected color.  Optional text can be displayed
 *  next to the color using the QPushButton::setText() method.
 *
 *  When the user clicks the button, a common color dialog is invoked in which
 *  the user can select a new color.  By calling usePopupGrid(), this behavior
 *  can change to invoke a popup color grid and a Custom menu command.  The
 *  user can then select a color from the grid or select the Custom command to
 *  invoke the common color dialog.  When a popup color grid is used, a down
 *  arrow appears on the right side of the button.
 *
 *  Regardless of the behavior when the button is clicked, the pixmap
 *  automatically updates when a new color is selected.
 */
class CustomColorButton : public QToolButton
{
   Q_OBJECT

public:
   /**
    *  Creates a new color button with no text a default color of Qt::black.
    *
    *  @param   parent
    *           The parent widget.
    */
   CustomColorButton(QWidget* parent = 0);

   /**
    *  Creates a new color button with text \e strText and a default color of
    *  Qt::black.
    *
    *  @param   strText
    *           The text to appear on the button.
    *  @param   parent
    *           The parent widget.
    */
   CustomColorButton(const QString& strText, QWidget* parent = 0);

   /**
    *  Creates a new color button with text \e strText and color \e color.
    *
    *  @param   strText
    *           The text to appear on the button.
    *  @param   color
    *           The initial color for the button.
    *  @param   parent
    *           The parent widget.
    */
   CustomColorButton(const QString& strText, const QColor& color, QWidget* parent = 0);

   /**
    *  Destroys the color button.
    */
   ~CustomColorButton();

   /**
    *  Returns the current button color.
    *
    *  @return  The button color.
    */
   QColor getColor() const;

   /**
    *  Returns the current button color.
    *  Convenience method that converts
    *  the QColor back into a ColorType.
    *
    *  @return  The button color.
    */
   ColorType getColorType() const;

   /**
    *  Queries whether clicking the button invokes a popup color grid.
    *
    *  @return  Returns \b true if clicking the button invokes a popup color
    *           grid. Returns \b false if clicking the button invokes a common
    *           color dialog.
    */
   bool isPopupGridUsed() const;

public slots:
   /**
    *  Sets the button color.
    *
    *  @param   clrNew
    *           The new button color.  Must be a valid color.
    */
   void setColor(const QColor& clrNew);

   /**
    *  Sets the button color.  Convenience method
    *  that convers the ColorType into a QColor.
    *
    *  @param   clrNew
    *           The new button color.  Must be a valid color.
    */
   void setColor(const ColorType& clrNew);

   /**
    *  Toggles the behavior when the button is clicked.
    *
    *  @param   bGrid
    *           Pass in a value of \b true to invoke a popup color grid when
    *           the button is clicked.  Pass in a value of \b false to invoke
    *           a common color dialog when the button is clicked.
    */
   void usePopupGrid(bool bGrid);

signals:
   /**
    *  This signal is emitted when the button color changes.
    *
    *  The color changes when the user selects a new color or when the color is set
    *  programmatically.
    *
    *  @param   clrNew
    *           The new button color.
    */
   void colorChanged(const QColor& clrNew);

private:
   QColor mColor;
   ColorMenu* mpMenu;

private:
   void initialize();

private slots:
   void initializeColorMenu();
};

#endif
