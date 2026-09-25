#include "isRepetition.h"

bool isRepetition(U64 key)
{
	int index = key & REPETITION_TABLE_MASK;

	if (repetition_table[index].key == key) {
		if (repetition_table[index].isOpen)
			return true;
	}

	return false;
}