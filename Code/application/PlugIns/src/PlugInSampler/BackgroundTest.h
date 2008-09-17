/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BACKGROUNDTEST_H
#define BACKGROUNDTEST_H

#include "AlgorithmShell.h"
#include "PlugInCallback.h"
#include "PlugInManagerServices.h"
#include "UtilityServices.h"

class BThread;
class Progress;

class BackgroundTest : public AlgorithmShell
{
public:
   BackgroundTest();
   virtual ~BackgroundTest();

   virtual bool setBatch();
   virtual bool setInteractive();
   virtual bool getInputSpecification(PlugInArgList*& pArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pArgList);
   virtual bool execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList);
   virtual bool abort();
   virtual bool hasAbort();
   virtual bool isBackground() const;

   static void runWorkerThread(void *pArg);

protected:
   bool workerThread();

   class Callback : public PlugInCallback
   {
   public:
      Callback(BackgroundTest *pPlugin, BThread *pThread, bool returnValue, Progress *pProgress);
      virtual ~Callback();
      virtual void operator()();
      virtual bool finish();
      virtual PlugIn *getPlugIn() const;
      virtual Progress *progress() const;

   private:
      BackgroundTest *mpPlugin;
      BThread *mpThread;
      bool mReturnValue;
      Progress *mpProgress;
   };

private:
   bool mInteractive;
   bool mAbort;
   Service<PlugInManagerServices> mpPlugInManager;
   Service<UtilityServices> mpUtility;

   Progress *mpProgress;
   BThread *mpWorkerThread;
};

#endif
