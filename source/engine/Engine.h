#pragma once
#include <memory>
#include "util/Timer.h"
#include "rendering/Window.h"
#include "input/Input.h"

class Camera;
class Application;

class Engine : public InputHandler
{
public:
    Engine();

    void init(Application* app);

    void update();

    bool running() const { return m_running; }

    void shutdown();

    /**
    * Register camera for automatic resize on window resize.
    */
    void registerCamera(Camera* camera);
    
    void unregisterCamera(Camera* camera);

protected:
    void onQuit() override;
    void onWindowEvent(const SDL_WindowEvent& windowEvent) override;
    void onSDLEvent(SDL_Event& sdlEvent) override;
private:
    void resize(int width, int height);

private:
    bool m_running;
    bool m_paused{ false };
    bool m_initialized;

    Application* m_app{nullptr};
    std::vector<Camera*> m_cameras;
};
