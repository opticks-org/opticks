/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "NitfConstants.h"
#include "NitfUnknownTreParser.h"

#include <ossim/support_data/ossimNitfRegisteredTag.h>

using namespace std;

const string Nitf::UnknownTreParser::PLUGIN_NAME = "Unknown Tre Parser";

Nitf::UnknownTreParser::UnknownTreParser()
{
   setName(PLUGIN_NAME);
   setDescriptorId("{12CF1BDF-F997-4428-9AED-6B076C3F036F}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

Nitf::UnknownTreParser::~UnknownTreParser()
{
   // Do nothing
}

bool Nitf::UnknownTreParser::toDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
   string &errorMessage) const
{
   ostringstream strm;
   const_cast<ossimNitfRegisteredTag&>(input).writeStream(strm);
   string unknownTagData = strm.str();
   output.setAttribute(TRE::UNPARSED_TAGS, unknownTagData);

   return true;
}

bool Nitf::UnknownTreParser::fromDynamicObject(const DynamicObject& input,
   ostream& output, size_t& numBytesWritten, string &errorMessage) const
{
   const string* pUnknownTagData = input.getAttribute(TRE::UNPARSED_TAGS).getPointerToValue<string>();
   if (pUnknownTagData == NULL)
   {
      return false;
   }

   output << *pUnknownTagData;
   numBytesWritten = pUnknownTagData->size();
   return true;
}

bool Nitf::UnknownTreParser::runAllTests(Progress *pProgress, ostream &failure)
{
   // nothing to test
   return true;
}
