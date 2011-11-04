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

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeWidget>

#include "DimensionDescriptor.h"

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

class ClassificationWidget;
class DataDescriptor;
class DataDescriptorWidget;
class FileDescriptorWidget;
class ImportDescriptor;
class Importer;
class MetadataWidget;
class Subject;
class SubsetWidget;
class WavelengthsWidget;

class ImportOptionsDlg : public QDialog
{
   Q_OBJECT

public:
   ImportOptionsDlg(Importer* pImporter, const QMap<QString, std::vector<ImportDescriptor*> >& files,
      QWidget* pParent = NULL);
   ~ImportOptionsDlg();

   void allowDeselectedFiles(bool allowDeselectedFiles);
   bool areDeselectedFilesAllowed() const;

   void setCurrentDataset(ImportDescriptor* pImportDescriptor);
   ImportDescriptor* getCurrentDataset() const;
   QString getCurrentFile() const;

public slots:
   void accept();

signals:
   void currentDatasetChanged(ImportDescriptor* pDataset);

protected:
   bool validateDataset(DataDescriptor* pDescriptor);
   bool validateDataset(DataDescriptor* pDescriptor, QString& validationMessage);
   void selectCurrentDatasetItem();
   void validateEditDataset();
   void enforceSelections(QTreeWidgetItem* pItem);
   void removeImporterPage();

   void editDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void datasetItemChanged(QTreeWidgetItem* pItem);
   void updateEditDataset();
   void selectAllDatasets();
   void deselectAllDatasets();
   void generateDimensionVector(const QString& strValueName);
   void updateDataRows(const std::vector<DimensionDescriptor>& rows);
   void updateDataColumns(const std::vector<DimensionDescriptor>& columns);
   void updateDataBands(const std::vector<DimensionDescriptor>& bands);
   void updateClassification();
   void updateClassificationLabel();
   void pagesModified();
   bool applyChanges();

private:
   void updateConnections(bool bConnect);

private:
   ImportOptionsDlg(const ImportOptionsDlg& rhs);
   ImportOptionsDlg& operator=(const ImportOptionsDlg& rhs);
   Importer* mpImporter;
   std::map<ImportDescriptor*, QTreeWidgetItem*> mDatasets;
   ImportDescriptor* mpCurrentDataset;
   DataDescriptor* mpEditDescriptor;    // Contains un-applied user changes
   bool mEditDescriptorModified;
   bool mPromptForChanges;
   bool mAllowDeselectedFiles;

   QTreeWidget* mpDatasetTree;
   QLabel* mpClassificationLabel;
   QTabWidget* mpTabWidget;
   DataDescriptorWidget* mpDataPage;
   FileDescriptorWidget* mpFilePage;
   ClassificationWidget* mpClassificationPage;
   SubsetWidget* mpSubsetPage;
   MetadataWidget* mpMetadataPage;
   WavelengthsWidget* mpWavelengthsPage;
   QWidget* mpImporterPage;
   QLabel* mpValidationLabel;
   QPushButton* mpOkButton;
};

#endif
