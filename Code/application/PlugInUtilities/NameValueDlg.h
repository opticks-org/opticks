/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef NAMEVALUEDLG_H
#define NAMEVALUEDLG_H

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

class NameValueDlg : public QDialog
{
   Q_OBJECT

public:
   NameValueDlg(QWidget* parent = 0);
   ~NameValueDlg();

   QString getName() const;
   QString getValue() const;

public slots:
   void setName(const QString& strName);
   void setNameLabel(const QString& strName);
   void setValue(const QString& strValue);
   void setValueLabel(const QString& strValue);

private:
   QLabel* mpNameLabel;
   QLineEdit* mpNameEdit;
   QLabel* mpValueLabel;
   QLineEdit* mpValueEdit;
};

#endif
