/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef APPASSERT_H
#define APPASSERT_H

#include <string>

class AssertException
{
public:
   explicit AssertException (const char *pText) : mText(pText) {}
   explicit AssertException (const std::string &str) : mText(str.c_str()) {}
   std::string getText() const { return mText; }
private:
   std::string mText;
};

extern void AppAssert(const char *pExpression, const char *pFilename, unsigned line);

#define APP_ASSERT(test) ((test)?(void)0 : AppAssert(#test, __FILE__, __LINE__))

#define REQUIRE(test) APP_ASSERT(test)
#define ENSURE(test) APP_ASSERT(test)
#define INVARIANT(test) APP_ASSERT(test)

#ifdef NDEBUG
#define REQUIRE_DEBUG(test)
#define ENSURE_DEBUG(test)
#define INVARIANT_DEBUG(test)
#else
#define REQUIRE_DEBUG(test) REQUIRE(test)
#define ENSURE_DEBUG(test) ENSURE(test)
#define INVARIANT_DEBUG(test) INVARIANT(test)
#endif

#endif

 
