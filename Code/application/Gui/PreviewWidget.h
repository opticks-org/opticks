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

#include <QtCore/QMap>
#include <QtCore/QString>
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
   void setDatasets(const QMap<QString, std::vector<ImportDescriptor*> >& datasets);
   void setCurrentFile(const QString& filename);
   QString getCurrentFile() const;
   void setCurrentDataset(ImportDescriptor* pDataset);
   ImportDescriptor* getCurrentDataset() const;

signals:
   void currentDatasetChanged(ImportDescriptor* pDataset);

protected:
   void progressUpdated(Subject& subject, const std::string& signal, const boost::any& data);

protected slots:
   void displayPreviousFile();
   void displayNextFile();
   void displayPreviousDataset();
   void displayNextDataset();
   void destroyPreview();
   void updateFileNumber();
   void updateCurrentDataset();

private:
   QStackedWidget* mpStack;

   QLabel* mpFileLabel;
   QToolButton* mpPreviousFileButton;
   QToolButton* mpNextFileButton;
   QStackedWidget* mpFileStack;

   QLabel* mpDatasetLabel;
   QToolButton* mpPreviousDatasetButton;
   QToolButton* mpNextDatasetButton;
   QStackedWidget* mpDatasetStack;

   QLabel* mpProgressLabel;
   QProgressBar* mpProgressBar;
   QWidget* mpPreview;
   QWidget* mpImporterWidget;

   Importer* mpImporter;
   QMap<QString, std::vector<ImportDescriptor*> > mDatasets;
   QString mCurrentFile;
   ImportDescriptor* mpCurrentDataset;
};

#endif
