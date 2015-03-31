/*
   The MIT License (MIT) (http://opensource.org/licenses/MIT)
   
   Copyright (c) 2015 Jacques Menuet
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "RLJJoystickEnumerationTrigger.h"

#include <assert.h>
#include <algorithm>
#include <sys/time.h>

namespace RLJ
{

/*
	TimeBasedEnumerationTrigger
*/
unsigned int TimeBasedEnumerationTrigger::mInitialSecond = 0xFFFFFFFF;

TimeBasedEnumerationTrigger::TimeBasedEnumerationTrigger( unsigned int intervalInMs )
	: mIntervalInMs(intervalInMs),
	  mStartTime(0),
	  mNextTime(0)
{
	mStartTime = getTimeAsMilliseconds();
	
	// At startup, we schedule the enumeration as soon as possible,
	// instead of waiting an initial interval like so:
	// updateNextTime();
	mNextTime = mStartTime;
}

bool TimeBasedEnumerationTrigger::enumerationNeeded()
{
	bool ret = false;
	unsigned int currentTime = getTimeAsMilliseconds();
	if ( currentTime>=mNextTime )
	{
		ret = true;
		updateNextTime();
	}
	return ret;
}

// Return the number of intervals done since start. 
// Returns -1 if interval was set to 0 (continuous firing)
int TimeBasedEnumerationTrigger::updateNextTime()
{
	int numIntervalsDone = -1;
	unsigned int currentTime = getTimeAsMilliseconds();
	if ( mIntervalInMs==0 )
	{
		mNextTime = currentTime;
	}
	else
	{
		unsigned int timeSinceStart = currentTime - mStartTime;
		numIntervalsDone = timeSinceStart / mIntervalInMs;
		assert( numIntervalsDone>=0 );
		mNextTime = mStartTime + (static_cast<unsigned int>(numIntervalsDone) + 1) * mIntervalInMs;
	}
	return numIntervalsDone;
}

unsigned int TimeBasedEnumerationTrigger::getTimeAsMilliseconds()
{
	struct timeval p;
	gettimeofday(&p, NULL);	// Gets the time since the Epoch (00:00:00 UTC, January 1, 1970) in sec, and microsec
	if ( mInitialSecond==0xFFFFFFFF )
		mInitialSecond = p.tv_sec;
	unsigned int tickCountInMs = (p.tv_sec - mInitialSecond) * 1000 + ( p.tv_usec / 1000 );
	return tickCountInMs;
}

}

