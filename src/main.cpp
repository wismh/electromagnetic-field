#include <engine/engine.h>

#include <game/game.h>

int main() {
    engine::Engine<game::Game> app;
    if (!app.init()) {
        return 1;
    }
    return app.run();
}
