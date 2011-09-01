/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BATCHWIZARDEXECUTOR_H
#define BATCHWIZARDEXECUTOR_H

#include "ObjectFactory.h"
#include "PlugInManagerServices.h"
#include "WizardShell.h"

#include <string>

class BatchWizard;
class Progress;
class Step;
class Value;
class WizardExecutor;
class WizardNode;
class WizardObject;

class BatchWizardExecutor : public WizardShell
{
public:
   BatchWizardExecutor();
   ~BatchWizardExecutor();

   bool hasAbort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

   WizardNode* getValueNode(const WizardObject* pWizard, const std::string& connectedItemName,
      const std::string& nodeName, const std::string& nodeType) const;
   bool runWizard(WizardObject* pWizard);

private:
   bool mbAbort;
   Progress* mpProgress;
   Step* mpStep;

   std::string mFilename;
   WizardExecutor* mpExecutor;

   Service<PlugInManagerServices> mpPlugInManager;
   Service<ObjectFactory> mpObjFact;
};

#endif
