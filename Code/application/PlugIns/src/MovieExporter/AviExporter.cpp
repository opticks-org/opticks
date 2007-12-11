/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AviExporter.h"
#include "AppVersion.h"

AviExporter::AviExporter()
{
   setName("AVI Product Exporter");
   setExtensions("AVI Movie Files (*.avi)");
   setShortDescription("Export AVI Movie Products");
   setDescriptorId("{B8281902-85B9-4037-A348-01C1DDB7D8CF}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AviExporter::~AviExporter()
{
}

AVOutputFormat *AviExporter::getOutputFormat() const
{
   return guess_format("avi", NULL, NULL);
}
