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

#include <QtGui/QWidget>

#include "DataDescriptor.h"
#include "SafePtr.h"
#include "TypesFile.h"

#include <boost/any.hpp>
#include <string>
#include <vector>

class CustomTreeWidget;
class QAction;
class QComboBox;
class QPushButton;
class QTreeWidgetItem;
class Subject;

class DataDescriptorWidget : public QWidget
{
   Q_OBJECT

public:
   DataDescriptorWidget(QWidget* pParent = NULL);
   virtual ~DataDescriptorWidget();

   void setDataDescriptor(DataDescriptor* pDescriptor, bool editAll);
   void setValidProcessingLocations(const std::vector<ProcessingLocation>& locations);

protected:
   virtual void showEvent(QShowEvent* pEvent);
   virtual void hideEvent(QHideEvent* pEvent);
   void dataDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);

   void initialize();
   void fileDescriptorModified(Subject& subject, const std::string& signal, const boost::any& value);
   void fileDescriptorUnitsModified(Subject& subject, const std::string& signal, const boost::any& value);
   QTreeWidgetItem* getDescriptorItem(const QString& strName, QTreeWidgetItem* pParentItem = NULL) const;

protected slots:
   void descriptorItemChanged(QTreeWidgetItem* pItem, int iColumn);
   void setDisplayBands(QAction* pAction);

private:
   DataDescriptorWidget(const DataDescriptorWidget& rhs);
   DataDescriptorWidget& operator=(const DataDescriptorWidget& rhs);

   SafePtr<DataDescriptor> mpDescriptor;
   bool mEditAll;
   bool mNeedsInitialization;

   CustomTreeWidget* mpTreeWidget;
   QComboBox* mpProcessingLocationCombo;
   QComboBox* mpDataTypeCombo;
   QComboBox* mpUnitTypeCombo;
   QComboBox* mpInterleaveCombo;
   QComboBox* mpDisplayModeCombo;
   QPushButton* mpSetDisplayButton;
};

#endif
