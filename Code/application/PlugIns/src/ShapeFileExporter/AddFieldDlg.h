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

#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>

class AddFieldDlg : public QDialog
{
   Q_OBJECT

public:
   AddFieldDlg(QWidget* parent = 0);
   ~AddFieldDlg();

   QString getFieldName() const;
   QString getFieldType() const;

protected:
   void accept();

private:
   QLineEdit* mpNameEdit;
   QComboBox* mpTypeCombo;
};

#endif
