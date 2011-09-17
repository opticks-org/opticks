/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QtGui/QLabel>

/**
 *  A specialized QLabel that displays an ellipsis when its text is larger than
 *  the label width.
 *
 *  If the text string displayed in the label is longer than the width of the
 *  label, an ellipsis is drawn based on the elide mode returned by
 *  getElideMode().  To allow the label to resize smaller than the text string,
 *  a minimum size must be set (e.g. QWidget::setMimimumWidth()).
 *
 *  @warning    With an ElidedLabel, when determining whether the text should be
 *              elided, the QLabel word wrap property is ignored.
 *
 *  @see        ElidedButton
 */
class ElidedLabel : public QLabel
{
   Q_OBJECT

public:
   /**
    *  Creates a new elided label with a default elide mode of Qt::ElideLeft.
    *
    *  @param   pParent
    *           The parent widget.
    *
    *  @see     setElideMode()
    */
   ElidedLabel(QWidget* pParent = NULL);

   /**
    *  Creates a new elided label with initial text and a default elide mode of
    *  Qt::ElideLeft.
    *
    *  @param   text
    *           The text string to display in the label.
    *  @param   pParent
    *           The parent widget.
    *
    *  @see     setElideMode()
    */
   ElidedLabel(const QString& text, QWidget* pParent = NULL);

   /**
    *  Destroys the label.
    */
   virtual ~ElidedLabel();

   /**
    *  Sets the position in the text of the ellipsis that is drawn when the
    *  text is longer the label width.
    *
    *  @param   mode
    *           The new ellipsis position within the text.
    */
   void setElideMode(Qt::TextElideMode mode);

   /**
    *  Returns the position in the text of the ellipsis that is drawn when the
    *  text is longer the label width.
    *
    *  @return  The current ellipsis position within the text.
    */
   Qt::TextElideMode getElideMode() const;

protected:
   /**
    *  Draws the label and its text.
    *
    *  The default implementation draws an ellipsis based on the elide mode
    *  returned by getElideMode() if the text string is longer than the label
    *  width.  Otherwise, the QLabel base class implementation is called.
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
