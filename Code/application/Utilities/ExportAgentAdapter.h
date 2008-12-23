/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXPORTAGENTADAPTER_H
#define EXPORTAGENTADAPTER_H

#include "ExportAgent.h"
#include "ExportAgentImp.h"

class ExportAgentAdapter : public ExportAgent, public ExportAgentImp
{
public:
   virtual ~ExportAgentAdapter()
   {
   }

   EXPORTAGENTADAPTER_METHODS(ExportAgentImp)
};

#endif
