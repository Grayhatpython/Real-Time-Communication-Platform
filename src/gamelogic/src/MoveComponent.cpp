#include "gamelogic/GameLogicPch.h"
#include "gamelogic/MoveComponent.h"
#include "gamelogic/Actor.h"

namespace gamelogic
{
	MoveComponent::MoveComponent(Actor* owner, int updateOrder)
		: Component(owner, updateOrder)
	{
		
	}

	void MoveComponent::Update(float deltaTime)
	{
		if (Math::NearZero(_angularSpeed) == false)
		{
			float rotation = _owner->GetRotation();
			rotation += _angularSpeed * deltaTime;
			_owner->SetRotation(rotation);
		}
		
		if (Math::NearZero(_forwardSpeed) == false)
		{
			Vector2 position = _owner->GetPosition();
			position += _owner->GetForward() * _forwardSpeed * deltaTime;
			
			if (position.x < 0.0f) { position.x = 1022.0f; }
			else if (position.x > 1024.0f) { position.x = 2.0f; }

			if (position.y < 0.0f) { position.y = 766.0f; }
			else if (position.y > 768.0f) { position.y = 2.0f; }

			_owner->SetPosition(position);
		}
	}
}