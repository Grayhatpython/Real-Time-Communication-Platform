#pragma once
#include "gamelogic/MoveComponent.h"

namespace gamelogic
{
	class InputComponent : public MoveComponent
	{
	public:
		InputComponent(class Actor* owner);

	public:
		void ProcessInput(const uint8* keyState) override;

	public:
		float   GetMaxForwardSpeed() const { return _maxForwardSpeed; }
		void    SetMaxForwardSpeed(float speed) { _maxForwardSpeed = speed; }

		float   GetMaxAngularSpeed() const { return _maxAngularSpeed; }
		void    SetMaxAngularSpeed(float speed) { _maxAngularSpeed = speed; }
		int32   GetForwardKey() const { return _forwardKey; }
		void    SetForwardKey(int32 key) { _forwardKey = key; }

		int32   GetBackKey() const { return _backKey; }
		void    SetBackKey(int32 key) { _backKey = key; }

		int32   GetClockwiseKey() const { return _clockwiseKey; }
		void    SetClockwiseKey(int32 key) { _clockwiseKey = key; }

		int32   GetCounterClockwiseKey() const { return _counterClockwiseKey; }
		void    SetCounterClockwiseKey(int32 key) { _counterClockwiseKey = key; }

	private:
		float _maxForwardSpeed{0.0f};
		float _maxAngularSpeed{0.0f};

		int32 _forwardKey{0};
		int32 _backKey{0};

		int32 _clockwiseKey{0};
		int32 _counterClockwiseKey{0};
	};
} // namespace gamelogic
