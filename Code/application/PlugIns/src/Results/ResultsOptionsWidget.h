/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RESULTSOPTIONSWIDGET_H
#define RESULTSOPTIONSWIDGET_H

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QWidget>

#include "TypesFile.h"

class PassAreaComboBox;

class ResultsOptionsWidget : public QWidget
{
   Q_OBJECT

public:
   ResultsOptionsWidget(QWidget* pParent = 0);
   ~ResultsOptionsWidget();

   void setAppendToFile(bool bAppend);
   bool appendToFile() const;

   void enableMetadata(bool bEnable);
   bool isMetadataEnabled() const;

   void setFirstThreshold(double dThreshold);
   double getFirstThreshold() const;

   void setSecondThreshold(double dThreshold);
   double getSecondThreshold() const;

   void setPassArea(PassArea passArea);
   PassArea getPassArea() const;

   void setGeocoordType(GeocoordType geocoordType);
   GeocoordType getGeocoordType() const;

protected slots:
   void enableSecondThreshold(PassArea area);

private:
   PassAreaComboBox* mpPassAreaCombo;
   QLineEdit* mpFirstThresholdEdit;
   QLabel* mpSecondThresholdLabel;
   QLineEdit* mpSecondThresholdEdit;
   QLabel* mpCoordLabel;
   QComboBox* mpCoordCombo;
   QCheckBox* mpMetadataCheck;
   QCheckBox* mpAppendCheck;
};

#endif
