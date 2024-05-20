#pragma once
#include "Task.h"
#include "LockBasedQueue.h"
#include "TaskScheduler.h"

class TaskQueue : public std::enable_shared_from_this<TaskQueue>
{
public:
	void PushTask(FCallback&& f_callback)
	{
		Push(std::make_shared<Task>(std::move(f_callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void PushTask(Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(std::make_shared<Task>(owner, memFunc, std::forward<Args>(args)...));
	}

	std::shared_ptr<Task> ReservePush(uint64_t time, FCallback&& callback)
	{
		std::shared_ptr<Task> task = std::make_shared<Task>(std::move(callback));
		g_taskScheduler->Reserve(time, shared_from_this(), task);

		return task;
	}

	template<typename T, typename Ret, typename... Args>
	std::shared_ptr<Task> ReservePush(uint64_t time, Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		std::shared_ptr<Task> task = std::make_shared<Task>(owner, memFunc, std::forward<Args>(args)...);
		g_taskScheduler->Reserve(time, shared_from_this(), task);
		return task;
	}

	void				ClearTasks() { _taskQueue.Clear(); }

public:
	// TODO push ¿À¸¥°ª
	void				Push(std::shared_ptr<Task> task, bool pushOnly = false);
	void				Execute();

protected:
	LockBasedQueue<std::shared_ptr<Task>>	_taskQueue;
	std::atomic<int32_t>		_taskCount = 0;
};

