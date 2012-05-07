/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef STRETCHTYPECOMBOBOX_H
#define STRETCHTYPECOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class StretchTypeComboBox : public QComboBox
{
   Q_OBJECT

public:
   StretchTypeComboBox(QWidget* pParent);
   void setCurrentValue(StretchType value);
   StretchType getCurrentValue() const;

signals: 
   void valueChanged(StretchType value);

private slots:
   void translateActivated(int newIndex);

private:
   StretchTypeComboBox(const StretchTypeComboBox& rhs);
   StretchTypeComboBox& operator=(const StretchTypeComboBox& rhs);
};

#endif
