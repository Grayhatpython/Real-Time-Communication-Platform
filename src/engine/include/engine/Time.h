#pragma once

namespace engine
{
    class Time
    {
        static constexpr double S_TargetFrameTime = 1.0 / 60.0;

    public:
        Time();

    public:
        void Update();

    public:
        double GetTime() const;    
        float  GetDeltaTime() const { return _deltaTime; }
    
        int    GetFPS() const { return static_cast<int>(1.0f / _deltaTime); }

    private:
        std::chrono::high_resolution_clock::time_point _startTime;
        double _lastFrameTime = 0.0;
        float  _deltaTime = 0.0f;
        
    };
}