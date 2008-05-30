/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MODTRAN_RUNNER_H
#define MODTRAN_RUNNER_H

#include <math.h>

#include "AlgorithmPattern.h"
#include "AlgorithmShell.h"
#include "ModtranRunnerInputs.h"

#include <string>
#include <vector>

class Progress;
class ModtranRunnerAlgorithm;
class ModtranRunnerGuiImp;

/**
 * ModtranRunner plug-in class. 
 */
class ModtranRunner : public AlgorithmPlugIn
{
private:
   // obligations from base class
   bool canRunBatch() const;
   bool canRunInteractive() const;
   bool populateBatchInputArgList(PlugInArgList *);
   bool populateInteractiveInputArgList(PlugInArgList *);
   bool populateDefaultOutputArgList(PlugInArgList *);
   bool parseInputArgList(PlugInArgList *);
   bool setActualValuesInOutputArgList(PlugInArgList *);
   QDialog *getGui(void *pAlgData);
   bool extractFromGui();
   void propagateAbort();

   // data members
   Progress* mpProgress;
   ModtranRunnerAlgorithm *mpRunnerAlg;
   ModtranRunnerInputs mInputs;

public:
   ModtranRunner();
   ~ModtranRunner();

   // obligations from base class
   bool hasAbort();
   bool needToRunAlgorithm();
};

#endif
