#include "pch.h"
#include "TaskQueue.h"
#include "TaskQueueManager.h"

void TaskQueue::Push(std::shared_ptr<Task> task, bool pushOnly)
{
	// TODO ���� lock
	const int32_t prevCount = _taskCount.fetch_add(1);
	_taskQueue.Push(task); // lock

	// ù��° Task�� ���� �����尡 ������� ���
	if (prevCount == 0)
	{
		// �̹� �������� TaskQueue�� ������ ����
		if (t_myTaskQueue == nullptr)
		{
			Execute();
		}
		else
		{
			// �ٸ� �����尡 �����ϵ��� �ѱ��
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

		// ���� �ϰ��� 0����� ����
		if (_taskCount.fetch_sub(taskCount) == taskCount)
		{
			t_myTaskQueue = nullptr;
			return;
		}

		const uint64_t now = ::GetTickCount64();
		if (now >= t_endTime)
		{
			t_myTaskQueue = nullptr;
			// �ٸ� �����尡 �����ϵ��� �ѱ��
			g_taskQueueManager->Push(shared_from_this());
			break;
		}
	}
}
