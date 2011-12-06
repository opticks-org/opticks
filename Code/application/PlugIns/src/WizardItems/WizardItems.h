/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDITEMS_H
#define WIZARDITEMS_H

#include "WizardShell.h"

#include <string>

class Progress;
class Step;

class WizardItems : public WizardShell
{
public:
   virtual bool getInputSpecification(PlugInArgList*& pArgList);

protected:
   WizardItems();
   virtual ~WizardItems();

   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   void reportProgress(const std::string& progressMsg, int iPercent, const std::string& key);
   void reportWarning(const std::string& warningMsg, const std::string& key);
   void reportError(const std::string& errorMsg, const std::string& key);
   void reportComplete();
   Progress* getProgress() const;

   Step* mpStep;

private:
   Progress* mpProgress;
};

#endif
