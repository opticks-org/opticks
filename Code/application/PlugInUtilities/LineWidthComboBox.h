/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LINEWIDTHCOMBOBOX_H
#define LINEWIDTHCOMBOBOX_H

#include <QtGui/QComboBox>

class LineWidthComboBox : public QComboBox
{
   Q_OBJECT

public:
   LineWidthComboBox(QWidget* pParent);
   virtual ~LineWidthComboBox();

   void setCurrentValue(unsigned int value);
   unsigned int getCurrentValue() const;

signals:
   void valueChanged(unsigned int value);

private slots:
   void translateActivated(int newIndex);

private:
   LineWidthComboBox(const LineWidthComboBox& rhs);
   LineWidthComboBox& operator=(const LineWidthComboBox& rhs);
};

#endif
