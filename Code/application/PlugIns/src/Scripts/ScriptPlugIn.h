/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCRIPTPLUGIN_H
#define SCRIPTPLUGIN_H

#include "AlgorithmShell.h"

class ScriptPlugIn : public AlgorithmShell
{
public:
   ScriptPlugIn();
   ~ScriptPlugIn();

   bool getInputSpecification(PlugInArgList *&);
   bool getOutputSpecification(PlugInArgList *&);
   bool execute(PlugInArgList *, PlugInArgList *);
   bool initialize();

private:
   bool populateOutputArgList(PlugInArgList *pOutArgList);
};

#endif
