/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef TEST_DATA_PATH_H
#define TEST_DATA_PATH_H

#include "AppConfig.h"

#include <string>

static std::string getTestDataPath()
{
#if defined(WIN_API)
   static std::string testFilePath = std::string("T:/cppTestData/");
#else
   static std::string testFilePath = std::string("/TestData/cppTestData/");
#endif
   return testFilePath;
}

#endif
