/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTOPTIONSDLG_H
#define IMPORTOPTIONSDLG_H

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeWidget>

#include "DimensionDescriptor.h"

#include <map>
#include <vector>

class DataDescriptor;
class DataDescriptorWidget;
class FileDescriptorWidget;
class ImportDescriptor;
class Importer;
class MetadataWidget;
class SubsetWidget;

class ImportOptionsDlg : public QDialog
{
   Q_OBJECT

public:
   ImportOptionsDlg(Importer* pImporter, const std::vector<ImportDescriptor*>& descriptors,
      QWidget* pParent = NULL);
   ~ImportOptionsDlg();

   void setCurrentDataset(ImportDescriptor* pImportDescriptor);
   ImportDescriptor* getCurrentDataset() const;

public slots:
   void accept();

signals:
   void currentDatasetChanged(ImportDescriptor* pDataset);

protected:
   void removeImporterPage();

protected slots:
   void updateCurrentDataset();
   void generateDimensionVector(const QString& strValueName);
   void updateDataRows(const std::vector<DimensionDescriptor>& rows);
   void updateDataColumns(const std::vector<DimensionDescriptor>& columns);
   void updateDataBands(const std::vector<DimensionDescriptor>& bands);
   void updateMetadata();
   void pagesModified();
   void validate();
   bool applyChanges();
   void enforceSelections(QTreeWidgetItem *pItem);

private:
   void updateConnections(bool bConnect);

private:
   Importer* mpImporter;
   std::map<ImportDescriptor*, QTreeWidgetItem*> mDatasets;
   ImportDescriptor* mpCurrentDataset;
   DataDescriptor* mpEditDescriptor;    // Contains un-applied user changes

   QTreeWidget* mpDatasetTree;
   QTabWidget* mpTabWidget;
   DataDescriptorWidget* mpDataPage;
   FileDescriptorWidget* mpFilePage;
   SubsetWidget* mpSubsetPage;
   MetadataWidget* mpMetadataPage;
   QWidget* mpImporterPage;
   QLabel* mpValidationLabel;
   QPushButton* mpOkButton;
};

#endif
