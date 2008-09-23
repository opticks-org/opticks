/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#include "FilePlugInDlg.h"
#include "PlugInResource.h"

class ExportDlg : public FilePlugInDlg
{
   Q_OBJECT

public:
   ExportDlg(ExporterResource& pExporter, const std::vector<PlugInDescriptor*>& availablePlugins,
      QWidget* pParent = NULL);
   ~ExportDlg();

   ExporterResource &getExporterResource() const;

public slots:
   void accept();

protected:
   void hideEvent(QHideEvent* e);
   virtual QString getExportFile() const;
   QString updateExtension(const QString& strFilename,
      bool preserveFullPath = false,
      bool alwaysReplaceExtension = true);

protected slots:
   virtual void updateFromFile(const QString& strFilename);
   virtual void updateFromExporter(const QString& strExporter);
   bool invokeOptionsDialog();

private:
   ExporterResource& mpExporter;
};

#endif
