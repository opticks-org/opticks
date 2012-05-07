/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILESETWIDGET_H
#define FILESETWIDGET_H

#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>

class BatchFileset;
class CustomTreeWidget;
class FileBrowser;

class FilesetWidget : public QWidget
{
   Q_OBJECT

public:
   FilesetWidget(QWidget* parent = 0);
   ~FilesetWidget();

   void setActiveFileset(BatchFileset* pFileset);
   void acceptEditedValues(bool bAccept);

signals:
   void modified();

protected slots:
   void browseForDirectory();
   void addSearchCriterion();
   void addFiles();
   void deleteSelectedCriteria();
   void validateFile(QTreeWidgetItem* pItem, int iColumn);
   void updateFileset();

private:
   FilesetWidget(const FilesetWidget& rhs);
   FilesetWidget& operator=(const FilesetWidget& rhs);

   BatchFileset* mpFileset;

   QLineEdit* mpDirectoryEdit;
   QPushButton* mpBrowseButton;
   CustomTreeWidget* mpCriteriaTree;
   FileBrowser* mpFileBrowser;
   QComboBox* mpInclusionCombo;
   QPushButton* mpNewButton;
   QPushButton* mpAddButton;
   QPushButton* mpDeleteButton;
};

#endif
