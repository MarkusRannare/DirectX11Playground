//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
// Taken from files 3D Game Programming with DirectX 11
// @todo: Remove this and write a proper game timer class that doesn't require Tick to be
// called every frame, but for now, keep it to focus on rendering
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

#include "Types.h"

class GameTimer
{
public:
	GameTimer();

	double TotalTime() const;  // in seconds
	double DeltaTime() const; // in seconds

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	double mSecondsPerCount;
	double mDeltaTime;

	int64 mBaseTime;
	int64 mPausedTime;
	int64 mStopTime;
	int64 mPrevTime;
	int64 mCurrTime;

	bool mStopped;
};

#endif // GAMETIMER_H