#include "searchTime.h"

void searchTime(int totalTime)
{
	int waitTime = totalTime;

	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

	stopAndJoinSearch();
	return;
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

	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));

	stopAndJoinSearch();
	return;
}