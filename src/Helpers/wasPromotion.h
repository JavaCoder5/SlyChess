#pragma once
#include <src/Constants/Constants.h>

extern UndoInfo history[MAX_UNDO];
extern int historyTop;

inline bool wasPromotion()
{
	if (history[historyTop].wasPromotion)
		return true;
	else
		return false;
}