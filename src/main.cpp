#include <emscripten/emscripten.h>
#include "application.hpp"

auto main() -> int {
    Application app;

    if (!app.Initialize()) {
        return 1;
    }

    app.Start();
}