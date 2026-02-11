#pragma once
#include "gamelogic/Component.h"

namespace gamelogic
{
	class MoveComponent : public Component
	{
	public:
		MoveComponent(class Actor* owner, int updateOrder = 10);

	public:
		void Update(float deltaTime) override;

	public:
		float   GetAngularSpeed() const { return _angularSpeed; }
		float   GetForwardSpeed() const { return _forwardSpeed; }
		void    SetAngularSpeed(float speed) { _angularSpeed = speed; }
		void    SetForwardSpeed(float speed) { _forwardSpeed = speed; }

	private:
		float _angularSpeed{0.0f};
		float _forwardSpeed{0.0f};
	};
}