/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREFILTERDLG_H
#define SIGNATUREFILTERDLG_H

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtGui/QDialog>

class FilterWidget;
class QCheckBox;
class QLineEdit;
class QShowEvent;

class SignatureFilterDlg : public QDialog
{
   Q_OBJECT

public:
   SignatureFilterDlg(QWidget* pParent = NULL);
   virtual ~SignatureFilterDlg();

   void setFilterName(const QString& name);
   void setLibrarySignatures(bool librarySignatures);
   void setSignatureNameFilter(const QRegExp& filter);
   void setMetadataNameFilter(const QRegExp& filter);
   void setMetadataValueFilter(const QRegExp& filter);

   QString getFilterName() const;
   bool getLibrarySignatures() const;
   QRegExp getSignatureNameFilter() const;
   QRegExp getMetadataNameFilter() const;
   QRegExp getMetadataValueFilter() const;

public slots:
   virtual void accept();

protected:
   void showEvent(QShowEvent* pEvent);

private:
   QLineEdit* mpFilterNameEdit;
   QCheckBox* mpLibrarySignatureCheck;
   FilterWidget* mpSignatureNameFilter;
   FilterWidget* mpMetadataNameFilter;
   FilterWidget* mpMetadataValueFilter;
};

#endif
