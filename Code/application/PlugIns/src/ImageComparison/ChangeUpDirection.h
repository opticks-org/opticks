/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CHANGEUPDIRECTION_H
#define CHANGEUPDIRECTION_H

#include "AlgorithmShell.h"

class ChangeUpDirection : public AlgorithmShell
{
public:
   ChangeUpDirection();
   virtual ~ChangeUpDirection();
   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool abort();

private:
   bool mAbort;
};

#endif