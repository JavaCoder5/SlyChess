#include "stopAndJoinSearch.h"

void stopAndJoinSearch()
{
	stopSearch = true;

	while (searchRunning)
	{
		if (!searchRunning)
			break;
	}

	if (searchThread.joinable())
		searchThread.join();

	searchThread = std::thread();

	stopSearch = false;
}