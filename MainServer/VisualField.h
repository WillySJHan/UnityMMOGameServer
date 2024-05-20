#pragma once

class Player;
class Task;
class BaseObject;

class VisualField : public std::enable_shared_from_this<VisualField>
{
public:
	VisualField(std::shared_ptr<Player> player);
	~VisualField();

	std::optional<std::unordered_set<std::shared_ptr<BaseObject>>> SurroundingObjects();

	void Update();

public:
	std::weak_ptr<Player> _owner;
	std::unordered_set<std::shared_ptr<BaseObject>> _previousObjects;
	std::shared_ptr<Task> _task;
};
