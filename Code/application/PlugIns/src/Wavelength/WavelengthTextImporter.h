/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHTEXTIMPORTER_H
#define WAVELENGTHTEXTIMPORTER_H

#include "TypesFile.h"
#include "WavelengthImporter.h"

class Wavelengths;

class WavelengthTextImporter : public WavelengthImporter
{
public:
   WavelengthTextImporter();
   virtual ~WavelengthTextImporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool loadWavelengths(Wavelengths* pWavelengths, std::string& errorMessage) const;

private:
   WavelengthUnitsType mUnits;
};

#endif
