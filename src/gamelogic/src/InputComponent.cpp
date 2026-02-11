#include "gamelogic/GameLogicPch.h"
#include "gamelogic/InputComponent.h"
#include "gamelogic/Actor.h"

namespace gamelogic
{
	InputComponent::InputComponent(Actor* owner)
		: MoveComponent(owner)
	{
		
	}

	void InputComponent::ProcessInput(const uint8* keyState)
	{
		float forwardSpeed = 0.0f;
		if (keyState[_forwardKey])
		{
			forwardSpeed += _maxForwardSpeed;
		}
		if (keyState[_backKey])
		{
			forwardSpeed -= _maxForwardSpeed;
		}
		SetForwardSpeed(forwardSpeed);

		float angularSpeed = 0.0f;
		if (keyState[_clockwiseKey])
		{
			angularSpeed += _maxAngularSpeed;
		}
		if (keyState[_counterClockwiseKey])
		{
			angularSpeed -= _maxAngularSpeed;
		}
		SetAngularSpeed(angularSpeed);
	}
}