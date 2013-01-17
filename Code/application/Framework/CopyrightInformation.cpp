/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "CopyrightInformation.h"
#include "UtilityServices.h"

CopyrightInformation::CopyrightInformation()
{
   setName("Copyright Information");
   setType("Copyright Information");
   setDescription("Provides copyright and license information for application dependencies.");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT_MSG);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setDescriptorId("{3FA82540-856C-43e3-8B8D-232409A7C47A}");
   addDependencyCopyright("Boost", Service<UtilityServices>()->getTextFromFile(":/licenses/boost"));
   addDependencyCopyright("Cg", Service<UtilityServices>()->getTextFromFile(":/licenses/cg"));
   addDependencyCopyright("Expat", Service<UtilityServices>()->getTextFromFile(":/licenses/expat"));
   addDependencyCopyright("EHS", Service<UtilityServices>()->getTextFromFile(":/licenses/ehs"));
   addDependencyCopyright("FFmpeg", Service<UtilityServices>()->getTextFromFile(":/licenses/ffmpeg"));
   addDependencyCopyright("GLEW", Service<UtilityServices>()->getTextFromFile(":/licenses/glew"));
   addDependencyCopyright("IJG", Service<UtilityServices>()->getTextFromFile(":/licenses/ijg"));
   addDependencyCopyright("MiniZip", Service<UtilityServices>()->getTextFromFile(":/licenses/minizip"));
   addDependencyCopyright("OSSIM", Service<UtilityServices>()->getTextFromFile(":/licenses/ossim"));
   addDependencyCopyright("proj4", Service<UtilityServices>()->getTextFromFile(":/licenses/proj4"));
   addDependencyCopyright("PCRE", Service<UtilityServices>()->getTextFromFile(":/licenses/pcre"));
   addDependencyCopyright("Pthreads", Service<UtilityServices>()->getTextFromFile(":/licenses/pthreads"));
   addDependencyCopyright("Qt", Service<UtilityServices>()->getTextFromFile(":/licenses/qt"));
   addDependencyCopyright("Qwt", Service<UtilityServices>()->getTextFromFile(":/licenses/qwt"));
   addDependencyCopyright("Raptor", Service<UtilityServices>()->getTextFromFile(":/licenses/raptor"));
   addDependencyCopyright("Splash Screen", "The photo in the Opticks splash screen is copyright Andrew Davidhazy "
      "(http://www.rit.edu/~andpph) and is used with permission.");
   addDependencyCopyright("TBB", Service<UtilityServices>()->getTextFromFile(":/licenses/tbb"));
   addDependencyCopyright("Xerces", Service<UtilityServices>()->getTextFromFile(":/licenses/xerces"));
   addDependencyCopyright("XQilla", Service<UtilityServices>()->getTextFromFile(":/licenses/xqilla"));
   addDependencyCopyright("yaml-cpp", Service<UtilityServices>()->getTextFromFile(":/licenses/yaml-cpp"));
   addDependencyCopyright("zlib", Service<UtilityServices>()->getTextFromFile(":/licenses/zlib"));
}
