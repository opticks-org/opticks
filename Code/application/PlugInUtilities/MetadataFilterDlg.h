/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef METADATAFILTERDLG_H
#define METADATAFILTERDLG_H

#include <QtCore/QRegExp>
#include <QtGui/QDialog>

class QCheckBox;
class QLineEdit;

class MetadataFilterDlg : public QDialog
{
   Q_OBJECT

public:
   MetadataFilterDlg(QWidget* pParent = NULL);
   virtual ~MetadataFilterDlg();

   QRegExp getNameFilter() const;
   QRegExp getValueFilter() const;

public slots:
   virtual void accept();

private:
   QLineEdit* mpNameEdit;
   QCheckBox* mpNameWildcardCheck;
   QCheckBox* mpNameCaseCheck;
   QLineEdit* mpValueEdit;
   QCheckBox* mpValueWildcardCheck;
   QCheckBox* mpValueCaseCheck;
};

#endif
