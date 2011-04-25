/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFIMPORTER_H
#define NITFIMPORTER_H

#include "NitfImporterShell.h"
#include "Testable.h"

namespace Nitf
{
   class NitfImporter : public Nitf::NitfImporterShell, public Testable
   {
   public:
      NitfImporter();
      virtual ~NitfImporter();

      virtual bool runOperationalTests(Progress* pProgress, std::ostream& failure);
      virtual bool runAllTests(Progress* pProgress, std::ostream& failure);
   };
}
#endif
