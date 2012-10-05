/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLLADAUTIL_H
#define COLLADAUTIL_H

#include "AppConfig.h"
#if defined(OPENCOLLADA_SUPPORT)

#include <string>

namespace ColladaUtilities
{
   inline std::string getAnnotationSuffix()
   {
      return "-opticksAnno";
   }

   inline std::string getBasicSceneName()
   {
      return "opticks-scene";
   }

   inline std::string formatGeometryId(std::string geometryId)
   {
      return geometryId + getAnnotationSuffix();
   }
}

#endif
#endif