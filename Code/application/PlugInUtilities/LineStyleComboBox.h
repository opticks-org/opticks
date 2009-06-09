/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINESTYLECOMBOBOX_H
#define LINESTYLECOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class LineStyleComboBox : public QComboBox
{
   Q_OBJECT

public:
   LineStyleComboBox(QWidget* pParent);
   void setCurrentValue(LineStyle value);
   LineStyle getCurrentValue() const;

signals: 
   void valueChanged(LineStyle value);

private slots:
   void translateActivated(int newIndex);
};

#endif
