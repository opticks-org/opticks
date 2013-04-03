/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHMETADATAIMPORTER_H
#define WAVELENGTHMETADATAIMPORTER_H

#include "WavelengthImporter.h"

class Wavelengths;

class WavelengthMetadataImporter : public WavelengthImporter
{
public:
   WavelengthMetadataImporter();
   virtual ~WavelengthMetadataImporter();

protected:
   virtual bool loadWavelengths(Wavelengths* pWavelengths, std::string& errorMessage) const;
};

#endif
