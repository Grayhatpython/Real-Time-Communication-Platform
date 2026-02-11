#pragma once

namespace gamelogic
{
    class Actor;
    class Component
    {
    public:
        Component(Actor* owner, int32 updateOrder = 100);
        virtual ~Component();

    public:
        virtual void Update(float deltaTime);
        virtual void ProcessInput(const uint8* KeyState) {}

    public:
        int32   GetUpdateOrder() const { return _updateOrder; }

    protected:
        Actor*  _owner = nullptr;
        int32   _updateOrder{0};
    };
}