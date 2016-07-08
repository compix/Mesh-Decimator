#include "Input.h"
#include <SDL.h>
#include <algorithm>
#include <assert.h>

DragState Input::m_rightDragState(SDL_BUTTON_RIGHT);
DragState Input::m_leftDragState(SDL_BUTTON_LEFT);
std::vector<InputHandler*> Input::m_inputHandlers;
glm::vec3 Input::mousePosition;

DragState::DragState(uint32_t sdlButton)
    :m_dragging(false), m_dragStarted(false), m_dragStopped(false), m_sdlButton(sdlButton)
{
    
}

void DragState::update(int screenHeight, bool flipMouseY)
{
    m_dragStarted = m_dragStopped = false;

    m_lastDragPos = m_curDragPos;

    int x, y;
    bool dragging = getDragInput(&x, &y);
    y = flipMouseY ? (screenHeight - y) : y;

    if (!m_dragging && dragging)
    {
        m_dragStarted = true;
        m_curDragPos = m_lastDragPos = glm::vec2(float(x), float(y));
        m_startDragPos = m_curDragPos;
    }

    if (m_dragging && !dragging)
    {
        m_dragStopped = true;
    }

    m_dragging = dragging;

    if (m_dragging)
        m_curDragPos = glm::vec2(float(x), float(y));
}

bool DragState::getDragInput(int* x, int* y)
{
    return (SDL_GetMouseState(x, y) & SDL_BUTTON(m_sdlButton)) != 0;
}

void Input::update(int screenHeight, bool flipMouseY)
{
    assert(flipMouseY ? screenHeight > 0 : true);

    // Need to poll events first to provide correct mouse information (position, drag info)
    // Otherwise SDL_GetMouseState yields previous information
    SDL_Event sdlEvent;
    std::vector<SDL_Event> sdlEvents;
    while (SDL_PollEvent(&sdlEvent))
        sdlEvents.push_back(sdlEvent);

    int x, y;
    SDL_GetMouseState(&x, &y);
    mousePosition = glm::vec3(float(x), flipMouseY ? (screenHeight - y) : float(y), 1.f);
    m_rightDragState.update(screenHeight, flipMouseY);
    m_leftDragState.update(screenHeight, flipMouseY);

    for (auto& e : sdlEvents)
    {
        for (auto h : m_inputHandlers) h->onSDLEvent(e);

        switch (e.type)
        {
        case SDL_QUIT:
            for (auto h : m_inputHandlers) h->onQuit();
            break;
        case SDL_MOUSEWHEEL:
            for (auto h : m_inputHandlers) h->onMousewheel(float(e.wheel.y));
            break;
        case SDL_KEYDOWN:
            for (auto h : m_inputHandlers) h->onKeyDown(e.key.keysym.sym);
            break;
        case SDL_WINDOWEVENT:
            for (auto h : m_inputHandlers) h->onWindowEvent(e.window);
            break;
        case SDL_MOUSEBUTTONDOWN:
            for (auto h : m_inputHandlers) h->onMouseDown(e.button);
            break;
        case SDL_MOUSEBUTTONUP:
            for (auto h : m_inputHandlers) h->onMouseUp(e.button);
            break;
        case SDL_MOUSEMOTION:
            for (auto h : m_inputHandlers) h->onMouseMotion(e.motion);
            break;
        }
    }
}

void Input::subscribe(InputHandler* inputHandler)
{
    m_inputHandlers.push_back(inputHandler);
}

void Input::unsubscribe(InputHandler* inputHandler)
{
    m_inputHandlers.erase(std::remove(m_inputHandlers.begin(), m_inputHandlers.end(), inputHandler),
        m_inputHandlers.end());
}

bool Input::isKeyDown(SDL_Scancode scancode)
{
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    return keyState[scancode] != 0;
}
