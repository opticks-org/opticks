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
   virtual ~CreateFileDescriptor();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

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
   virtual ~CreateRasterFileDescriptor();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual FileDescriptor* createFileDescriptor() const;
   virtual bool populateFileDescriptor(FileDescriptor* pFileDescriptor) const;

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

class CreateSignatureFileDescriptor : public CreateFileDescriptor
{
public:
   CreateSignatureFileDescriptor();
   virtual ~CreateSignatureFileDescriptor();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pInArgList);
   virtual FileDescriptor* createFileDescriptor() const;
   virtual bool populateFileDescriptor(FileDescriptor* pFileDescriptor) const;

private:
   std::string mUnitsComponentName;
   std::string* mpUnitsName;
   UnitType* mpUnitsType;
   double* mpUnitsScale;
   double* mpUnitsRangeMin;
   double* mpUnitsRangeMax;
};

#endif
