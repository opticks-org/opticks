/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <string>

using namespace std;

static string getAnnotationSuffix()
{
   return "-opticksAnno";
}

static string getBasicSceneName()
{
   return "opticks-scene";
}

static string formatGeometryId(string geometryId)
{
   return geometryId + getAnnotationSuffix();
}