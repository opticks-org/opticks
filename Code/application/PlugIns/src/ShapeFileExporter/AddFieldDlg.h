/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ADDFIELDDLG_H
#define ADDFIELDDLG_H

#include "DataVariant.h"

#include <QtGui/QDialog>

class QComboBox;
class QIntValidator;
class QLineEdit;
class QDoubleValidator;

class AddFieldDlg : public QDialog
{
   Q_OBJECT

public:
   AddFieldDlg(QWidget* pParent = NULL);
   virtual ~AddFieldDlg();

   QString getFieldName() const;
   QString getFieldType() const;
   DataVariant getFieldValue() const;

public slots:
   virtual void accept();

protected slots:
   void updateValue(int typeIndex);

private:
   AddFieldDlg(const AddFieldDlg& rhs);
   AddFieldDlg& operator=(const AddFieldDlg& rhs);

   QLineEdit* mpNameEdit;
   QComboBox* mpTypeCombo;
   QLineEdit* mpValueEdit;
   QIntValidator* mpIntValidator;
   QDoubleValidator* mpDoubleValidator;
};

#endif
