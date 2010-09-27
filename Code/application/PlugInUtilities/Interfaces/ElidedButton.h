/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ELIDEDBUTTON_H
#define ELIDEDBUTTON_H

#include <QtGui/QPushButton>

/**
 *  A specialized QPushButton that displays an ellipsis when its text is larger
 *  than the button width.
 *
 *  If the text string displayed in the button is longer than the width of the
 *  button, an ellipsis is drawn on the left side of the text.  To allow the
 *  button to resize smaller than the text string, a minimum size must be set
 *  (e.g. QWidget::setMimimumWidth()).
 *
 *  @see        ElidedLabel
 */
class ElidedButton : public QPushButton
{
   Q_OBJECT

public:
   /**
    *  Creates a new elided push button.
    *
    *  @param   pParent
    *           The parent widget.
    */
   ElidedButton(QWidget* pParent = NULL);

   /**
    *  Creates a new elided push button.
    *
    *  @param   text
    *           The text string to display in the button.
    *  @param   pParent
    *           The parent widget.
    */
   ElidedButton(const QString& text, QWidget* pParent = NULL);

   /**
    *  Destroys the push button.
    */
   virtual ~ElidedButton();

protected:
   /**
    *  Draws the push button and its text.
    *
    *  The default implementation draws an ellipsis on the left side of the
    *  text if the text string is longer than the button width.  Otherwise, the
    *  QPushButton base class implementation is called.
    *
    *  @param   pEvent
    *           The paint event.
    */
   virtual void paintEvent(QPaintEvent* pEvent);
};

#endif
