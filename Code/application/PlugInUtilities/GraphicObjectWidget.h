/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICOBJECTWIDGET_H
#define GRAPHICOBJECTWIDGET_H

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QWidget>

#include "LocationType.h"

class GraphicObjectWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicObjectWidget(QWidget* pParent = NULL);
   ~GraphicObjectWidget();

   LocationType getLowerLeft() const;
   LocationType getUpperRight() const;
   double getRotation() const;

public slots:
   void setLowerLeft(const LocationType& lowerLeft);
   void setUpperRight(const LocationType& upperRight);
   void setRotation(double rotation);

signals:
   void lowerLeftChanged(const LocationType& lowerLeft);
   void upperRightChanged(const LocationType& upperRight);
   void rotationChanged(double rotation);

protected slots:
   void notifyLowerLeftChange();
   void notifyUpperRightChange();

private:
   GraphicObjectWidget(const GraphicObjectWidget& rhs);
   GraphicObjectWidget& operator=(const GraphicObjectWidget& rhs);
   QDoubleSpinBox* mpLowerLeftXSpin;
   QDoubleSpinBox* mpLowerLeftYSpin;
   QDoubleSpinBox* mpUpperRightXSpin;
   QDoubleSpinBox* mpUpperRightYSpin;
   QDoubleSpinBox* mpRotationSpin;
};

#endif
