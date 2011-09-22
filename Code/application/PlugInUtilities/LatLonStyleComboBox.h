/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LATLONSTYLECOMBOBOX_H
#define LATLONSTYLECOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class LatLonStyleComboBox : public QComboBox
{
   Q_OBJECT

public:
   LatLonStyleComboBox(QWidget* pParent);
   void setCurrentValue(LatLonStyle value);
   LatLonStyle getCurrentValue() const;

signals: 
   void valueChanged(LatLonStyle value);

private slots:
   void translateActivated(int newIndex);
};

#endif
