/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HIGH_RESOLUTION_TIMER_H
#define HIGH_RESOLUTION_TIMER_H 1

#include "AppConfig.h"

#if defined(WIN_API)
#include "Windows.h"
#else
#include <sys/time.h>
#endif

#include <ostream>

namespace HrTimer
{

#if defined(WIN_API)
#define HrTimingType LONGLONG
#else
#define HrTimingType hrtime_t
#endif

inline HrTimingType getTime()
{
#if defined(WIN_API)
LARGE_INTEGER currentTime;
QueryPerformanceCounter(&currentTime);
return currentTime.QuadPart;
#else
return gethrtime();
#endif
}

inline double convertToSeconds(HrTimingType val)
{
#if defined(WIN_API)
LARGE_INTEGER frequency;
QueryPerformanceFrequency(&frequency);
LONGLONG longFrequency = frequency.QuadPart;
return val / ((double)longFrequency); 
#else
return val / 1000000000.0; //val on Solaris is in nano-seconds
#endif
}

class Resource
{
public:

   Resource()
	{
		mOutputType = 0; //use ostream
		init();
	}

   Resource(double* outputInto, bool milliSecond = true)
	{
		mOutputType = 2; //use double
		mpDoubleOutput = outputInto;
		mMillisecondResolution = milliSecond;
		init();
	}

   Resource(HrTimingType* start, HrTimingType* end)
   {
      mOutputType = 3;
      mpStartOutput = start;
      mpEndOutput = end;
      init();
   }

	~Resource()
	{
		HrTimingType end = HrTimer::getTime();
      if (mOutputType == 2)
		{
			//output value into a double
			double timeDiff = convertToSeconds(end - mStart);
			if (mMillisecondResolution)
			{
				timeDiff *= 1000.0;
			}
			*mpDoubleOutput = timeDiff;
		}
      if (mOutputType == 3)
      {
         *mpStartOutput = mStart;
         *mpEndOutput = end;
      }
	}

private:
	void init()
	{
		mStart = HrTimer::getTime();
	}

	HrTimingType mStart;
	int mOutputType;
	double* mpDoubleOutput;
   HrTimingType* mpStartOutput;
   HrTimingType* mpEndOutput;
	bool mMillisecondResolution;
};

};

#endif //HIGH_RESOLUTION_TIMER_H
