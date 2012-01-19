/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "NitfImporter.h"
#include "NitfUtilities.h"
#include "PlugInRegistration.h"
#include "Progress.h"

#include <sstream>
#include <vector>

REGISTER_PLUGIN(OpticksNitf, NitfImporter, Nitf::NitfImporter);

using namespace std;

Nitf::NitfImporter::NitfImporter()
{
   setName("NITF Importer");
   setDescriptorId("{2130D292-2647-4e98-BEF1-BA743234148C}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::NitfImporter::~NitfImporter()
{}

bool Nitf::NitfImporter::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return runAllTests(pProgress, failure);
}

bool Nitf::NitfImporter::runAllTests(Progress* pProgress, ostream& failure)
{
   FactoryResource<DynamicObject> pMetadata;
   vector<double> centerConst;
   centerConst.push_back(10);
   centerConst.push_back(20);
   centerConst.push_back(30);

   vector<double> startConst;
   startConst.push_back(5);
   startConst.push_back(19.75);
   startConst.push_back(26.85);

   vector<double> endConst;
   endConst.push_back(15);
   endConst.push_back(20.25);
   endConst.push_back(33.15);

   vector<double> fwhmConst;
   fwhmConst.push_back(10);
   fwhmConst.push_back(.5);
   fwhmConst.push_back(6.3);

   vector<double> center;
   vector<double> start;
   vector<double> end;
   vector<double> fwhm;

   // center/fwhm --> start/end
   center = centerConst;
   start.clear();
   end.clear();
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (start != startConst || end != endConst)
   {
      failure << "Failed to compute start/end wavelengths from center/fwhm" << endl;
      return false;
   }

   // center/start --> end
   center = centerConst;
   start = startConst;
   end.clear();
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (end != endConst)
   {
      failure << "Failed to compute end wavelengths from center/start" << endl;
      return false;
   }

   // center/end --> start
   center = centerConst;
   start.clear();
   end = endConst;
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (start != startConst)
   {
      failure << "Failed to compute start wavelengths from center/end" << endl;
      return false;
   }

   // start/fwhm --> center/end
   center.clear();
   start = startConst;
   end.clear();
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst || end != endConst)
   {
      failure << "Failed to compute center/end wavelengths from start/fwhm" << endl;
      return false;
   }

   // start/end --> center
   center.clear();
   start = startConst;
   end = endConst;
   fwhm.clear();
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst)
   {
      failure << "Failed to compute center wavelengths from start/end" << endl;
      return false;
   }

   // end/fwhm --> center/start
   center.clear();
   start.clear();
   end = endConst;
   fwhm = fwhmConst;
   updateSpecialMetadata(pMetadata.get(), center, start, end, fwhm);
   if (center != centerConst || start != startConst)
   {
      failure << "Failed to compute center/start wavelengths from end/fwhm" << endl;
      return false;
   }

   return true;
}
