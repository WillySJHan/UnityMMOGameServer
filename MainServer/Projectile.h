#pragma once
#include "BaseObject.h"
#include "Data.h"

class Projectile : public BaseObject
{
public:
	Projectile();
	virtual ~Projectile();

	virtual void Update();

public:
	std::shared_ptr<Skill> _data;
};

