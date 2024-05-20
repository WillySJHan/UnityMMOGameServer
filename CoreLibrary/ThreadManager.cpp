#include "pch.h"
#include "ThreadManager.h"
#include "GlobaVariable.h"
#include "TLSVariable.h"
#include "TaskQueueManager.h"

ThreadManager::ThreadManager()
{
	// MainThread
	InitTLSVar();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Run(std::function<void()> task)
{
	std::lock_guard<std::recursive_mutex> lockGuard(_rMutex);
	_threads.push_back(std::thread([=]()
		{
			// WorkerThread
			InitTLSVar();
			task();
			DestroyTLSVar();
		}));
}

void ThreadManager::Join()
{
	for (std::thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}

	_threads.clear();
}

void ThreadManager::InitTLSVar()
{
	static std::atomic<__int32> s_threadId = 1;
	t_threadId = s_threadId.fetch_add(1);
}

void ThreadManager::DestroyTLSVar()
{
}

void ThreadManager::ExecuteTask()
{
	while (true)
	{
		uint64_t now = ::GetTickCount64();
		if (now > t_endTime)
			break;

		std::shared_ptr<TaskQueue> taskQueue = g_taskQueueManager->Pop();
		if (taskQueue == nullptr)
			break;

		taskQueue->Execute();
	}
}

void ThreadManager::PushReservedTask()
{
	const uint64_t now = ::GetTickCount64();

	g_taskScheduler->PushTask(now);
}
