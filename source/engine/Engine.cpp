#include "Engine.h"
#include "Application.h"
#include "util/Logger.h"
#include <assert.h>
#include <string>
#include "rendering/Window.h"
#include "input/Input.h"
#include "camera/Camera.h"
#include "rendering/Screen.h"
#include "imgui_impl/imgui_impl_sdl_gl3.h"
#include <imgui/imgui.h>

Engine::Engine()
    :m_running(true), m_initialized(false)
{
}

void Engine::init(Application* app)
{
    assert(app);
    m_app = app;
    m_app->m_engine = this;
    Screen::init(800, 600, true);
    Screen::setTitle("Mesh-Decimator");
    m_initialized = true;
    Input::subscribe(this);

    ImGui_ImplSdlGL3_Init(Screen::getSDLWindow());
}

void Engine::update()
{
    if(!m_initialized)
    {
        Logger::log("Engine is not initialized. Shutting down...");
        exit(1);
    }

    Time::update();
    Input::update(Screen::getHeight(), true);

    if (m_paused)
        return;

    if(m_app->isInitializing())
    {
        m_app->initUpdate();
    }
    else
    {
        ImGui_ImplSdlGL3_NewFrame(Screen::getSDLWindow());
        m_app->update();
        ImGui::Render();
    }

    Screen::update();
}

void Engine::shutdown()
{
    ImGui_ImplSdlGL3_Shutdown();
    SDL_Quit();
}

void Engine::registerCamera(Camera* camera)
{
    m_cameras.push_back(camera);
}

void Engine::unregisterCamera(Camera* camera)
{
    m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
}

void Engine::onQuit()
{
    m_running = false;
}

void Engine::onWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
        resize(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        m_paused = true;
        break;
    case SDL_WINDOWEVENT_RESTORED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        m_paused = true;
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        m_paused = false;
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        m_paused = true;
        break;
    }
}

void Engine::onSDLEvent(SDL_Event& sdlEvent)
{
    ImGui_ImplSdlGL3_ProcessEvent(&sdlEvent);
}

void Engine::resize(int width, int height)
{
    Screen::resize(width, height);

    for (Camera* camera : m_cameras)
    {
        camera->resize(float(width), float(height));
    }
}
