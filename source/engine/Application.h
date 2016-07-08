#pragma once
#include <SDL.h>

class Engine;

class Application
{
    friend class Engine;
public:
	virtual ~Application() {}

    Application() {}

    virtual void handleSdlEvent(const SDL_Event&) {}

protected:
    virtual void update() = 0;
    virtual void initUpdate() = 0;

    bool isInitializing() const { return m_initializing; }

protected:
    bool m_initializing{ true };
    Engine* m_engine{nullptr};
};
