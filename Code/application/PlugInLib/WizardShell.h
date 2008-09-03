/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WIZARDSHELL_H
#define WIZARDSHELL_H

#include "ExecutableShell.h"

/**
 *  Wizard Shell
 *
 *  This class represents the shell for a wizard plug-in.  Wizard
 *  developers would take this class and extend it to support their
 *  wizard specific code.
 *
 *  @see     ExecutableShell
 */
class WizardShell : public ExecutableShell
{
public:
   /**
    *  Creates a wizard plug-in.
    *
    *  The constructor sets the plug-in type to PlugInManagerServices::WizardType().
    *
    *  @see     getType()
    */
   WizardShell();

   /**
    *  Destroys the wizard plug-in.
    */
   ~WizardShell();

   /**
    *  @copydoc Executable::getOutputSpecification()
    *
    *  @default The default implementation does not set any args in the arg
    *           list and returns \b true.
    */
   bool getOutputSpecification(PlugInArgList*& pArgList);
};

#endif
