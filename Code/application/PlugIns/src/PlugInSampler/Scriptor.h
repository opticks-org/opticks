/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SCRIPTOR_H
#define SCRIPTOR_H

#include "InterpreterShell.h"

#include <string>

class Scriptor : public InterpreterShell
{
public:
   Scriptor();
   virtual ~Scriptor() {}

   virtual std::string getPrompt() const;
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   virtual bool serialize(SessionItemSerializer &serializer) const;
   virtual bool deserialize(SessionItemDeserializer &deserializer);

private:
   std::string mCommand;
   unsigned int mCommandNumber;
};

#endif
