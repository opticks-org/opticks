/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NAMETYPEVALUEDLG_H
#define NAMETYPEVALUEDLG_H

#include <QtCore/QString>
#include <QtGui/QDialog>

class DataVariant;
class DataVariantEditor;
class QLineEdit;
class QListWidget;

class NameTypeValueDlg : public QDialog
{
   Q_OBJECT

public:
   NameTypeValueDlg(QWidget* pParent = 0);
   virtual ~NameTypeValueDlg();

   void setEmptyValue(const QString& name, const QString& type);
   void setValue(const QString& strName, const DataVariant& value);

   QString getName() const;
   QString getType() const;
   const DataVariant& getValue() const;

protected slots:
   void changeType(int newTypeRowIndex);
   void accept();

private:
   NameTypeValueDlg(const NameTypeValueDlg& rhs);
   NameTypeValueDlg& operator=(const NameTypeValueDlg& rhs);

   QLineEdit* mpNameEdit;
   QListWidget* mpTypeList;
   DataVariantEditor* mpValueEditor;

   QString mCurrentType;
};

#endif
