/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ARCREGIONCOMBOBOX_H
#define ARCREGIONCOMBOBOX_H

#include "TypesFile.h"

#include <QtGui/QComboBox>

class ArcRegionComboBox : public QComboBox
{
   Q_OBJECT

public:
   ArcRegionComboBox(QWidget* pParent);
   void setCurrentValue(ArcRegion value);
   ArcRegion getCurrentValue() const;

signals: 
   void valueChanged(ArcRegion value);

private slots:
   void translateActivated(int newIndex);
};

#endif
