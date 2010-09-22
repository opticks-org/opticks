/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "Filename.h"
#include "TestUtilities.h"

std::string TestUtilities::getTestDataPath()
{
   std::string testDataPath;

   const Filename* pPath = ConfigurationSettings::getSettingImportPath();
   if (pPath != NULL)
   {
      testDataPath = pPath->getFullPathAndName() + "/";
   }

   return testDataPath;
}

bool TestUtilities::assertionLogger(std::ostream& output, const std::string& expression, const std::string& file,
                                    int line)
{
   output << "Assertion failed: '" << expression << "', File: " << file << ", Line: " << line << std::endl;
   return false;
}
