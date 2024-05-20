#pragma once
#include <functional>

using FCallback = std::function<void(void)>;

class Task
{
public:
	Task(FCallback&& f_callback) : _f_callback(std::move(f_callback))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Task(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		_f_callback = [owner, memFunc, args...]()
			{
			// TODO
				(owner.get()->*memFunc)(args...);
			};
	}

	void Execute()
	{
		if (_cancel == false)
			_f_callback();
	}

private:
	FCallback _f_callback;

public:
	bool _cancel = false;
};


