/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORTOPTIONSDLG_H
#define EXPORTOPTIONSDLG_H

#include <QtGui/QDialog>
#include <QtGui/QTabWidget>

#include "PlugInResource.h"

class SubsetWidget;

class ExportOptionsDlg : public QDialog
{
   Q_OBJECT

public:
   ExportOptionsDlg(ExporterResource& pExporter, QWidget* pParent = NULL);
   ~ExportOptionsDlg();

public slots:
   void accept();

private:
   ExportOptionsDlg(const ExportOptionsDlg& rhs);
   ExportOptionsDlg& operator=(const ExportOptionsDlg& rhs);
   ExporterResource& mpExporter;

   QTabWidget* mpTabWidget;
   SubsetWidget* mpSubsetPage;
   QWidget* mpExporterPage;
};

#endif
