/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ANYPLUGIN_H
#define ANYPLUGIN_H

#include "AlgorithmShell.h"

class AnyPlugIn : public AlgorithmShell
{
public:
   AnyPlugIn();
   ~AnyPlugIn();

   bool setBatch();
   bool setInteractive();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   bool mInteractive;
};

#endif
