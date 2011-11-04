/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDPROPERTIES_H
#define WIZARDPROPERTIES_H

#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QWidget>

#include "AttachmentPtr.h"
#include "WizardObjectAdapter.h"

class WizardProperties : public QWidget
{
   Q_OBJECT

public:
   WizardProperties(QWidget* pParent = 0);
   virtual ~WizardProperties();

   void setWizard(WizardObject* pWizard);
   WizardObject* getWizard();
   const WizardObject* getWizard() const;

protected:
   void wizardRenamed(Subject& subject, const std::string& signal, const boost::any& value);
   void menuLocationChanged(Subject& subject, const std::string& signal, const boost::any& value);
   void executionModeChanged(Subject& subject, const std::string& signal, const boost::any& value);

protected slots:
   void setWizardName(const QString& wizardName);
   void setWizardMenuLocation(const QString& menuLocation);
   void setWizardExecutionMode(int modeIndex);
   void executeWizard();

private:
   WizardProperties(const WizardProperties& rhs);
   WizardProperties& operator=(const WizardProperties& rhs);
   QLineEdit* mpNameEdit;
   QLineEdit* mpMenuEdit;
   QComboBox* mpModeCombo;

   AttachmentPtr<WizardObjectAdapter> mpWizard;
};

#endif
