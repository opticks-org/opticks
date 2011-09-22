/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAPLOTTERPLUGIN_H
#define DATAPLOTTERPLUGIN_H

#include "AlgorithmShell.h"

class DataPlotterPlugIn : public AlgorithmShell
{
public:
   DataPlotterPlugIn();
   ~DataPlotterPlugIn();

   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   bool mInteractive;
};

#endif
