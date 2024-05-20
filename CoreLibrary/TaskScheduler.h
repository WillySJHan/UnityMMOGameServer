#pragma once

struct TaskData
{
	TaskData(std::weak_ptr<TaskQueue> owner, std::shared_ptr<Task> task) : owner(owner), task(task)
	{

	}

	std::weak_ptr<TaskQueue>			owner;
	std::shared_ptr<Task>				task;
};

struct TimeData
{
	bool operator<(const TimeData& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64_t executeTick = 0;
	TaskData* taskData = nullptr;
};

class TaskScheduler
{
public:
	void			Reserve(uint64_t tickAfter, std::weak_ptr<TaskQueue> owner, std::shared_ptr<Task> task);
	void			PushTask(uint64_t now);
	void			Clear();

private:
	std::recursive_mutex _rMutex;
	std::priority_queue<TimeData>	_datas;
	std::atomic<bool>				_distributing = false;
};