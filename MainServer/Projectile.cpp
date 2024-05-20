#include "pch.h"
#include "Projectile.h"

Projectile::Projectile() : BaseObject(Protocol::ObjectType::PROJECTILE), _data(std::make_shared<Skill>())
{
}

Projectile::~Projectile()
{
	std::cout << "~Projectile" << std::endl;
}

void Projectile::Update()
{
}
