/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTDLG_H
#define IMPORTDLG_H

#include <QtCore/QMap>

#include "FilePlugInDlg.h"

#include <vector>

class ImportDescriptor;
class Importer;
class PreviewWidget;

class ImportDlg : public FilePlugInDlg
{
   Q_OBJECT

public:
   ImportDlg(const QString& strPlugInSubtype = QString(), const QString& strInitialPlugIn = QString(),
      QWidget* pParent = NULL);
   ~ImportDlg();

   QMap<QString, std::vector<ImportDescriptor*> > getImportDescriptors();
   Importer* getSelectedImporter() const;

public slots:
   void accept();
   void reject();

protected:
   void hideEvent(QHideEvent* pEvent);
   void updateDescriptorsIfNeeded();
   void clearDescriptors(const QString& filename);
   void clearDescriptors();

protected slots:
   bool eventFilter(QObject* pObject, QEvent* pEvent);
   bool updateFromFiles();
   void updateFromImporter(const QString& strImporter);
   bool invokeOptionsDialog();
   void updatePreviewDatasets();

private:
   ImportDlg(const ImportDlg& rhs);
   ImportDlg& operator=(const ImportDlg& rhs);
   Importer* mpImporter;
   QMap<QString, std::vector<ImportDescriptor*> > mFiles;

   PreviewWidget* mpPreview;
   bool mPreviewEnabled;
};

#endif
