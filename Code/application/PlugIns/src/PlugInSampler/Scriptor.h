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
#include "PlugInManagerServices.h"

#include <string>
#include <vector>

class Scriptor : public InterpreterShell
{
public:
   Scriptor();
   ~Scriptor();

   //----- Inherited obligations
   bool setBatch();
   bool setInteractive();
   bool hasAbort();
   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer &deserializer);

   //----- Inherited obligations from InterpreterShell
   void getKeywordList(std::vector<std::string>& list) const;
   bool getKeywordDescription(const std::string& keyword, std::string& description) const;
   void getUserDefinedTypes(std::vector<std::string>& list) const;
   bool getTypeDescription(const std::string& type, std::string& description) const;

   //----- Inherited obligations
   bool isBackground() const;

protected:
   bool extractInputArgs(PlugInArgList* pArgList);

private:
   Service<PlugInManagerServices> mpPlugInManager;
   std::string mCommand;
};

#endif
