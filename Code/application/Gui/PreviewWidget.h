/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolButton>
#include <QtGui/QWidget>

#include <boost/any.hpp>
#include <vector>

class ImportDescriptor;
class Importer;
class Subject;

class PreviewWidget : public QWidget
{
   Q_OBJECT

public:
   PreviewWidget(QWidget* pParent = NULL);
   ~PreviewWidget();

   void setImporter(Importer* pImporter);
   void setDatasets(const std::vector<ImportDescriptor*>& datasets);
   void setActiveDataset(ImportDescriptor* pDataset);
   ImportDescriptor* getActiveDataset() const;

signals:
   void activeDatasetChanged(ImportDescriptor* pDataset);

protected:
   void progressUpdated(Subject& subject, const std::string& signal, const boost::any& data);
   unsigned int getNumImportedDatasets() const;

protected slots:
   void displayPreviousPreview();
   void displayNextPreview();
   void destroyPreview();
   void updateActiveDataset();

private:
   QStackedWidget* mpStack;
   QLabel* mpProgressLabel;
   QProgressBar* mpProgressBar;
   QWidget* mpPreview;
   QLabel* mpDatasetLabel;
   QLabel* mpAllDatasetsLabel;
   QLabel* mpPreviewDatasetsLabel;
   QToolButton* mpBackButton;
   QToolButton* mpNextButton;
   QWidget* mpImporterWidget;

   Importer* mpImporter;
   std::vector<ImportDescriptor*> mDatasets;
   ImportDescriptor* mpActiveDataset;
};

#endif
