#pragma once

#include <vector>
#include "gamelogic/Math.h"

namespace gamelogic
{
	enum class ActorState : uint16
	{
		Active,
		Paused,
		Dead
	};
	class Component;
	class Actor
	{
	public:
		Actor();
		virtual ~Actor();

	public:
		void            Update(float deltaTime);
		void            UpdateComponents(float deltaTime);
		virtual void    UpdateActor(float deltaTime);

		void            ProcessInput(const uint8* keyState);
		virtual void    ActorInput(const uint8* keyState);

	public:
		const Vector2&  GetPosition() const { return _position; }
		void            SetPosition(const Vector2& position) { _position = position; }
		float           GetScale() const { return _scale; }
		void            SetScale(float scale) { _scale = scale; }
		float           GetRotation() const { return _rotation; }
		void            SetRotation(float rotation) { _rotation = rotation; }
		ActorNetworkId 	GetActorNetworkId() const { return _actorNetworkId; }
    	void       		SetActorNetworkId(ActorNetworkId id) { _actorNetworkId = id; }

		ActorState      GetState() const { return _state; }
		void            SetState(ActorState state) { _state = state; }

		Vector2         GetForward() const { return Vector2(Math::Cos(_rotation), -Math::Sin(_rotation)); }

	public:
		void            AddComponent(Component* component);
		void            RemoveComponent(Component* component);

	private:
		std::vector<Component*> _components;
		Vector2     			_position{};

		float       			_scale{1.f};
		float       			_rotation{0.f};
		ActorNetworkId			_actorNetworkId = 0;

		ActorState  			_state = ActorState::Active;
	};
}