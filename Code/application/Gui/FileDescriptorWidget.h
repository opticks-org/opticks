/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FILEDESCRIPTORWIDGET_H
#define FILEDESCRIPTORWIDGET_H

#include <QtGui/QGroupBox>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>

class FileBrowser;
class FileDescriptor;
class CustomTreeWidget;

class FileDescriptorWidget : public QWidget
{
   Q_OBJECT

public:
   FileDescriptorWidget(QWidget* parent = 0);
   ~FileDescriptorWidget();

   void setFileDescriptor(FileDescriptor* pFileDescriptor);
   void setFileDescriptor(const FileDescriptor* pFileDescriptor);

   void setDescriptorValue(const QString& strValueName, const QString& strValue);
   QString getDescriptorValue(const QString& strValueName) const;

   void initialize();
   bool isModified() const;
   bool applyChanges();
   bool applyToFileDescriptor(FileDescriptor* pFileDescriptor);

   QSize sizeHint() const;

signals:
   void valueChanged(const QString& strValueName);
   void modified();

protected:
   QTreeWidgetItem* getDescriptorItem(const QString& strName, QTreeWidgetItem* pStartAt = 0) const;

protected slots:
   void descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn);
   void updateBandFiles();

private:
   FileDescriptorWidget(const FileDescriptorWidget& rhs);
   FileDescriptorWidget& operator=(const FileDescriptorWidget& rhs);
   FileDescriptor* mpFileDescriptor;
   bool mReadOnly;
   bool mModified;

   CustomTreeWidget* mpTreeWidget;
   FileBrowser* mpFileBrowser;
   QGroupBox* mpGcpGroup;
   CustomTreeWidget* mpGcpTree;
};

#endif
