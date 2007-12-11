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

#include "FilePlugInDlg.h"

#include <vector>

class ImportDescriptor;
class PlugIn;
class PreviewWidget;

class ImportDlg : public FilePlugInDlg
{
   Q_OBJECT

public:
   ImportDlg(const QString& strPlugInSubtype = QString(), const QString& strInitialPlugIn = QString(),
      QWidget* pParent = 0);
   ~ImportDlg();

   const std::vector<ImportDescriptor*>& getImportDescriptors() const;
   PlugIn* getSelectedImporter() const;

public slots:
   void accept();
   void reject();

protected:
   void hideEvent(QHideEvent* pEvent);
   void updateDescriptorsIfNeeded();
   void clearDescriptors();
   ImportDescriptor* getFirstImportedDescriptor() const;
   QString getSelectedFile() const;

protected slots:
   bool eventFilter(QObject* o, QEvent* e);
   virtual bool updateFromFile(const QString& strFilename);
   virtual void updateFromImporter(const QString& strImporter);
   virtual bool invokeOptionsDialog();
   void updatePreviewDatasets();

private:
   bool mFileChanged;
   PlugIn* mpImporter;
   std::vector<ImportDescriptor*> mDescriptors;

   PreviewWidget* mpPreview;
   bool mPreviewEnabled;
};

#endif
