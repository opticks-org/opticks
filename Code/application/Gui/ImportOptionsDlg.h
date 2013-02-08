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
#include <QtGui/QCheckBox>
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
class GeoreferenceWidget;
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
   virtual ~ImportOptionsDlg();

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
   void validateAllDatasets();
   void enforceSelections(QTreeWidgetItem* pItem);
   void removeImporterPage();

   void editDataDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editDataDescriptorRowsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editDataDescriptorColumnsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editDataDescriptorBandsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editFileDescriptorRowsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editFileDescriptorColumnsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editFileDescriptorBandsModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editClassificationModified(Subject& subject, const std::string& signal, const boost::any& value);
   void editGeoreferenceOnImportModified(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void datasetItemChanged(QTreeWidgetItem* pItem);
   void updateEditDataset();
   void selectAllDatasets();
   void deselectAllDatasets();
   void updateEditDataDescriptorRows(const std::vector<DimensionDescriptor>& rows);
   void updateEditDataDescriptorColumns(const std::vector<DimensionDescriptor>& columns);
   void updateEditDataDescriptorBands(const std::vector<DimensionDescriptor>& bands);
   void updateClassificationLabel();
   void enableGeoreference(bool enable);
   bool applyChanges();

private:
   void setSubsetBands(const std::vector<DimensionDescriptor>& bands,
      const std::vector<DimensionDescriptor>& selectedBands);
   void updateConnections(bool bConnect);

private:
   ImportOptionsDlg(const ImportOptionsDlg& rhs);
   ImportOptionsDlg& operator=(const ImportOptionsDlg& rhs);

   Importer* mpImporter;
   std::map<ImportDescriptor*, QTreeWidgetItem*> mDatasets;
   ImportDescriptor* mpCurrentDataset;
   DataDescriptor* mpEditDescriptor;    // Contains un-applied user changes
   bool mEditDataDescriptorModified;
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
   QWidget* mpGeoreferencePage;
   QCheckBox* mpGeoreferenceCheck;
   GeoreferenceWidget* mpGeoreferenceWidget;
   QWidget* mpImporterPage;
   QLabel* mpValidationLabel;
   QPushButton* mpOkButton;
};

#endif
