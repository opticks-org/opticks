/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SETDATASETWAVELENGTHS_H
#define SETDATASETWAVELENGTHS_H

#include "ExecutableShell.h"

class SetDataSetWavelengths : public ExecutableShell
{
public:
   SetDataSetWavelengths();
   virtual ~SetDataSetWavelengths();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
