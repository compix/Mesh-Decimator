#pragma once
#include <SDL_stdinc.h>

using Seconds = float;

class Timer
{
public:
    Timer();

    void start();
    Seconds totalTime() const;

    /**
     * @return deltaTime
     */
    Seconds tick();
    Seconds deltaTime() const;

private:
    Uint32 m_deltaTimeMS{ 0 };
    Uint32 m_prevTimeMS{ 0 };
    Uint32 m_totalTimeMS{ 0 };
};

class Time
{
public:
    static void update();

    static Seconds deltaTime() { return m_timer.deltaTime(); }
    static Seconds totalTime() { return m_timer.totalTime(); }
private:
    static Timer m_timer;
};
