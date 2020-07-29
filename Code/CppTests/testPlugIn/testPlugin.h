/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __COMETtests_h__
#define __COMETtests_h__

#include "AlgorithmShell.h"


class COMET_tests : public AlgorithmShell
{
public:

    COMET_tests();

    ~COMET_tests();

public:  // AlgorithmShell methods

    bool isInputValid( PlugInArgList * );

    bool getInputSpecification( PlugInArgList *& );

    bool getOutputSpecification( PlugInArgList *& );

    bool execute( PlugInArgList *, PlugInArgList * );

    bool hasAbort() { return false; };

private:

    bool getInterfacePointers();

   // plug in attributes
   const int mNumPluginArgs;
   bool interactiveFlag;
   bool mbAbort;
   bool mbError;
};

#endif  // __COMETtests_h__
