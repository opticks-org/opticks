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

#include "DataVariant.h"

#include <QtCore/QString>
#include <QtGui/QDialog>

class DataVariantEditor;
class QLineEdit;
class QListWidget;
class QWidget;

#include <string>

class NameTypeValueDlg : public QDialog
{
   Q_OBJECT

public:
   NameTypeValueDlg(QWidget* parent = 0);
   ~NameTypeValueDlg();

   void setEmptyValue(const QString& name, const QString& type);
   void setValue(const QString& strName, const DataVariant& value);

   QString getName() const;
   QString getType() const;
   const DataVariant &getValue();

protected slots:
   void changeType(int newTypeRowIndex);
   void accept();

private:
   QLineEdit* mpNameEdit;
   QListWidget* mpTypeList;
   DataVariantEditor* mpValueEditor;

   QString mCurrentType;
};

#endif
