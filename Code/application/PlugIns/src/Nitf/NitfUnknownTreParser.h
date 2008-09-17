/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef NITFUNKNOWNTREPARSER_H
#define NITFUNKNOWNTREPARSER_H

#include "NitfTreParserShell.h"
#include <string>

namespace Nitf
{
   class UnknownTreParser : public TreParserShell
   {
   public:
      static const std::string PLUGIN_NAME;

      UnknownTreParser();
      ~UnknownTreParser();

      bool runAllTests(Progress *pProgress, std::ostream &failure);

      bool toDynamicObject(const ossimNitfRegisteredTag& input, DynamicObject& output, 
         std::string &errorMessage) const;
      bool fromDynamicObject(const DynamicObject& input, std::ostream& output, size_t& numBytesWritten, 
         std::string &errorMessage) const;

   };
}

#endif