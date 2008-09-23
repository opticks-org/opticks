/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CREATEFILEDESCRIPTOR_H
#define CREATEFILEDESCRIPTOR_H

#include "ModelItems.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class FileDescriptor;
class Filename;

class CreateFileDescriptor : public ModelItems
{
public:
   CreateFileDescriptor();
   ~CreateFileDescriptor();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual FileDescriptor* createFileDescriptor() const;
   virtual bool populateFileDescriptor(FileDescriptor* pFileDescriptor) const;

private:
   Filename* mpFilename;
   std::string mDatasetLocation;
   EndianType mEndianType;
};

class CreateRasterFileDescriptor : public CreateFileDescriptor
{
public:
   CreateRasterFileDescriptor();
   ~CreateRasterFileDescriptor();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);

protected:
   bool extractInputArgs(PlugInArgList* pInArgList);
   FileDescriptor* createFileDescriptor() const;
   bool populateFileDescriptor(FileDescriptor* pFileDescriptor) const;

private:
   unsigned int* mpHeaderBytes;
   unsigned int* mpTrailerBytes;
   unsigned int* mpPrelineBytes;
   unsigned int* mpPostlineBytes;
   unsigned int* mpPrebandBytes;
   unsigned int* mpPostbandBytes;
   unsigned int mBitsPerElement;
   unsigned int mNumRows;
   unsigned int mNumColumns;
   unsigned int mNumBands;
   double* mpPixelSizeX;
   double* mpPixelSizeY;
   std::string* mpUnitsName;
   UnitType* mpUnitsType;
   double* mpUnitsScale;
   double* mpUnitsRangeMin;
   double* mpUnitsRangeMax;
   InterleaveFormatType mInterleave;
   std::vector<Filename*>* mpBandFiles;
};

#endif
