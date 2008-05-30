/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

#include <cstdlib>
#include <hdf5.h>
#include <string>

#include "AppVersion.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "Hdf5Attribute.h"
#include "Hdf5Dataset.h"
#include "Hdf5File.h"
#include "Hdf5Group.h"
#include "Hdf5ImporterShell.h"
#include "Hdf5Pager.h"
#include "Hdf5Resource.h"
#include "Hdf5Utilities.h"
#include "ObjectResource.h"
#include "ProgressTracker.h"
#include "RasterElement.h"

using namespace std;
using namespace HdfUtilities;

Hdf5ImporterShell::Hdf5ImporterShell()
{
   setExtensions("HDF5 Files (*.h5)");
   addDependencyCopyright("Hdf5",
      "The version of Hdf5 distributed with " APP_NAME " has been modified from the original software "
      "provided by NCSA to support 64-bit Windows<br>"
      "<br><pre>"
      "Copyright Notice and Statement for NCSA Hierarchical Data Format (HDF)\n"
      "Software Library and Utilities\n"
      "\n"
      "NCSA HDF5 (Hierarchical Data Format 5) Software Library and Utilities \n"
      "Copyright 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 by the Board of \n"
      "Trustees of the University of Illinois.  All rights reserved.\n"
      "\n"
      "Contributors: National Center for Supercomputing Applications (NCSA) at the\n"
      "University of Illinois at Urbana-Champaign (UIUC), Lawrence Livermore \n"
      "National Laboratory (LLNL), Sandia National Laboratories (SNL), Los Alamos \n"
      "National Laboratory (LANL), Jean-loup Gailly and Mark Adler (gzip library).\n"
      "\n"
      "Redistribution and use in source and binary forms, with or without\n"
      "modification, are permitted for any purpose (including commercial purposes)\n"
      "provided that the following conditions are met:\n"
      "\n"
      "1.  Redistributions of source code must retain the above copyright notice,\n"
      "    this list of conditions, and the following disclaimer.\n"
      "\n"
      "2.  Redistributions in binary form must reproduce the above copyright notice,\n"
      "    this list of conditions, and the following disclaimer in the documentation\n"
      "    and/or materials provided with the distribution.\n"
      "\n"
      "3.  In addition, redistributions of modified forms of the source or binary\n"
      "    code must carry prominent notices stating that the original code was\n"
      "    changed and the date of the change.\n"
      "\n"
      "4.  All publications or advertising materials mentioning features or use of\n"
      "    this software are asked, but not required, to acknowledge that it was \n"
      "    developed by the National Center for Supercomputing Applications at the \n"
      "    University of Illinois at Urbana-Champaign and to credit the contributors.\n"
      "\n"
      "5.  Neither the name of the University nor the names of the Contributors may\n"
      "    be used to endorse or promote products derived from this software without\n"
      "    specific prior written permission from the University or the Contributors,\n"
      "    as appropriate for the name(s) to be used.\n"
      "\n"
      "6.  THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND THE CONTRIBUTORS \"AS IS\"\n"
      "    WITH NO WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED.  In no event\n"
      "    shall the University or the Contributors be liable for any damages\n"
      "    suffered by the users arising out of the use of this software, even if\n"
      "    advised of the possibility of such damage.\n"
      "\n"
      "--------------------------------------------------------------------------\n"
      "Portions of HDF5 were developed with support from the University of \n"
      "California, Lawrence Livermore National Laboratory (UC LLNL).\n"
      "The following statement applies to those portions of the product\n"
      "and must be retained in any redistribution of source code, binaries,\n"
      "documentation, and/or accompanying materials:\n"
      "\n"
      "    This work was partially produced at the University of California,\n"
      "    Lawrence Livermore National Laboratory (UC LLNL) under contract no.\n"
      "    W-7405-ENG-48 (Contract 48) between the U.S. Department of Energy \n"
      "    (DOE) and The Regents of the University of California (University) \n"
      "    for the operation of UC LLNL.\n"
      "\n"
      "    DISCLAIMER:\n"
      "    This work was prepared as an account of work sponsored by an agency \n"
      "    of the United States Government.  Neither the United States \n"
      "    Government nor the University of California nor any of their \n"
      "    employees, makes any warranty, express or implied, or assumes any \n"
      "    liability or responsibility for the accuracy, completeness, or \n"
      "    usefulness of any information, apparatus, product, or process \n"
      "    disclosed, or represents that its use would not infringe privately-\n"
      "    owned rights.  Reference herein to any specific commercial products, \n"
      "    process, or service by trade name, trademark, manufacturer, or \n"
      "    otherwise, does not necessarily constitute or imply its endorsement, \n"
      "    recommendation, or favoring by the United States Government or the \n"
      "    University of California.  The views and opinions of authors \n"
      "    expressed herein do not necessarily state or reflect those of the \n"
      "    United States Government or the University of California, and shall \n"
      "    not be used for advertising or product endorsement purposes.\n"
      "--------------------------------------------------------------------------</pre>");
}

bool Hdf5ImporterShell::getFileHandle(hid_t& fileHandle, Hdf5FileResource& fileResource) const
{
   fileHandle = -1;

   const RasterElement* pElement = getRasterElement();
   if (pElement != NULL)
   {
      Hdf5PagerFileHandle* pPager = dynamic_cast<Hdf5PagerFileHandle*>(pElement->getPager());
      if (pPager != NULL)
      {
         // Able to retrieve already open file from the pager
         fileHandle = pPager->getFileHandle();
      }
   }

   if (fileHandle < 0)
   {
      if (fileResource.get() == NULL)
      {
         // Not able to retrieve already open file handle; open the file and return.
         fileResource = Hdf5FileResource(pElement->getFilename());
      }

      if (*fileResource >= 0)
      {
         fileHandle = *fileResource;
      }
   }

   return (fileHandle >= 0);
}

bool Hdf5ImporterShell::createRasterPager(RasterElement *pRaster) const
{
   return HdfImporterShell::createRasterPagerPlugIn("Hdf5Pager", *pRaster);
}

