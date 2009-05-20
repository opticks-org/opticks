/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AebPlatform.h"

AebPlatform::AebPlatform() : mPlatform(currentPlatform())
{
}

AebPlatform::AebPlatform(const std::string& platform) : mPlatform(platform)
{
}

AebPlatform::AebPlatform(const AebPlatform& other) : mPlatform(other.mPlatform)
{
}

bool AebPlatform::isValid() const
{
   return mPlatform == currentPlatform();
}

bool AebPlatform::operator==(const AebPlatform& other) const
{
   return mPlatform == other.mPlatform;
}

bool AebPlatform::operator!=(const AebPlatform& other) const
{
   return !operator==(other);
}

std::string AebPlatform::toString() const
{
   if (isValid())
   {
      return mPlatform;
   }
   return std::string();
}

std::string AebPlatform::currentPlatform()
{
   std::string platform;

   // Operating system
#if defined(_WIN64)
   platform += "win64";
#elif defined(_WIN32)
   platform += "win32";
#elif defined(SOLARIS)
   platform += "solaris";
#elif defined(LINUX)
   platform += "linux";
#else
   platform += "unknown";
#endif

   // Processor
#if defined(_M_IX86) || defined(_M_X64)
   platform += "-x86";
#elif defined(__sparc)
   platform += "-sparc";
#elif defined(LINUX) && defined(__amd64__)
   platform += "-x86_64";
#else
   platform += "-unknown";
#endif

   // Compiler
#if _MSC_VER == 1400
   platform += "-msvc8.1";
#elif __SUNPRO_CC == 0x580
   platform += "-studio11";
#elif __SUNPRO_CC == 0x590
   platform += "-studio12";
#elif __GNUC__ == 4
   platform += "-gcc4";
#else
   platform += "-unknown";
#endif

   // Debug/release
#if defined(DEBUG)
   platform += "-debug";
#else
   platform += "-release";
#endif
   return platform;
}
