#include "pch.h"
#include "TaskScheduler.h"
#include "TaskQueue.h"

void TaskScheduler::Reserve(unsigned __int64 tickAfter, std::weak_ptr<TaskQueue> owner, std::shared_ptr<Task> task)
{
	const uint64_t executeTick = ::GetTickCount64() + tickAfter;
	/////////////////////////////////////////////////////////////////////////////////////
	TaskData* task_data = new TaskData(owner, task);

	std::lock_guard<std::recursive_mutex> lock(_rMutex);

	_datas.push(TimeData{ executeTick, task_data });
}

void TaskScheduler::PushTask(unsigned __int64 now)
{
	// 한 번에 1 쓰레드만 통과
	if (_distributing.exchange(true) == true)
		return;

	std::vector<TimeData> datas;

	{
		std::lock_guard<std::recursive_mutex> lock(_rMutex);

		while (_datas.empty() == false)
		{
			const TimeData& timerItem = _datas.top();
			if (now < timerItem.executeTick)
				break;

			datas.push_back(timerItem);
			_datas.pop();
		}
	}

	for (TimeData& data : datas)
	{
		if (std::shared_ptr<TaskQueue> owner = data.taskData->owner.lock())
			owner->Push(data.taskData->task);

		delete(data.taskData);
	}

	// 끝났으면 풀어준다
	_distributing.store(false);
}

void TaskScheduler::Clear()
{
	std::lock_guard<std::recursive_mutex> lock(_rMutex);

	while (_datas.empty() == false)
	{
		const TimeData& timerData = _datas.top();
		delete(timerData.taskData);
		_datas.pop();
	}
}