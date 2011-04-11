/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELENGTHEXPORTER_H
#define WAVELENGTHEXPORTER_H

#include "ExecutableShell.h"

#include <string>

class Progress;
class Step;
class Wavelengths;

class WavelengthExporter : public ExecutableShell
{
public:
   WavelengthExporter();
   virtual ~WavelengthExporter();

   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

protected:
   virtual bool extractInputArgs(PlugInArgList* pArgList);
   virtual bool saveWavelengths(const Wavelengths* pWavelengths) const = 0;
   Progress* getProgress() const;
   const std::string& getFilename() const;

private:
   Step* mpStep;
   Progress* mpProgress;
   Wavelengths* mpWavelengths;
   std::string mFilename;
};

#endif
