/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "MultiThreadedAlgorithm.h"
#include "MessageLogMgrImp.h"
#include "Progress.h"
#include "Units.h"

using namespace mta;

//------------ MultiThreadReporter ---------------//

/*
   The interplay between the thread reporter (via signalMainThread) and the
   main thread's waitForThreadsToComplete method is as follows:

   Main Thread                         Thread Reporter::signalMainThread
   -----------                         ---------------
   Lock mutex a
   Start Threads
                     ** Time 1 **
   Wait on signal a (unlocks mutex a)  (Thread Starts)
                     ** Time 2 **
                                       Lock mutex a
                                       Unlock mutex a
                                       Set info for main thread
                                       Lock mutex b
                                       Send signal a
                     ** Time 3 **
   Wake up (locks mutex a)
                                       Wait on signal b (unlocks mutex b)
                     ** Time 4 **
   Lock mutex b
   Get info from thread reporter
   Do work
   Send signal b
                     ** Time 5 **
   Unlock mutex b                      Wake up (locks mutex b)
                                       Unlock mutex b
   Loop
*/

struct ProgressFunctor : public ThreadCommand
{
   ProgressFunctor(int *pProgressValue, int percent) : 
      mpProgressValue(pProgressValue), mPercent(percent) {}
   void run()
   {
      *mpProgressValue = mPercent;
   }
private:
   int* mpProgressValue;
   int mPercent;
};
struct CompletionFunctor : public ProgressFunctor
{
   CompletionFunctor(int *pProgressValue) : ProgressFunctor(pProgressValue, 100) {}
};
struct ErrorFunctor : public ThreadCommand
{
   ErrorFunctor(std::string errorText, std::string& errorMessage, Result *pResult) : 
      mErrorText(errorText), mMessage(errorMessage), mpResult(pResult) {}
   void run()
   {
      mMessage = mErrorText;
      *mpResult = FAILURE;
   }
private:
   std::string mErrorText;
   std::string& mMessage;
   Result* mpResult;
};
struct WorkFunctor : public ThreadCommand
{
   WorkFunctor(ThreadCommand*&pCommand, ThreadCommand& command) : 
      mpCommand(pCommand), mCommand(command) {}
   void run()
   {
      mpCommand = &mCommand;
   }
private:
   ThreadCommand*& mpCommand;
   ThreadCommand& mCommand;
};

MultiThreadReporter::MultiThreadReporter(int threadCount, Result* pResult, 
        BMutex &mutexA, BThreadSignal& signalA, BMutex &mutexB, BThreadSignal& signalB) : 
   mMutexA(mutexA), mSignalA(signalA), mMutexB(mutexB), mSignalB(signalB), 
   mpResult(pResult), mThreadProgress(threadCount), 
   mReportType(THREAD_NO_REPORT), mpThreadCommand(NULL)
{
   if (threadCount == 0 && pResult != NULL)
   {
      *mpResult = FAILURE;
      mErrorMessage = "Error: Thread count = 0";
   }
}

MultiThreadReporter::MultiThreadReporter(MultiThreadReporter &reporter) :
   mMutexA(reporter.mMutexA), mSignalA(reporter.mSignalA), mMutexB(reporter.mMutexB), mSignalB(reporter.mSignalB), 
   mpResult(reporter.mpResult), mThreadProgress(reporter.mThreadProgress), mErrorMessage(reporter.mErrorMessage),
   mReportType(reporter.mReportType), mpThreadCommand(NULL)
{
   Result result = *mpResult;
}

Result MultiThreadReporter::reportProgress(int threadIndex, int percentDone)
{
   Result result = SUCCESS;
   if (percentDone != mThreadProgress[threadIndex])
   {
      result = signalMainThread(ProgressFunctor(&mThreadProgress[threadIndex], percentDone), THREAD_PROGRESS);
   }
   return result;
}

Result MultiThreadReporter::reportError(std::string errorText)
{
   return signalMainThread(ErrorFunctor(errorText, mErrorMessage, mpResult), THREAD_ERROR);
}

