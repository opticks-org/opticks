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
 *  button, an ellipsis is drawn based on the elide mode returned by
 *  getElideMode().  To allow the button to resize smaller than the text string,
 *  a minimum size must be set (e.g. QWidget::setMimimumWidth()).
 *
 *  @see        ElidedLabel
 */
class ElidedButton : public QPushButton
{
   Q_OBJECT

public:
   /**
    *  Creates a new elided push button with a default elide mode of
    *  Qt::ElideLeft.
    *
    *  @param   pParent
    *           The parent widget.
    *
    *  @see     setElideMode()
    */
   ElidedButton(QWidget* pParent = NULL);

   /**
    *  Creates a new elided push button with initial text and a default elide
    *  mode of Qt::ElideLeft.
    *
    *  @param   text
    *           The text string to display in the button.
    *  @param   pParent
    *           The parent widget.
    *
    *  @see     setElideMode()
    */
   ElidedButton(const QString& text, QWidget* pParent = NULL);

   /**
    *  Destroys the push button.
    */
   virtual ~ElidedButton();

   /**
    *  Sets the position in the text of the ellipsis that is drawn when the
    *  text is longer the button width.
    *
    *  @param   mode
    *           The new ellipsis position within the text.
    */
   void setElideMode(Qt::TextElideMode mode);

   /**
    *  Returns the position in the text of the ellipsis that is drawn when the
    *  text is longer the button width.
    *
    *  @return  The current ellipsis position within the text.
    */
   Qt::TextElideMode getElideMode() const;

protected:
   /**
    *  Draws the push button and its text.
    *
    *  The default implementation draws an ellipsis based on the elide mode
    *  returned by getElideMode() if the text string is longer than the button
    *  width.  Otherwise, the QPushButton base class implementation is called.
    *
    *  @param   pEvent
    *           The paint event.
    */
   virtual void paintEvent(QPaintEvent* pEvent);

private:
   Qt::TextElideMode mElideMode;
   QString mPreviousText;
   QRect mPreviousRect;
   QString mElidedText;
};

#endif
