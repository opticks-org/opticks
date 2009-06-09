/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAFUSION_H
#define DATAFUSION_H

#include "ViewerShell.h"
#include "PlugInManagerServices.h"
#include "ProgressTracker.h"
#include "Testable.h"

class DataFusionDlg;

class DataFusion : public ViewerShell, public Testable
{
public:
   DataFusion();
   ~DataFusion();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

   bool setBatch();
   bool setInteractive();
   bool abort();
   bool execute(PlugInArgList* pInputArgs, PlugInArgList* pOutputArgs);

   bool runOperationalTests(Progress* pProgress, std::ostream& failure);
   bool runAllTests(Progress* pProgress, std::ostream& failure);

   static const std::string PLUGIN_NAME;

protected:
   QWidget* getWidget() const;

private:
   Service<PlugInManagerServices> mpPlugInManager;
   bool mbInteractive;
   DataFusionDlg* mpWizardDlg;
   ProgressTracker mProgressTracker;
};

#endif
