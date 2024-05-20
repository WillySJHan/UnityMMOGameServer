#include "pch.h"
#include "TaskQueue.h"
#include "TaskQueueManager.h"

void TaskQueue::Push(std::shared_ptr<Task> task, bool pushOnly)
{
	// TODO 순서 lock
	const int32_t prevCount = _taskCount.fetch_add(1);
	_taskQueue.Push(task); // lock

	// 첫번째 Task을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		// 이미 실행중인 TaskQueue가 없으면 실행
		if (t_myTaskQueue == nullptr)
		{
			Execute();
		}
		else
		{
			// 다른 쓰레드가 실행하도록 넘긴다
			auto b = shared_from_this();
			g_taskQueueManager->Push(b);
		}
	}
}


void TaskQueue::Execute()
{
	t_myTaskQueue = this;

	while (true)
	{
		std::vector<std::shared_ptr<Task>> tasks;
		_taskQueue.PopAll(tasks);

		const int32_t taskCount = static_cast<int32_t>(tasks.size());
		for (int32_t i = 0; i < taskCount; i++)
			tasks[i]->Execute();

		// 남은 일감이 0개라면 종료
		if (_taskCount.fetch_sub(taskCount) == taskCount)
		{
			t_myTaskQueue = nullptr;
			return;
		}

		const uint64_t now = ::GetTickCount64();
		if (now >= t_endTime)
		{
			t_myTaskQueue = nullptr;
			// 다른 쓰레드가 실행하도록 넘긴다
			g_taskQueueManager->Push(shared_from_this());
			break;
		}
	}
}
