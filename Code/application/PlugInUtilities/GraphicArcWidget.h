/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICARCWIDGET_H
#define GRAPHICARCWIDGET_H

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QWidget>

#include "TypesFile.h"

class ArcRegionComboBox;

class GraphicArcWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicArcWidget(QWidget* pParent = NULL);
   ~GraphicArcWidget();

   double getStartAngle() const;
   double getStopAngle() const;
   ArcRegion getRegion() const;

public slots:
   void setStartAngle(double angle);
   void setStopAngle(double angle);
   void setRegion(ArcRegion region);

signals:
   void startAngleChanged(double angle);
   void stopAngleChanged(double angle);
   void regionChanged(ArcRegion region);

private:
   QDoubleSpinBox* mpStartSpin;
   QDoubleSpinBox* mpStopSpin;
   ArcRegionComboBox* mpRegionCombo;
};

#endif
