/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ExecutableShell.h"

using namespace std;

ExecutableShell::ExecutableShell() :
   mAborted(false),
   mExecuteOnStartup(false),
   mDestroyAfterExecute(true),
   mBatch(true),
   mHasAbort(false),
   mBackground(false),
   mHasWizard(true)
{
}

ExecutableShell::~ExecutableShell()
{
}

bool ExecutableShell::isExecutedOnStartup() const
{
   return mExecuteOnStartup;
}

bool ExecutableShell::isDestroyedAfterExecute() const
{
   return mDestroyAfterExecute;
}

const vector<string>& ExecutableShell::getMenuLocations() const
{
   return mMenuLocations;
}

bool ExecutableShell::setBatch()
{
   mBatch = true;
   return true;
}

bool ExecutableShell::setInteractive()
{
   mBatch = false;
   return true;
}

bool ExecutableShell::initialize()
{
   return true;
}

bool ExecutableShell::abort()
{
   mAborted = hasAbort();
   return mAborted;
}

bool ExecutableShell::hasAbort()
{
   return mHasAbort;
}

bool ExecutableShell::isBackground() const
{
   return mBackground;
}

bool ExecutableShell::isBatch() const
{
   return mBatch;
}

bool ExecutableShell::isAborted() const
{
   return mAborted;
}

bool ExecutableShell::hasWizardSupport() const
{
   return mHasWizard;
}

void ExecutableShell::executeOnStartup(bool bExecute)
{
   mExecuteOnStartup = bExecute;
}

void ExecutableShell::destroyAfterExecute(bool bDestroy)
{
   mDestroyAfterExecute = bDestroy;
}

void ExecutableShell::setMenuLocation(const string& menuLocation)
{
   mMenuLocations.clear();
   addMenuLocation(menuLocation);
}

void ExecutableShell::addMenuLocation(const string& menuLocation)
{
   mMenuLocations.push_back(menuLocation);
}

void ExecutableShell::setAbortSupported(bool bSupported)
{
   mHasAbort = bSupported;
}

void ExecutableShell::setBackground(bool bBackground)
{
   mBackground = bBackground;
}

void ExecutableShell::setWizardSupported(bool bSupported)
{
   mHasWizard = bSupported;
}

