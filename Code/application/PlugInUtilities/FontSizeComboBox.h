/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FONTSIZECOMBOBOX_H
#define FONTSIZECOMBOBOX_H

#include <QtGui/QComboBox>

class FontSizeComboBox : public QComboBox
{
   Q_OBJECT

public:
   FontSizeComboBox(QWidget* pParent);
   ~FontSizeComboBox();

   void setCurrentValue(int fontSize);
   int getCurrentValue() const;

signals: 
   void valueEdited(int fontSize);
   void valueActivated(int fontSize);
   void valueChanged(int fontSize);

private slots:
   void translateEdited();
   void translateActivated();
   void translateChanged();
};

#endif
