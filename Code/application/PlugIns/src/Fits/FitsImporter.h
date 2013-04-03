/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FITSIMPORTER_H
#define FITSIMPORTER_H

#include "CachedPager.h"
#include "RasterElementImporterShell.h"

#include <map>
#include <fitsio.h>
#define _TCHAR_DEFINED

class FitsFileResource
{
public:
   FitsFileResource();
   explicit FitsFileResource(const std::string& fname);
   ~FitsFileResource();
   bool isValid() const;
   std::string getStatus() const;
   operator fitsfile*();
   void reset(const std::string& fname);

private:
   fitsfile* mpFile;
   int mStatus;
};

class FitsImporter : public RasterElementImporterShell
{
public:
   FitsImporter();
   virtual ~FitsImporter();

   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool createRasterPager(RasterElement* pRaster) const;

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

private:
   bool splitFilename(std::string& filename, int& hduCnt, int& specificHdu, int& hdu, FitsFileResource& pFile,
      std::vector<std::vector<std::string> >& errors, std::vector<std::vector<std::string> >& warnings);
   EncodingType checkForOverflow(EncodingType encoding, DynamicObject* pMetadata, int hdu,
      std::vector<std::vector<std::string> >& errors, std::vector<std::vector<std::string> >& warnings);

   std::map<std::string, std::vector<std::vector<std::string> > > mErrors;
   std::map<std::string, std::vector<std::vector<std::string> > > mWarnings;
};

class FitsSignatureImporter : public FitsImporter
{
public:
   FitsSignatureImporter();
   virtual ~FitsSignatureImporter();
};

class FitsRasterPager : public CachedPager
{
public:
   FitsRasterPager();
   virtual ~FitsRasterPager();

private:
   FitsRasterPager& operator=(const FitsRasterPager& rhs);

   virtual bool openFile(const std::string& filename);
   virtual CachedPage::UnitPtr fetchUnit(DataRequest* pOriginalRequest);

   FitsFileResource mpFile;
};

#endif
