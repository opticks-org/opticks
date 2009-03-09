/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef REGIONUNITSCOMBOBOX_H
#define REGIONUNITSCOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class RegionUnitsComboBox : public QComboBox
{
   Q_OBJECT

public:
   RegionUnitsComboBox(QWidget* pParent);
   void setCurrentValue(RegionUnits value);
   RegionUnits getCurrentValue() const;

signals: 
   void valueChanged(RegionUnits value);

private slots:
   void translateActivated(int newIndex);
};

#endif
