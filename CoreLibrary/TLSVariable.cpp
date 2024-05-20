#include "pch.h"
#include "TLSVariable.h"

thread_local uint32_t t_threadId = 0;
thread_local uint64_t t_endTime = 0;
thread_local TaskQueue* t_myTaskQueue = nullptr;
