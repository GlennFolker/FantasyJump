#include <SDL.h>
#include <gl/glew.h>

#include "renderer.h"

namespace Fantasy {
    Renderer::Renderer() {
        batch = new SpriteBatch();
    }

    Renderer::~Renderer() {
        batch->~SpriteBatch();
    }

    void Renderer::update() {
        //glEnable(GL_DEPTH_TEST);
        //glDepthMask(GL_TRUE);
        //glDepthFunc(GL_LEQUAL);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        batch->begin();
        batch->end();
    }
}
