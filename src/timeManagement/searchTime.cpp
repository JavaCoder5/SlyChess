/*
	Copyright (C) 2026 JavaCoder5

	SlyChess is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SlyChess is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "searchTime.h"
#include <iostream>
#include <mutex>
#include <condition_variable>

std::thread timerThread;

static std::mutex timerMutex;
static std::condition_variable timerCv;
static bool timerCancelled = false;

// Runs on the timer thread: waits for the given time, then tells the search to stop.
// It only sets the stop flag; the search thread is joined by the UCI thread
// (stopAndJoinSearch), so the two threads never join or reassign it at the same time.
// Returns early without stopping anything if the timer is cancelled.
static void waitThenStopSearch(int waitTime)
{
	std::unique_lock<std::mutex> lock(timerMutex);
	if (!timerCv.wait_for(lock, std::chrono::milliseconds(waitTime), [] { return timerCancelled; }))
		stopSearch = true;
}

void cancelSearchTimer()
{
	{
		std::lock_guard<std::mutex> lock(timerMutex);
		timerCancelled = true;
	}
	timerCv.notify_all();

	if (timerThread.joinable())
		timerThread.join();

	std::lock_guard<std::mutex> lock(timerMutex);
	timerCancelled = false;
}

void searchTime(int totalTime)
{
	int waitTime = std::max(totalTime - 50, 25);

	timerThread = std::thread(waitThenStopSearch, waitTime);
}

void searchTimeControl(int wTime, int bTime, int wInc, int bInc)
{
	int waitTime = 0;

	if (turn)
	{
		// White to play (engine is playing as white)
		waitTime = wTime / 20 + wInc / 2;
	}
	else
	{
		// Black to play
		waitTime = bTime / 20 + bInc / 2;
	}

	int moveOverhead = std::stoi(getUciOptionValue("Move Overhead"));
	waitTime = std::max(waitTime - moveOverhead, 25); // Ensure a minimum wait time of 25ms to avoid a null move return.

	timerThread = std::thread(waitThenStopSearch, waitTime);
}