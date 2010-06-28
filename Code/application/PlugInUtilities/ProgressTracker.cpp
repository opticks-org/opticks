/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProgressTracker.h"

using namespace std;

//----------------- ProgressTracker::Stage -------------------//

ProgressTracker::Stage::Stage(const string &message, const string &component, const string &key, int weight) :
            mMessage(message),
            mComponent(component),
            mKey(key),
            mWeight(weight),
            mpParent(NULL),
            mCurrentSubStage(mSubStages.begin()),
            mSubStageWeight(0),
            mWeightComplete(0),
            mpStep(NULL)
{
}

ProgressTracker::Stage::~Stage()
{
}

ProgressTracker::Stage* ProgressTracker::Stage::getParent() const
{
   return mpParent;
}

Step *ProgressTracker::Stage::getStep()
{
   return mpStep.get();
}

void ProgressTracker::Stage::initialize(Stage *pParent)
{
   mpParent = pParent;
   if (pParent == NULL)
   {
      mpStep = StepResource(mMessage, mComponent, mKey);
   }
   else
   {
      Step* pStep = pParent->getStep();
      if (pStep != NULL)
      {
         mpStep = StepResource(pStep->addStep(mMessage, mComponent, mKey, false));
      }
   }
}

void ProgressTracker::Stage::finalize(Message::Result result, const std::string &failureReason)
{
   if (mpStep.get() != NULL)
   {
      mpStep->finalize(result, failureReason);
   }
}

void ProgressTracker::Stage::subdivide(vector<ProgressTracker::Stage>& stages)
{
   vector<ProgressTracker::Stage>::iterator pStage;
   int totalWeight = 0;
   for (pStage = stages.begin(); pStage != stages.end(); ++pStage)
   {
      pStage->initialize(this);
      totalWeight += pStage->mWeight;
   }
   mSubStages = stages;
   mCurrentSubStage = mSubStages.begin();
   mSubStageWeight = totalWeight;
}

void ProgressTracker::Stage::toNextSubStage()
{
   mCurrentSubStage->finalize(Message::Success);
   vector<ProgressTracker::Stage>::iterator currentSubStage = mCurrentSubStage;
   ++currentSubStage;
   if (currentSubStage != mSubStages.end())
   {
      mWeightComplete += mCurrentSubStage->mWeight;
      mCurrentSubStage = currentSubStage;
   }
}

ProgressTracker::Stage &ProgressTracker::Stage::getActiveStage()
{
   if (!mSubStages.empty() && (mCurrentSubStage != mSubStages.end()))
   {
      return mCurrentSubStage->getActiveStage();
   }
   else
   {
      return *this;
   }
}

int ProgressTracker::Stage::getPercentComplete(int percent) const
{
   if (!mSubStages.empty() && (mCurrentSubStage != mSubStages.end()))
   {
      int percentComplete = 100 * mWeightComplete + 
         mCurrentSubStage->getPercentComplete(percent) *  mCurrentSubStage->mWeight;
      return percentComplete / mSubStageWeight;
   }

   return percent;
}

string ProgressTracker::Stage::getActiveMessage(string message, string indent) const
{
   indent = "-" + indent;
   if (!mSubStages.empty() && (mCurrentSubStage != mSubStages.end()))
   {
      return mMessage + "\n" + indent + mCurrentSubStage->getActiveMessage(message, indent);
   }

   return mMessage + "\n" + indent + message;
}

//------------------------- ProgressTracker -----------------------------------------//

void ProgressTracker::initialize(Progress *pProgress, const string &message,
                                 const string &component, const string &key)
{ 
   mpProgress = pProgress; 
   mMainStage = Stage(message, component, key, 1);
   mMainStage.initialize(NULL);
}

void ProgressTracker::subdivideCurrentStage(vector<Stage>& stages)
{
   Stage& currentStage = mMainStage.getActiveStage();
   currentStage.subdivide(stages);
}

void ProgressTracker::upALevel()
{
   Stage& currentStage = mMainStage.getActiveStage();
   Stage* pParentStage = currentStage.getParent();
   currentStage.finalize(Message::Success);
   if (pParentStage != NULL)
   {
      vector<Stage> stages;
      pParentStage->subdivide(stages);
   }
}

void ProgressTracker::nextStage()
{
   Stage& currentStage = mMainStage.getActiveStage();
   Stage* pParentStage = currentStage.getParent();
   if (pParentStage != NULL)
   {
      pParentStage->toNextSubStage();
   }
}

void ProgressTracker::report(const string &text, int percent, ReportingLevel gran, bool log)
{
   static int oldPercent = -1;
   static Stage* pOldStage = NULL;
   Stage& currentStage = mMainStage.getActiveStage();
   if (mpProgress != NULL)
   {
      if ((&currentStage != pOldStage) || (percent != oldPercent) || (percent == 100) || (percent == 0))
      {
         string overallMessage = mMainStage.getActiveMessage(text, ">");
         int overallPercent = mMainStage.getPercentComplete(percent);
         mpProgress->updateProgress(overallMessage, overallPercent, gran);

         pOldStage = &currentStage;
         oldPercent = percent;
      }
   }
   if (log && currentStage.getStep() != NULL)
   {
      switch (gran)
      {
      case ABORT:
         currentStage.getStep()->finalize(Message::Abort, text);
         break;
      case ERRORS:
         currentStage.getStep()->finalize(Message::Failure, text);
         break;
      default:
         currentStage.getStep()->addMessage(text, "app", "767D7B85-657E-471b-B6FF-440483CCF912", true);
         break;
      }
   }
}

void ProgressTracker::abort()
{
   mMainStage.getStep()->finalize(Message::Abort, "Progress Tracker Aborted");
   throw AlgorithmAbort("Algorithm Aborted.");
}

Step *ProgressTracker::getCurrentStep()
{
   return mMainStage.getActiveStage().getStep();
}
