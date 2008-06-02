/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMPORTDATASET_H
#define IMPORTDATASET_H

#include "DesktopItems.h"

#include <string>

class DataDescriptor;
class Filename;

class ImportDataSet : public DesktopItems
{
public:
   ImportDataSet();
   ~ImportDataSet();

   bool setBatch();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   Filename* mpFilename;
   DataDescriptor* mpDescriptor;
   std::string mImporterName;
   bool mShowDialog;
};

#endif
