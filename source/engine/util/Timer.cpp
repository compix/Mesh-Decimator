#include "Timer.h"
#include <SDL.h>

Timer Time::m_timer;

Timer::Timer()
    :m_prevTimeMS(SDL_GetTicks())
{
}

void Timer::start()
{
    m_prevTimeMS = SDL_GetTicks();
    m_deltaTimeMS = 0;
    m_totalTimeMS = 0;
}

Seconds Timer::totalTime() const
{
    return m_totalTimeMS / Seconds(1000);
}

Seconds Timer::tick()
{
    Uint32 curTimeMS = SDL_GetTicks();
    m_deltaTimeMS = curTimeMS - m_prevTimeMS;
    m_totalTimeMS += m_deltaTimeMS;

    m_prevTimeMS = curTimeMS;
    return m_deltaTimeMS / Seconds(1000);
}

Seconds Timer::deltaTime() const
{
    return m_deltaTimeMS / Seconds(1000);
}

void Time::update()
{
    m_timer.tick();
}