Result MultiThreadReporter::reportCompletion(int threadIndex)
{
   return signalMainThread(CompletionFunctor(&mThreadProgress[threadIndex]), THREAD_COMPLETE);
}

void MultiThreadReporter::runInMainThread(ThreadCommand &command)
{
   Result result = signalMainThread(WorkFunctor(mpThreadCommand, command), THREAD_WORK);
   mpThreadCommand = NULL;
}

void MultiThreadReporter::setReportType(ReportType type)
{
   mReportType = type;
}

unsigned int MultiThreadReporter::getReportType() const
{
   return mReportType;
}

ThreadCommand* MultiThreadReporter::getThreadCommand()
{
   return mpThreadCommand;
}

int MultiThreadReporter::getProgress() const 
{ 
   MutexLock lock(mReporterMutex);
   int total = std::accumulate(mThreadProgress.begin(), mThreadProgress.end(), 0) / mThreadProgress.size(); 
   return total;
}

int MultiThreadReporter::getProgress(int threadIndex) const 
{ 
   MutexLock lock(mReporterMutex);
   return mThreadProgress[threadIndex];
}

std::string MultiThreadReporter::getErrorText() const 
{ 
   MutexLock lock(mReporterMutex);
   return mErrorMessage; 
}

Result MultiThreadReporter::signalMainThread(ThreadCommand& reportStatus, ReportType type)
{
   MutexLock lock(mSignalMutex);

   Result currentResult = SUCCESS;
   if (mpResult != NULL)
   {
      currentResult = *mpResult;
   }

   if (currentResult == SUCCESS)
   {
      {
         MutexLock reporterLock(mReporterMutex);
         reportStatus.run();
      }
      mReportType |= type;
      if (mpResult != NULL)
      {
         currentResult = *mpResult;
      }

      { // scope the lock
         // ensures that the main thread is in its ThreadSignalWait
         mMutexA.MutexLock();
         mMutexA.MutexUnlock();

         MutexLock lockB(mMutexB);
         mSignalA.ThreadSignalActivate(); // wake up main thread
         if (type != THREAD_COMPLETE)
         {
            mSignalB.ThreadSignalWait(&mMutexB); // wait for main thread to say its done
         }
      }
   }

   return currentResult;
}

//------------ AlgorithmThread ---------------//

void AlgorithmThread::threadFunction(AlgorithmThread *pThreadData)
{
   pThreadData->waitForAlgorithmLoop();
   pThreadData->run();
   if (pThreadData->getReporter().getErrorText() == "")
   {
      if (pThreadData->getReporter().getProgress(pThreadData->getThreadIndex()) != 100)
      {
         pThreadData->getReporter().reportProgress(pThreadData->getThreadIndex(), 100);
      }
   }
   if (pThreadData->getReporter().getErrorText() == "")
   {
      if (pThreadData->getReporter().getProgress(pThreadData->getThreadIndex()) != 100)
      {
         pThreadData->getReporter().reportCompletion(pThreadData->getThreadIndex());
      }
   }
}

bool AlgorithmThread::launch()
{
   mThreadHandle.ThreadLaunch();
   return true;
}

bool AlgorithmThread::wait()
{
   mThreadHandle.ThreadWait();
   return true;
}

AlgorithmThread::Range AlgorithmThread::getThreadRange(int threadCount, int dataSize) const
{
   AlgorithmThread::Range range;
   if (dataSize < threadCount && mThreadIndex >= dataSize)
   {
      range.mFirst = 0;
      range.mLast = -1;
   }
   else
   {
      int countPerThread = static_cast<int>(ceil(static_cast<double>(dataSize) / static_cast<double>(threadCount)));
      range.mFirst = mThreadIndex * countPerThread;
      range.mLast = range.mFirst + countPerThread - 1;
      if (range.mLast >= dataSize)
      {
         if (range.mFirst >= dataSize)
         {
            range.mFirst = 0;
            range.mLast = -1;
         }
         else
         {
            range.mLast = dataSize-1;
         }
      }
   }
   return range;
}

