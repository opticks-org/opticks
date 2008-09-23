/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Mpeg1Exporter.h"
#include "AppVersion.h"

Mpeg1Exporter::Mpeg1Exporter()
{
   setName("MPEG-1 Product Exporter");
   setExtensions("MPEG-1 Movie Files (*.mpg *.mpeg)");
   setShortDescription("Export MPEG-1 Movie Products");
   setDescriptorId("{AB09D92D-DECD-4668-8980-A59A3C446BCD}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Mpeg1Exporter::~Mpeg1Exporter()
{
}

AVOutputFormat *Mpeg1Exporter::getOutputFormat() const
{
   return guess_format("mpeg", NULL, NULL);
}
