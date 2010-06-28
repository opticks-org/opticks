/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATADESCRIPTORWIDGET_H
#define DATADESCRIPTORWIDGET_H

#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QWidget>

#include "TypesFile.h"

#include <vector>

class CustomTreeWidget;
class DataDescriptor;
class QAction;
class QPushButton;

class DataDescriptorWidget : public QWidget
{
   Q_OBJECT

public:
   DataDescriptorWidget(QWidget* parent = 0);
   ~DataDescriptorWidget();

   void setDataDescriptor(DataDescriptor* pDescriptor, bool editAll);

   void setValidProcessingLocations(const std::vector<ProcessingLocation>& locations);

   void setDescriptorValue(const QString& strValueName, const QString& strValue);
   QString getDescriptorValue(const QString& strValueName) const;

   void initialize();
   bool isModified() const;
   bool applyChanges();
   bool applyToDataDescriptor(DataDescriptor* pDescriptor);

   QSize sizeHint() const;

signals:
   void valueChanged(const QString& strValueName);
   void modified();

protected:
   QTreeWidgetItem* getDescriptorItem(const QString& strName) const;

protected slots:
   void descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn);
   void setDisplayBands(QAction* pAction);

private:
   bool mEditAll;
   bool mModified;
   DataDescriptor* mpDescriptor;

   QLabel* mpClassificationLabel;
   CustomTreeWidget* mpTreeWidget;
   QPushButton* mpSetDisplayButton;

   std::vector<ProcessingLocation> mProcessingLocations;
   QComboBox* mpProcessingLocationCombo;
};

#endif
