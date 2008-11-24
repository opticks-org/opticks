/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef EXECUTABLEAGENTADAPTER_H
#define EXECUTABLEAGENTADAPTER_H

#include "ExecutableAgent.h"
#include "ExecutableAgentImp.h"

class ExecutableAgentAdapter : public ExecutableAgentCommon1, public ExecutableAgentImp
{
public:
   virtual ~ExecutableAgentAdapter()
   {
   }

   EXECUTABLEAGENTADAPTER_METHODS(ExecutableAgentImp)
};

#endif
