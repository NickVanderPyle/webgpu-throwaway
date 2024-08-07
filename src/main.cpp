#include <emscripten/emscripten.h>
#include "application.hpp"

int main() {
    Application app;

    if (!app.Initialize()) {
        return 1;
    }

    app.Start();
}