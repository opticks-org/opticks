/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDITEMPROPERTIES_H
#define WIZARDITEMPROPERTIES_H

#include <QtCore/QMap>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QTreeWidget>

#include "AttachmentPtr.h"
#include "WizardItemImp.h"

class WizardNodeImp;

class WizardItemProperties : public QWidget
{
   Q_OBJECT

public:
   WizardItemProperties(QWidget* pParent = 0);
   virtual ~WizardItemProperties();

   void setWizardItem(WizardItem* pItem);
   WizardItem* getWizardItem();
   const WizardItem* getWizardItem() const;

   virtual QSize sizeHint() const;

protected:
   void wizardExecutionModeChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void executionOrderChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void itemRenamed(Subject& subject, const std::string& signal, const boost::any& data);
   void itemExecutionModeChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeAdded(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeRemoved(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeRenamed(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeTypeChanged(Subject& subject, const std::string& signal, const boost::any& data);
   void nodeDescriptionChanged(Subject& subject, const std::string& signal, const boost::any& data);

   void addNode(WizardNodeImp* pNode);
   void removeNode(WizardNodeImp* pNode);

protected slots:
   void increaseItemOrder();
   void decreaseItemOrder();
   void setItemExecutionMode(int modeIndex);

private:
   QLabel* mpItemNameLabel;
   QLabel* mpItemTypeLabel;
   QLabel* mpItemOrderLabel;
   QToolButton* mpIncreaseOrderButton;
   QToolButton* mpDecreaseOrderButton;
   QComboBox* mpItemModeCombo;
   QTreeWidget* mpItemInputsTree;
   QTreeWidget* mpItemOutputsTree;
   QMap<WizardNodeImp*, QTreeWidgetItem*> mNodes;

   AttachmentPtr<WizardItemImp> mpItem;
};

#endif
