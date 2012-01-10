/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHIMPORTER_H
#define WAVELENGTHIMPORTER_H

#include "ExecutableShell.h"

#include <string>

class Wavelengths;

class WavelengthImporter : public ExecutableShell
{
public:
   WavelengthImporter();
   virtual ~WavelengthImporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool loadWavelengths(Wavelengths* pWavelengths, std::string& errorMessage) const = 0;
   const std::string& getFilename() const;

private:
   std::string mFilename;
};

#endif
