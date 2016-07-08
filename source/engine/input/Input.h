#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL.h>

class InputHandler
{
    friend class Input;
public:
    virtual ~InputHandler() {}

protected:
    virtual void onQuit() {}

    virtual void onMousewheel(float delta) {}

    virtual void onKeyDown(SDL_Keycode keyCode) {}

    virtual void onWindowEvent(const SDL_WindowEvent& windowEvent) {}

    virtual void onMouseDown(const SDL_MouseButtonEvent& e) {}
    virtual void onMouseUp(const SDL_MouseButtonEvent& e) {}
    virtual void onMouseMotion(const SDL_MouseMotionEvent& e) {}
    virtual void onSDLEvent(SDL_Event& e) {}
};

class DragState
{
    friend class Input;
public:
    DragState(uint32_t sdlButton);
    void update(int screenHeight, bool flipMouseY);

    bool isDragging() const { return m_dragging; }
    bool dragStarted() const { return m_dragStarted; }
    bool dragStopped() const { return m_dragStopped; }
    const glm::vec2& getLastDragPos() const { return m_lastDragPos; }
    const glm::vec2& getCurDragPos() const { return m_curDragPos; }
    const glm::vec2& getStartDragPos() const { return m_startDragPos; }

    /**
    * Returns the vector from current position to the position in the last frame
    */
    glm::vec2 getDragDelta() const { return m_lastDragPos - m_curDragPos; }

    /**
    * Returns the vector from current position to the drag start position
    */
    glm::vec2 getDragDeltaToStart() const { return m_startDragPos - m_curDragPos; }
private:
    bool getDragInput(int* x, int* y);

private:
    bool m_dragging;
    bool m_dragStarted; // Started dragging in current frame
    bool m_dragStopped; // Stopped dragging in current frame
    glm::vec2 m_lastDragPos;
    glm::vec2 m_curDragPos;
    glm::vec2 m_startDragPos;
    uint32_t m_sdlButton;
};

class Input
{
public:
    static void update(int screenHeight = 0.f, bool flipMouseY = false);

    static const DragState& rightDrag() { return m_rightDragState; }
    static const DragState& leftDrag() { return m_leftDragState; }
    static bool isDragging() { return m_leftDragState.isDragging() || m_rightDragState.isDragging(); }

    static void subscribe(InputHandler* inputHandler);

    static void unsubscribe(InputHandler* inputHandler);

    static void clearInputHandlers() { m_inputHandlers.clear(); }

    static bool isKeyDown(SDL_Scancode scancode);
public:
    static glm::vec3 mousePosition;
private:
    static DragState m_rightDragState;
    static DragState m_leftDragState;
    static std::vector<InputHandler*> m_inputHandlers;
};
