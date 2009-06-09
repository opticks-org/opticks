/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef FLOATINGLABEL_H
#define FLOATINGLABEL_H

#include <QtGui/QLabel>

#include "EnumWrapper.h"

class FloatingLabel : public QLabel
{
   Q_OBJECT

public:
   FloatingLabel(Qt::Orientation eOrientation, QWidget* parent = 0);
   ~FloatingLabel();

   enum TextJustificationEnum { AUTO_JUSTIFIED = 0, LEFT_JUSTIFIED, CENTER_JUSTIFIED, RIGHT_JUSTIFIED };

   /**
    * @EnumWrapper FloatingLabel::TextJustificationEnum.
    */
   typedef EnumWrapper<TextJustificationEnum> TextJustification;

   Qt::Orientation getOrientation() const;
   void setTextPosition(int iPosition);
   int getTextPosition() const;
   void setTextJustification(TextJustification eJustification);
   TextJustification getTextJustification() const;

   QSize sizeHint() const;

signals:
   void positionChanged(int iPosition);
   void justificationChanged(TextJustification eJustification);

protected:
   void paintEvent(QPaintEvent* pEvent);

   int getPositionLimit() const;

private:
   Qt::Orientation meOrientation;
   int miPosition;
   TextJustification meJustification;
};

#endif
