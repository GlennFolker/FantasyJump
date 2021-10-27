#include "renderer.h"

namespace Fantasy {
    Renderer::Renderer() {
        batch = new SpriteBatch();
    }

    Renderer::~Renderer() {
        batch->~SpriteBatch();
    }

    void Renderer::update() {
        batch->begin();
        batch->end();
    }
}
