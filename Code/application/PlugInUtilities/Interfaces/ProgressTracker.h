/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROGRESSTRACKER_H
#define PROGRESSTRACKER_H

#include <vector>
#include <string>

#include "MessageLogResource.h"
#include "Progress.h"

/**
 *  Exception class for algorithm aborts.
 */
class AlgorithmAbort
{
public:
   /**
    *  Construct a new %AlgorithmAbort with a specified message.
    *
    *  @param message
    *         The message to associate with this exception.
    */
   AlgorithmAbort(std::string message) :
      mMessage(message)
   {
   }

   /**
    *  Access the message.
    *
    *  @return The message associated with this exception.
    */
   std::string getText() const
   {
      return mMessage;
   }

private:
   std::string mMessage;
};

/**
 *  Automatically track progress basic on predefined steps.
 *
 *  @note There are short term plans to replace ProgressTracker
 *        with a similar system that functions more like StepResource.
 *        Keep watching the release notes and the developer's mailing
 *        list for announcements.
 */
class ProgressTracker
{
public:
   /**
    *  This is a stage in a progress track.
    */
   class Stage
   {
   public:
      /**
       *  Construct an empty stage.
       *  This is not usually called and exists as an
       *  implementation detail of the %ProgressTracker
       */
      Stage() : mWeight(-1), mpParent(NULL), mSubStageWeight(-1), mWeightComplete(-1), mpStep(NULL) {}

      /**
       *  Construct a stage.
       *
       *  @param message
       *         This is the message associated with this stage.
       *
       *  @param component
       *         The component for the message log %Step. See Message.
       *
       *  @param key
       *         The key for the message log %Step. See Message.
       *
       *  @param weight
       *         A multiplier specifying how much weight the %Stage holds compared to its siblings.
       */
      Stage(const std::string &message, const std::string &component, const std::string &key, int weight = 1);

      /**
       *  Destructor.
       */
      ~Stage();

      /**
       *  Accessor for the parent of this %Stage.
       *
       *  @return A pointer to the parent or NULL if this is the root %Stage.
       */
      Stage* getParent() const;

      /**
       *  Accessor for the %MessageLog %Step associated with this stage.
       *
       *  @return The %Step associated with this %Stage.
       */
      Step *getStep();

      /**
       *  Set this %Stage as a child of \a pParent.
       *  This is called internally by %ProgressTracker.
       *
       *  @param pParent
       *         The parent of this %Stage
       */
      void initialize(Stage *pParent);

      /**
       *  Finalize a &Stage.
       *  This is called internalled by %ProgressTracker.
       *
       *  @param result
       *         The result of the %Stage's execution. See Message.
       *
       *  @param failureReason
       *         The optional reason for failure of the %Stage.
       */
      void finalize(Message::Result result, const std::string &failureReason = "");

      /**
       *  Subdivide a %Stage into a series of sub-stages.
       *
       *  @param stages
       *         The vector of sub-stages.
       */
      void subdivide(std::vector<Stage>& stages);

      /**
       *  Advance to the next sub-stage.
       */
      void toNextSubStage();

      /**
       *  Return the active %Stage.
       *
       *  @return The active sub-stage or this %Stage if there is no active sub-stage.
       */
      Stage &getActiveStage();

      /**
       *  Get the percentage complete of this %Stage.
       *
       *  @param percent
       *         This will be returned if there are no sub-stages.
       *
       *  @return The percentage complete.
       */
      int getPercentComplete(int percent) const;

      /**
       *  Get the message associated with this %Stage and all sub-stages.
       *
       *  @param message
       *         This will be prefixed to the messages of sub-stages.
       *
       *  @param indent
       *         This string is used to indent messages of sub-stages.
       *
       *  @return A composite message calculated from sub-stages.
       */
      std::string getActiveMessage(std::string message, std::string indent) const;

   private:
      std::string mMessage;
      std::string mComponent;
      std::string mKey;
      int mWeight;
      mutable Stage* mpParent;
      std::vector<Stage> mSubStages;
      std::vector<Stage>::iterator mCurrentSubStage;
      int mSubStageWeight;
      int mWeightComplete;
      StepResource mpStep;
   };

   /**
    *  Construct an empty progress tracker.
    */
   ProgressTracker() : mpProgress(NULL) {}

