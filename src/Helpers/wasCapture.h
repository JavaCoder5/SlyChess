#pragma once
#include <src/Constants/Constants.h>

extern UndoInfo history[MAX_UNDO];
extern int historyTop;

inline bool wasCapture()
{
	if (history[historyTop].capturedPiece)
		return true;
	else
		return false;
}