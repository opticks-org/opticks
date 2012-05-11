/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHMETADATAEXPORTER_H
#define WAVELENGTHMETADATAEXPORTER_H

#include "WavelengthExporter.h"

class Wavelengths;

class WavelengthMetadataExporter : public WavelengthExporter
{
public:
   WavelengthMetadataExporter();
   virtual ~WavelengthMetadataExporter();

   static unsigned int WavelengthFileVersion()
   {
      return 1;
   }

protected:
   virtual bool saveWavelengths(const Wavelengths* pWavelengths) const;
};

#endif
