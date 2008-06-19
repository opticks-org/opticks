/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDEXECUTOR_H
#define WIZARDEXECUTOR_H

#include "DesktopServices.h"
#include "ObjectFactory.h"
#include "WizardShell.h"

#include <string>

class Executable;
class Progress;
class Step;
class WizardItem;
class WizardObject;

namespace boost
{
   class any;
}

class WizardExecutor : public WizardShell
{
public:
   WizardExecutor();
   ~WizardExecutor();

   void wizardDeleted(Subject &subject, const std::string &signal, const boost::any &v);

   bool setBatch();
   bool setInteractive();
   bool hasAbort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool abort();

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);
   void populatePlugInArgList(PlugInArgList* pArgList, WizardItem* pItem, bool bInArgs);
   bool launchPlugIn(WizardItem* pItem);
   void setConnectedNodeValues(WizardItem* pItem, PlugInArgList* pOutArgList = NULL);
   void resetNodeValues(WizardItem* pItem);
   void resetAllNodeValues();

private:
   bool mbInteractive;
   bool mbAbort;
   bool mbDeleteWizard;
   Service<DesktopServices> mpDesktop;
   Service<ObjectFactory> mpObjFact;
   Progress* mpProgress;
   WizardObject* mpWizard;
   Executable* mpCurrentPlugIn;

   Step* mpStep;
   std::string mMessage;
};

#endif   // WIZARDEXECUTOR_H
