/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MESSAGELOGTEST_H__
#define MESSAGELOGTEST_H__

#include "AlgorithmShell.h"

class MessageLogTestPlugin : public AlgorithmShell
{
public:
   MessageLogTestPlugin();
   ~MessageLogTestPlugin();

public:
   bool getInputSpecification( PlugInArgList *& );
   bool getOutputSpecification( PlugInArgList *& );
   bool execute( PlugInArgList *, PlugInArgList * );
};

#endif