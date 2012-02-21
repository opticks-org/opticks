/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CREATEEXPORTFILEDESCRIPTOR_H
#define CREATEEXPORTFILEDESCRIPTOR_H

#include "ModelItems.h"

#include <string>

class DataElement;
class FileDescriptor;
class Filename;
class Units;

class CreateExportFileDescriptor : public ModelItems
{
public:
   CreateExportFileDescriptor();
   ~CreateExportFileDescriptor();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);
   void populateUnits(FileDescriptor* pDescriptor);

private:
   Filename* mpFilename;
   DataElement* mpElement;

   unsigned int* mpStartRow;            // One-based original row number
   unsigned int* mpEndRow;              // One-based original row number
   unsigned int* mpRowSkipFactor;
   unsigned int* mpStartColumn;         // One-based original column number
   unsigned int* mpEndColumn;           // One-based original column number
   unsigned int* mpColumnSkipFactor;
   unsigned int* mpStartBand;           // One-based original band number
   unsigned int* mpEndBand;             // One-based original band number
   unsigned int* mpBandSkipFactor;
   std::string* mpUnitsName;
   UnitType* mpUnitsType;
   double* mpUnitsScale;
   double* mpUnitsRangeMin;
   double* mpUnitsRangeMax;
   std::string* mpComponentName;
};

#endif
