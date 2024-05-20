#pragma once
#include "Projectile.h"


class Bullet : public Projectile
{
public:
	virtual ~Bullet();
	void Update() override;
	virtual std::shared_ptr<BaseObject> GetOwner() override;

public:
	std::shared_ptr<BaseObject> _owner;

};

