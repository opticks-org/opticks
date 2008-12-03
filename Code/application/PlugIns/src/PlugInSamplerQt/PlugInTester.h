/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGINTESTER_H
#define PLUGINTESTER_H

#include "AlgorithmShell.h"
#include "DesktopServices.h"
#include "PlugInManagerServices.h"

class PlugInTester : public AlgorithmShell
{
public:
   PlugInTester();

   bool getInputSpecification(PlugInArgList *&pArgList);
   bool getOutputSpecification(PlugInArgList *&pArgList);

   bool execute(PlugInArgList*, PlugInArgList*);

private:
   Service<PlugInManagerServices> mpPlugMgr;
   Service<DesktopServices> mpDesktop;
};

#endif
