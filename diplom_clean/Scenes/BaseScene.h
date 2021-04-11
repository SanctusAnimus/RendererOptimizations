#pragma once
#ifndef BASESCENE_CLASS_DECL
#define BASESCENE_CLASS_DECL

#include <entt/entt.hpp>

class BaseScene
{
public:
	virtual void Setup() = 0;
	virtual void Render() = 0;

	virtual entt::entity AddEntity() { return registry.create(); };
	virtual void RemoveEntity(entt::entity& ent) {};

	bool active = false;
protected:
	entt::registry registry;
};

#endif // !BASESCENE_CLASS_DECL



