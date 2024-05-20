#pragma once
#include "LockBasedQueue.h"

class TaskQueueManager
{
public:
	TaskQueueManager();
	~TaskQueueManager();

	void					Push(std::shared_ptr<class TaskQueue> taskQueue);
	std::shared_ptr<class TaskQueue>				Pop();

private:
	LockBasedQueue<std::shared_ptr<class TaskQueue>> _taskQueues;
};


