/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GDALRASTERPAGER_H__
#define GDALRASTERPAGER_H__

#include "CachedPager.h"
#include <gdal_priv.h>

class GdalRasterPager : public CachedPager
{
public:
   GdalRasterPager();
   virtual ~GdalRasterPager();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool parseInputArgs(PlugInArgList* pInputArgList);

private:
   virtual bool openFile(const std::string& filename);
   virtual CachedPage::UnitPtr fetchUnit(DataRequest* pOriginalRequest);

   std::auto_ptr<GDALDataset> mpDataset;
   std::string mDatasetName;
};

#endif
