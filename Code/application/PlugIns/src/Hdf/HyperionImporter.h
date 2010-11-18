/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HYPERIONIMPORTER_H
#define HYPERIONIMPORTER_H

#include "AppConfig.h"

#include "Hdf4ImporterShell.h"
#include "Testable.h"

/**
 * The purpose of this class is to illustrate how to load a data cube that also
 * has wavelength data encoded as Hdf4Dataset for the starts, centers, and ends.
 * This importer shows how to use the HdfUtilities to extract information from
 * an Hdf4Attribute and convert it to the application's native data model (ie.
 * converting an %Hdf4Attribute to a wavelength vector and storing it in the
 * DatasetParameters).
 *
 * See help in Hdf4ImporterShell on how to build this sample plugin.
 */
class HyperionImporter : public Hdf4ImporterShell, public Testable
{
public:
   /**
    *  Constructs the HyperionImporter object.
    */
   HyperionImporter();

   /**
    *  Destroys the HyperionImporter object.
    */
   ~HyperionImporter();

   /**
    * Tests the HyperionImporter
    */
   bool runOperationalTests(Progress* pProgress, std::ostream& failure);
   bool runAllTests(Progress* pProgress, std::ostream& failure);

   unsigned char getFileAffinity(const std::string& filename);

private:
   std::vector<ImportDescriptor*> getImportDescriptors(const std::string& filename);
};

#endif   // HYPERIONIMPORTER_H
