/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DEMO_H
#define DEMO_H

#include <math.h>

#include "AlgorithmPattern.h"
#include "AlgorithmShell.h"
#include "DemoInputs.h"
#include "Node.h"

#include <string>
#include <vector>

class Progress;
class DemoAlgorithm;
class DemoGuiImp;

/**
 * Demo plug-in class. Implements the interface between the Dynamic Colormap
 * algorithm and the main application.
 */
class Demo : public AlgorithmPlugIn
{
public:
   Demo();
   ~Demo();

   // obligations from base class
   bool hasAbort();
   bool needToRunAlgorithm();

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
   DemoAlgorithm* mpDemoAlg;
   DemoInputs mInputs;
   DemoGuiImp* mpDemoGui;
};

#endif
