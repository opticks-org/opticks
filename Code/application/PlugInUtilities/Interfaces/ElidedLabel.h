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
 *  label, an ellipsis is drawn on the left side of the text.  To allow the
 *  label to resize smaller than the text string, a minimum size must be set
 *  (e.g. QWidget::setMimimumWidth()).
 *
 *  @see        ElidedButton
 */
class ElidedLabel : public QLabel
{
   Q_OBJECT

public:
   /**
    *  Creates a new elided label.
    *
    *  @param   pParent
    *           The parent widget.
    */
   ElidedLabel(QWidget* pParent = NULL);

   /**
    *  Creates a new elided label.
    *
    *  @param   text
    *           The text string to display in the label.
    *  @param   pParent
    *           The parent widget.
    */
   ElidedLabel(const QString& text, QWidget* pParent = NULL);

   /**
    *  Destroys the label.
    */
   virtual ~ElidedLabel();

protected:
   /**
    *  Draws the label and its text.
    *
    *  The default implementation draws an ellipsis on the left side of the
    *  text if the text string is longer than the label width.  Otherwise, the
    *  QLabel base class implementation is called.
    *
    *  @param   pEvent
    *           The paint event.
    */
   virtual void paintEvent(QPaintEvent* pEvent);
};

#endif
