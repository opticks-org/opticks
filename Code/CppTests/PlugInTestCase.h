/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PLUGIN_TEST_CASE_H
#define PLUGIN_TEST_CASE_H

#include "TestCase.h"
#include "Executable.h"

class PlugInArgList;

class PlugInTestCase : public TestCase
{
public:
   PlugInTestCase(std::string name) : TestCase(name) {}
   bool runPlugIn(std::string pName, PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList, bool runInBatch);
   PlugInArgList* getInputArgList(std::string pName, bool runInBatch);
   PlugInArgList* getOutputArgList(std::string pName, bool runInBatch);
   PlugInArgList* runPlugIn(std::string pName, bool runInBatch);
private:
   Executable* getPlugIn(std::string pName, bool runInBatch);
   PlugInArgList* getArgList(std::string pName, bool runInBatch, bool (Executable::*getSpecification)(PlugInArgList *&));
};

#endif
