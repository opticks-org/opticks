/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "ModtranRunner.h"
#include "ModtranRunnerAlgorithm.h"

bool ModtranRunner::canRunBatch() const
{
   return false;
}

bool ModtranRunner::canRunInteractive() const
{
   return true;
}

bool ModtranRunner::populateBatchInputArgList(PlugInArgList *)
{
   return true;
}

bool ModtranRunner::populateInteractiveInputArgList(PlugInArgList *)
{
   return true;
}

bool ModtranRunner::populateDefaultOutputArgList(PlugInArgList *)
{
   return true;
}

bool ModtranRunner::parseInputArgList(PlugInArgList *)
{
   mpRunnerAlg = new ModtranRunnerAlgorithm(NULL, NULL, isInteractive());

   setAlgorithmPattern(Resource<AlgorithmPattern>(mpRunnerAlg));

   return true;
}

bool ModtranRunner::setActualValuesInOutputArgList(PlugInArgList *)
{
   return true;
}

QDialog *ModtranRunner::getGui(void *pAlgData)
{
   return NULL;
}

bool ModtranRunner::extractFromGui()
{
   return true;
}

void ModtranRunner::propagateAbort()
{
}

ModtranRunner::ModtranRunner() : AlgorithmPlugIn(&mInputs)
{
   setName("MODTRAN Runner");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright("Copyright 2006, BATC");
   setShortDescription("MODTRAN Runner");
   setDescription("Example MODTRAN Plugin");
   setMenuLocation("[Demo]\\MODTRAN");
   setProductionStatus(false);
   setDescriptorId("{BAD6C9AA-FB99-4d15-BACC-98922C085523}");
   allowMultipleInstances(true);
}

ModtranRunner::~ModtranRunner()
{
}

bool ModtranRunner::hasAbort()
{
   return false;
}

bool ModtranRunner::needToRunAlgorithm()
{
   return true;
}
