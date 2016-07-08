#include <engine/Engine.h>
#include "app/MeshDecimationApp.h"
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

std::unique_ptr<Engine> engine;

void onFrame();

int main(int, char**)
{
    engine = std::make_unique<Engine>();
    std::unique_ptr<MeshDecimationApp> app = std::make_unique<MeshDecimationApp>();

    engine->init(app.get());

#ifdef __EMSCRIPTEN__
    Logger::log("Using emscripten.");
    emscripten_set_main_loop(onFrame, 0, 1);
#else
    while(engine->running())
        onFrame();
#endif

    engine->shutdown();
    return 0;
}

void onFrame()
{
    engine->update();
}
