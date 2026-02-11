#include "gamelogic/GameLogicPch.h"
#include "gamelogic/Component.h"
#include "gamelogic/Actor.h"

namespace gamelogic
{
    Component::Component(Actor* owner, int32 updateOrder)
        : _owner(owner), _updateOrder(updateOrder)
    {
        _owner->AddComponent(this);
    }

    Component::~Component()
    {
        _owner->RemoveComponent(this);
    }

    void Component::Update(float deltaTime)
    {
        
    }
}