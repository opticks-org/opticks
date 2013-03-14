/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMMENUPLUGIN_H
#define CUSTOMMENUPLUGIN_H

#include "ExecutableShell.h"

#include <string>

class PlugInArgList;

class CustomMenuPlugIn : public ExecutableShell
{
public:
   CustomMenuPlugIn(const std::string& name, const std::string& fullPath);
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

private:
   std::string mMenuCommand;
   std::string mFullPath;
};

#endif