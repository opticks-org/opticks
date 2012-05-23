/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EDITDATADESCRIPTOR_H
#define EDITDATADESCRIPTOR_H

#include "ModelItems.h"
#include "TypesFile.h"

#include <string>

class DataDescriptor;
class FileDescriptor;
class Filename;
class Units;

class EditDataDescriptor : public ModelItems
{
public:
   EditDataDescriptor();
   ~EditDataDescriptor();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);

private:
   DataDescriptor* mpDescriptor;
   FileDescriptor* mpFileDescriptor;

   ProcessingLocation* mpProcessingLocation;
   EncodingType* mpDataType;
   InterleaveFormatType* mpInterleave;
   std::string* mpBadValuesStr;

   unsigned int* mpStartRow;            // One-based original row number
   unsigned int* mpEndRow;              // One-based original row number
   unsigned int* mpRowSkipFactor;
   unsigned int* mpStartColumn;         // One-based original column number
   unsigned int* mpEndColumn;           // One-based original column number
   unsigned int* mpColumnSkipFactor;
   unsigned int* mpStartBand;           // One-based original band number
   unsigned int* mpEndBand;             // One-based original band number
   unsigned int* mpBandSkipFactor;
   Filename* mpBadBandsFile;

   double* mpPixelSizeX;
   double* mpPixelSizeY;

   std::string* mpUnitsName;
   UnitType* mpUnitsType;
   double* mpUnitsScale;
   double* mpUnitsRangeMin;
   double* mpUnitsRangeMax;

   DisplayMode* mpDisplayMode;
   unsigned int* mpGrayBand;            // One-based original band number
   unsigned int* mpRedBand;             // One-based original band number
   unsigned int* mpGreenBand;           // One-based original band number
   unsigned int* mpBlueBand;            // One-based original band number

   std::string* mpComponentName;
};

#endif
