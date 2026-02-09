#include "engine/EnginePch.h"
#include "engine/Time.h"
#include <emmintrin.h>

namespace engine
{
    Time::Time()
    {
        _startTime = std::chrono::high_resolution_clock::now();
        _lastFrameTime = GetTime();
    }

    void Time::Update()
    {
        double targetTime = _lastFrameTime + S_TargetFrameTime;

        while (GetTime() < targetTime) 
        {
            ::_mm_pause();
        }

        double currentTime = GetTime();
        _deltaTime = static_cast<float>(currentTime - _lastFrameTime);
        _lastFrameTime = currentTime;
    }

    double Time::GetTime() const 
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> doubleSeconds = now - _startTime;
        return doubleSeconds.count();
    }
}