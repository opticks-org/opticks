/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DMSFORMATTYPECOMBOBOX_H
#define DMSFORMATTYPECOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class DmsFormatTypeComboBox : public QComboBox
{
   Q_OBJECT

public:
   DmsFormatTypeComboBox(QWidget* pParent);
   virtual~DmsFormatTypeComboBox();

   void setCurrentValue(DmsFormatType value);
   DmsFormatType getCurrentValue() const;

signals: 
   void valueChanged(DmsFormatType value);

private:
   DmsFormatTypeComboBox(const DmsFormatTypeComboBox& rhs);
   DmsFormatTypeComboBox& operator=(const DmsFormatTypeComboBox& rhs);

private slots:
   void translateIndexChanged(const QString& text);
};

#endif
