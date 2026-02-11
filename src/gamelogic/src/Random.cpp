#include "gamelogic/GameLogicPch.h"
#include "gamelogic/Random.h"

std::mt19937 Random::S_Generator;

void Random::Initialize()
{
	std::random_device rd;
	Random::Seed(rd());
}

void Random::Seed(unsigned int seed)
{
	S_Generator.seed(seed);
}

float Random::GetFloat()
{
	return GetFloatRange(0.0f, 1.0f);
}

float Random::GetFloatRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(S_Generator);
}

int Random::GetIntRange(int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(S_Generator);
}

Vector2 Random::GetVector(const Vector2& min, const Vector2& max)
{
	Vector2 r = Vector2(GetFloat(), GetFloat());
	return min + (max - min) * r;
}

Vector3 Random::GetVector(const Vector3& min, const Vector3& max)
{
	Vector3 r = Vector3(GetFloat(), GetFloat(), GetFloat());
	return min + (max - min) * r;
}
