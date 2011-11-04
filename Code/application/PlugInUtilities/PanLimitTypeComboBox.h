/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PANLIMITTYPECOMBOBOX_H
#define PANLIMITTYPECOMBOBOX_H

#include "SpatialDataView.h"

#include <QtGui/QComboBox>

class PanLimitTypeComboBox : public QComboBox
{
   Q_OBJECT

public:
   PanLimitTypeComboBox(QWidget* pParent);
   void setCurrentValue(PanLimitType value);
   PanLimitType getCurrentValue() const;

signals: 
   void valueChanged(PanLimitType value);

private slots:
   void translateActivated(int newIndex);

private:
   PanLimitTypeComboBox(const PanLimitTypeComboBox& rhs);
   PanLimitTypeComboBox& operator=(const PanLimitTypeComboBox& rhs);
};

#endif
