/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ALGORITHMSHELL_H
#define ALGORITHMSHELL_H

#include "ExecutableShell.h"

/**
 *  Algorithm Shell
 *
 *  This class represents the shell for an algorithm plug-in.  Algorithm
 *  developers would take this class and extend it to support their algorithm
 *  specific code.
 *
 *  @see     ExecutableShell
 */
class AlgorithmShell : public ExecutableShell
{
public:
   /**
    *  Creates an algorithm plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::AlgorithmType().
    *
    *  @see     getType()
    */
   AlgorithmShell();

   /**
    *  Destroys the algorithm plug-in.
    */
   ~AlgorithmShell();
};

#endif
