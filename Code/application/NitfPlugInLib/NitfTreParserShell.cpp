/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "NitfTreParserShell.h"

#include <string>
using namespace std;
using namespace Nitf;

Nitf::TreParserShell::TreParserShell()
{
   setType(Type());
   setSubtype(NormalSubtype());
}

Nitf::TreParserShell::~TreParserShell()
{
}

string Nitf::TreParserShell::getShortDescription() const
{
   string shortDescription = "Parses a NITF " + getName() + " Tagged Record Extension (TRE).";
   return shortDescription;
}

bool Nitf::TreParserShell::runOperationalTests(Progress* pProgress, ostream& failure)
{
   return true;
}

TreState Nitf::TreParserShell::isTreValid(const DynamicObject& tre, ostream& reporter) const
{
   return UNTESTED;
}

bool Nitf::TreParserShell::ossimTagToDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
   string &errorMessage) const
{
   return false;
}

bool Nitf::TreParserShell::toDynamicObject(istream& input, size_t numBytes, DynamicObject& output, 
   string &errorMessage) const
{
   return false;
}

bool Nitf::TreParserShell::fromDynamicObject(const DynamicObject& input, ostream& output, size_t& numBytesWritten, 
   string &errorMessage) const
{
   return false;
}

bool Nitf::TreParserShell::ossimTagFromDynamicObject(const DynamicObject& input, ossimNitfRegisteredTag& tre, 
   string &errorMessage) const
{
   return false;
}

bool Nitf::TreParserShell::importMetadata(const DynamicObject &tre, RasterDataDescriptor &descriptor, 
   string &errorMessage) const
{
   return false;
}

Nitf::TreExportStatus Nitf::TreParserShell::exportMetadata(const RasterDataDescriptor &descriptor, 
   const RasterFileDescriptor &exportDescriptor, DynamicObject &tre,
   unsigned int &ownerIndex, string &tagType, string &errorMessage) const
{
   return UNCHANGED;
}
