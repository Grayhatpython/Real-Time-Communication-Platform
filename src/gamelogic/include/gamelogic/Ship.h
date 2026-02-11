#pragma once
#include "gamelogic/Actor.h"

namespace gamelogic
{
	class Ship : public Actor
	{
	public:
		Ship();

		void UpdateActor(float deltaTime) override;
		void ActorInput(const uint8* keyState) override;

	private:
	};
}