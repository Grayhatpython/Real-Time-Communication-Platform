#include "gamelogic/GameLogicPch.h"
#include "gamelogic/Actor.h"
#include "gamelogic/Component.h"
#include "gamelogic/Component.h"
#include <algorithm>

namespace gamelogic
{
	Actor::Actor()
	{

	}

	Actor::~Actor()
	{		
		while (_components.empty() == false)
		{
			delete _components.back();
		}

		_components.clear();
	}

	void Actor::Update(float deltaTime)
	{
		if (_state == ActorState::Active)
		{
			UpdateComponents(deltaTime);
			UpdateActor(deltaTime);
		}
	}

	void Actor::UpdateComponents(float deltaTime)
	{
		for (auto component : _components)
		{
			component->Update(deltaTime);
		}
	}

	void Actor::UpdateActor(float deltaTime)
	{
	}

	void Actor::ProcessInput(const uint8* keyState)
	{
		if (_state == ActorState::Active)
		{
			for (auto component : _components)
			{
				component->ProcessInput(keyState);
			}

			ActorInput(keyState);
		}
	}

	void Actor::ActorInput(const uint8* keyState)
	{
	}

	void Actor::AddComponent(Component* component)
	{
		int updateOrder = component->GetUpdateOrder();
		auto iter = _components.begin();
		for (;iter != _components.end();++iter)
		{
			if (updateOrder < (*iter)->GetUpdateOrder())
			{
				break;
			}
		}

		_components.insert(iter, component);
	}

	void Actor::RemoveComponent(Component* component)
	{
		auto iter = std::find(_components.begin(), _components.end(), component);
		if (iter != _components.end())
		{
			_components.erase(iter);
		}
	}
}