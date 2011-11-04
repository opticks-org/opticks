/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COMPLEXCOMPONENTCOMBOBOX_H
#define COMPLEXCOMPONENTCOMBOBOX_H

#include "ComplexData.h"

#include <QtGui/QComboBox>

class ComplexComponentComboBox : public QComboBox
{
   Q_OBJECT

public:
   ComplexComponentComboBox(QWidget* pParent);
   void setCurrentValue(ComplexComponent value);
   ComplexComponent getCurrentValue() const;

signals: 
   void valueChanged(ComplexComponent value);

private:
   ComplexComponentComboBox(const ComplexComponentComboBox& rhs);
   ComplexComponentComboBox& operator=(const ComplexComponentComboBox& rhs);

private slots:
   void translateActivated(int newIndex);
};

#endif
