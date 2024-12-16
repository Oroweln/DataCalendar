#include "framework.h"

void * SafeCalloc(size_t size, int bytes)
{
	assert(bytes > 0);
	void* pointer = { 0 };
	if (pointer != NULL)
		CrashDumpFunction(66, 1);
	pointer = calloc(size, bytes);
	if (pointer == NULL)
		CrashDumpFunction(67, 1);
	return pointer;
}