int AlgorithmThread::getThreadIndex() const
{
   return mThreadIndex;
}

ThreadReporter& AlgorithmThread::getReporter() const
{
   return mReporter;
}

void AlgorithmThread::runInMainThread(ThreadCommand &command)
{
   getReporter().runInMainThread(command);
}

void AlgorithmThread::setAlgorithmMutex(DMutex *pMutex)
{
   mpAlgorithmMutex = pMutex;
}

void AlgorithmThread::waitForAlgorithmLoop()
{
   if (mpAlgorithmMutex != NULL)
   {
      MutexLock lock(*mpAlgorithmMutex);
   }
}

//------------ ProgressObjectReporter ---------------//

void ProgressObjectReporter::reportError(const std::string &text)
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress(text, 0, ERRORS);
   }
}

void ProgressObjectReporter::reportProgress(int percent)
{
   if (mpProgress)
   {
      mpProgress->updateProgress(mMessage, percent, NORMAL);
   }
}

//------------ MultiPhaseProgressReporter ---------------//

void MultiPhaseProgressReporter::reportProgress(int percent)
{
   mReporter.reportProgress(convertPhaseProgressToTotalProgress(percent));
}

void MultiPhaseProgressReporter::reportError(const std::string &text)
{
   mReporter.reportError(text);
}

void MultiPhaseProgressReporter::setCurrentPhase(int phase)
{
   int iPhases = 0;
   iPhases = mPhaseWeights.size();

   mCurrentPhase = phase >= iPhases ? iPhases - 1 : phase;
   mCurrentPhase = mCurrentPhase <= 0 ? 0 : mCurrentPhase;
}

int MultiPhaseProgressReporter::getCurrentPhase() const
{
   return mCurrentPhase;
}

int MultiPhaseProgressReporter::convertPhaseProgressToTotalProgress(int phaseProgress)
{
   int totalProgress = 0;
   for (int i = 0; i < mCurrentPhase; ++i)
   {
      totalProgress += mPhaseWeights[i];
   }

   totalProgress += (phaseProgress * mPhaseWeights[mCurrentPhase]) / 100;
   return totalProgress;
}

Cube::Cube(void* pData, EncodingType type, int rowCount, int columnCount, int bandCount) :
   mpData(pData),
   mType(type),
   mRowCount(rowCount),
   mColumnCount(columnCount),
   mBandCount(bandCount),
   mPixelSize(computePixelSize(bandCount,type)),
   mScaleFromStandard(1.0)
{
}

Cube::Cube(const Cube& cube) :
   mpData(cube.mpData),
   mType(cube.mType),
   mRowCount(cube.mRowCount),
   mColumnCount(cube.mColumnCount),
   mBandCount(cube.mBandCount),
   mPixelSize(cube.mPixelSize),
   mScaleFromStandard(cube.mScaleFromStandard)
{
}

const void* Cube::getData() const
{
   return mpData;
}

EncodingType Cube::getType() const
{
   return mType;
}

int Cube::getRowCount() const
{
   return mRowCount;
}

int Cube::getColumnCount() const
{
   return mColumnCount;
}

int Cube::getBandCount() const
{
   return mBandCount;
}

void* Cube::getPixel(int row, int col) const
{
   return ((unsigned char*)mpData) + (mColumnCount * row + col) * mPixelSize;
}

double Cube::getScaleFromStandard() const
{
   return mScaleFromStandard;
}

int Cube::computePixelSize(int bandCount, EncodingType type)
{
   int elementSize = 0;

   switch (type)
   {
   case INT1UBYTE:
   case INT1SBYTE:
      elementSize = 1;
      break;

   case INT2UBYTES:
   case INT2SBYTES:
      elementSize = 2;
      break;

   case INT4SCOMPLEX:
   case INT4UBYTES:
   case INT4SBYTES:
   case FLT4BYTES:
      elementSize = 4;
      break;

   case FLT8COMPLEX:
   case FLT8BYTES:
      elementSize = 8;
      break;
   }

   return elementSize * bandCount;
}
