/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GRAPHICMEASUREMENTWIDGET_H
#define GRAPHICMEASUREMENTWIDGET_H

#include <QtGui/QWidget>

class QCheckBox;
class QSpinBox;

class GraphicMeasurementWidget : public QWidget
{
   Q_OBJECT

public:
   GraphicMeasurementWidget(QWidget* pParent = NULL);
   ~GraphicMeasurementWidget();

   void setDistancePrecision(int precision);
   int getDistancePrecision() const;

   void setBearingPrecision(int precision);
   int getBearingPrecision() const;

   void setEndPointsPrecision(int precision);
   int getEndPointsPrecision() const;

signals:
   void distancePrecisionChanged(int precision);
   void bearingPrecisionChanged(int precision);
   void endPointsPrecisionChanged(int precision);

private:
   GraphicMeasurementWidget(const GraphicMeasurementWidget& rhs);
   GraphicMeasurementWidget& operator=(const GraphicMeasurementWidget& rhs);
   QSpinBox* mpDistancePrecisionSpin;
   QSpinBox* mpBearingPrecisionSpin;
   QSpinBox* mpEndPointsPrecisionSpin;

};

#endif
