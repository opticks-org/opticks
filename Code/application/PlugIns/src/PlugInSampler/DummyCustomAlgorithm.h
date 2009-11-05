/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DUMMYCUSTOMALGORITHM_H
#define DUMMYCUSTOMALGORITHM_H

#include "AlgorithmShell.h"
#include "ModelServices.h"
#include "PlugInManagerServices.h"

class DummyCustomAlgorithm : public AlgorithmShell
{
public:
   DummyCustomAlgorithm();
   ~DummyCustomAlgorithm();

public:
   bool getInputSpecification(PlugInArgList*&);
   bool getOutputSpecification(PlugInArgList*&);
   bool execute(PlugInArgList*, PlugInArgList*);

private:
   Service<PlugInManagerServices> mpPlugInManager;
   Service<ModelServices> mpModel;
};

#endif
