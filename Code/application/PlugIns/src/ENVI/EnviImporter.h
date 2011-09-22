/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENVIIMPORTER_H
#define ENVIIMPORTER_H

#include "EnumWrapper.h"
#include "EnviField.h"
#include "RasterElementImporterShell.h"

#include <string>
#include <vector>

class EnviImporter : public RasterElementImporterShell
{
public:
   EnviImporter();
   virtual ~EnviImporter();

   virtual std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
   virtual unsigned char getFileAffinity(const std::string& filename);
   virtual bool validate(const DataDescriptor* pDescriptor, std::string& errorMessage) const;
   static bool parseBbl(EnviField* pField, std::vector<unsigned int>& goodBands);

protected:
   virtual int getValidationTest(const DataDescriptor* pDescriptor) const;

   enum WavelengthUnitsEnum
   {
      WU_UNKNOWN,
      WU_MICROMETERS,
      WU_NANOMETERS,
      WU_WAVENUMBER,
      WU_GHZ,
      WU_MHZ,
      WU_INDEX
   };
   typedef EnumWrapper<WavelengthUnitsEnum> WavelengthUnitsType;

   WavelengthUnitsType strToType(std::string strType);

   bool parseHeader(const std::string& filename);
   bool parseWavelengths(EnviField* pField, std::vector<double>* pWavelengthCenters);
   bool parseFwhm(EnviField* pField, std::vector<double>* pWavelengthStarts,
      const std::vector<double>* pWavelengthCenters, std::vector<double>* pWavelengthEnds);
   std::string findDataFile(const std::string& headerPath);
   std::string findHeaderFile(const std::string& dataPath);

private:
   EnviField mFields;
   WavelengthUnitsType mWavelengthUnits;
};

#endif
