/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CUSTOMELEMENTPLUGIN_H
#define CUSTOMELEMENTPLUGIN_H

#include "AlgorithmShell.h"

class CustomElementPlugIn : public AlgorithmShell
{
public:
   CustomElementPlugIn();
   ~CustomElementPlugIn();

   bool getInputSpecification(PlugInArgList*& pArgList);
   bool getOutputSpecification(PlugInArgList*& pArgList);
   bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   bool serialize(SessionItemSerializer &serializer) const;
   bool deserialize(SessionItemDeserializer & deserializer);
};

#endif
