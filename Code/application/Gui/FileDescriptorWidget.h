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

#include <QtGui/QWidget>

#include "FileDescriptor.h"
#include "SafePtr.h"

#include <boost/any.hpp>
#include <string>

class FileBrowser;
class CustomTreeWidget;
class QComboBox;
class QGroupBox;
class QTreeWidgetItem;
class Subject;

class FileDescriptorWidget : public QWidget
{
   Q_OBJECT

public:
   FileDescriptorWidget(QWidget* pParent = NULL);
   virtual ~FileDescriptorWidget();

   void setFileDescriptor(FileDescriptor* pFileDescriptor, bool editable);

protected:
   virtual void showEvent(QShowEvent* pEvent);
   virtual void hideEvent(QHideEvent* pEvent);
   void fileDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);

   void initialize();
   QTreeWidgetItem* getDescriptorItem(const QString& strName, QTreeWidgetItem* pParentItem = NULL) const;

protected slots:
   void descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn);
   void updateBandFiles();

private:
   FileDescriptorWidget(const FileDescriptorWidget& rhs);
   FileDescriptorWidget& operator=(const FileDescriptorWidget& rhs);

   SafePtr<FileDescriptor> mpFileDescriptor;
   bool mEditable;
   bool mNeedsInitialization;

   CustomTreeWidget* mpTreeWidget;
   FileBrowser* mpFileBrowser;
   QComboBox* mpEndianCombo;
   QComboBox* mpInterleaveCombo;
   QComboBox* mpUnitTypeCombo;
   QGroupBox* mpGcpGroup;
   CustomTreeWidget* mpGcpTree;
};

#endif
