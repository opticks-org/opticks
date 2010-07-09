/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GETDATASETWAVELENGTHS_H
#define GETDATASETWAVELENGTHS_H

#include "ExecutableShell.h"

class GetDataSetWavelengths : public ExecutableShell
{
public:
   GetDataSetWavelengths();
   virtual ~GetDataSetWavelengths();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