   /**
    *  Construct a progress tracker with a %Progress object and %Step message.
    *  This convenience constructor is equivalent to constructing an empty
    *  progress tracker then calling initialize().
    *
    *  @param pProgress
    *         The %Progress object associated with this tracker.
    *
    *  @param message
    *         The message of the root %Stage.
    *
    *  @param component
    *         The component associated with the root message log %Step.
    *
    *  @param key
    *         The key associated with the root message log %Step.
    */
   ProgressTracker(Progress *pProgress, const std::string &message,
                   const std::string &component, const std::string &key)
   {
      initialize(pProgress, message, component, key);
   }

   /**
    *  Initialize a tracker.
    *
    *  @param pProgress
    *         The %Progress object associated with this tracker.
    *
    *  @param message
    *         The message of the root %Stage.
    *
    *  @param component
    *         The component associated with the root message log %Step.
    *
    *  @param key
    *         The key associated with the root message log %Step.
    */
   void initialize(Progress *pProgress, const std::string &message,
                   const std::string &component, const std::string &key);
   
   /**
    *  Initialize a tracker using the existing Progress object.
    *
    *  @param message
    *         The message of the root %Stage.
    *
    *  @param component
    *         The component associated with the root message log %Step.
    *
    *  @param key
    *         The key associated with the root message log %Step.
    */
   void initialize(const std::string &message, const std::string &component, const std::string &key)
   {
      initialize(mpProgress, message, component, key);
   }

   /**
    *  Subdivide the current &Stage into sub-stages.
    *
    *  @param stages
    *         The sub-stage vector.
    */
   void subdivideCurrentStage(std::vector<Stage>& stages);

   /**
    *  Finalize the current sub-stage and set the current stage to its parent.
    */
   void upALevel();

   /**
    *  Advance to the next sub-stage.
    */
   void nextStage();

   /**
    *  Report progress within a %Stage.
    *
    *  @param text
    *         The text to send to the message log and %Progress object.
    *
    *  @param percent
    *         The new percentage complete for the current %Stage.
    *
    *  @param gran
    *         The ReportingLevel associated with this report.
    *
    *  @param log
    *         Should the message be sent to the message log as well as the %Progress object?
    */
   void report(const std::string &text, int percent, ReportingLevel gran, bool log = false);

   /**
    *  Get the current status as reported by the %Progess object.
    *
    *  @param text
    *         Output argument which will hold the current %Progress object text.
    *
    *  @param percent
    *         Output argument which will hold the current %Progress object percent complete.
    *
    *  @param level
    *         Output argument which will hold the current state of the %Progress object.
    *
    *  @return True if successfull, false if there is no %Progress object.
    */
   bool getStatus(std::string &text, int& percent, ReportingLevel &level)
   {
      if (mpProgress != NULL)
      {
         mpProgress->getProgress(text, percent, level);
      }
      else
      {
         text = "";
         percent = 0;
         level = NORMAL;
      }
      return mpProgress != NULL;
   }

   /**
    *  Get the Progress object associated with the current step.
    *
    *  @return Pointer to the current Progress or NULL if there is no current Progress.
    */
   Progress* getCurrentProgress() const
   {
      return mpProgress;
   }

   /**
    *  Abort the process tracked by this %ProgressTracker.
    */
   void abort();

   /**
    *  Accessor for the message log %Step associated with the current %Stage.
    *
    *  @return The %Step associated with the current %Stage.
    */
   Step *getCurrentStep();

private:
   Progress* mpProgress;
   Stage mMainStage;
};

/**
 *  Resource for subdividing the current %Stage.
 */
class ProgressSubdivision
{
public:
   /**
    *  Construct a %ProgressSubdivision in a specified %ProgressTracker.
    *
    *  @param pTracker
    *         The tracker to subdivide.
    *
    *  @param stages
    *         The vector of sub-stages.
    */
   ProgressSubdivision(ProgressTracker *pTracker, std::vector<ProgressTracker::Stage> &stages) :
               mpTracker(pTracker),
               mStageCount(static_cast<int>(stages.size()))
   {
      mpTracker->subdivideCurrentStage(stages);
   }

   /**
    *  Destructor.
    *  Automatically moves up to the parent %Stage of the subdivision managed by this resource.
    */
   ~ProgressSubdivision()
   {
      if (mStageCount > 0)
      {
         mpTracker->upALevel();
      }
   }

private:
   ProgressTracker* mpTracker;
   int mStageCount;
};

#endif
