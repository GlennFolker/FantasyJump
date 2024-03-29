#include <stdexcept>
#include <GL/glew.h>

#include "frame_buffer.h"
#include "../app.h"

namespace Fantasy {
    FrameBuffer *FrameBuffer::last = nullptr;

    FrameBuffer::FrameBuffer(): FrameBuffer(2, 2) {}
    FrameBuffer::FrameBuffer(int width, int height) : FrameBuffer(width, height, true) {}
    FrameBuffer::FrameBuffer(int width, int height, bool color) : FrameBuffer(width, height, color, false) {}
    FrameBuffer::FrameBuffer(int width, int height, bool color, bool depth) : FrameBuffer(width, height, color, depth, false) {}
    FrameBuffer::FrameBuffer(int width, int height, bool color, bool depth, bool stencil) {
        if(!color && !depth && !stencil) throw std::runtime_error("Framebuffers must have at least either a color, depth, or stencil attachment.");

        this->width = width;
        this->height = height;
        hasColor = color;
        hasDepth = depth;
        hasStencil = stencil;
        before = nullptr;
        capturing = false;

        glGenFramebuffers(1, &data);
        begin();

        if(color) {
            texture = new Tex2D(width, height, nullptr);
            texture->load();
            texture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
            texture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->data, 0);
        } else {
            texture = nullptr;
        }

        if(depth || stencil) {
            glGenRenderbuffers(1, &render);
            glBindRenderbuffer(GL_RENDERBUFFER, render);

            glRenderbufferStorage(
                GL_RENDERBUFFER,
                (depth && stencil) ? GL_DEPTH24_STENCIL8 :
                depth ? GL_DEPTH_COMPONENT24 : GL_STENCIL_INDEX8,
                width, height
            );

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                (depth && stencil) ? GL_DEPTH_STENCIL_ATTACHMENT :
                depth ? GL_DEPTH_ATTACHMENT : GL_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER, render
            );
        } else {
            render = 0;
        }

        end();
    }

    FrameBuffer::~FrameBuffer() {
        if(texture != nullptr) delete texture;
        if(render != 0) glDeleteRenderbuffers(1, &render);
        glDeleteFramebuffers(1, &data);
    }

    void FrameBuffer::resize(int width, int height) {
        if(capturing || (this->width == width && this->height == height)) return;

        this->~FrameBuffer();
        before = nullptr;
        capturing = false;
        this->width = width;
        this->height = height;

        glGenFramebuffers(1, &data);
        begin();

        if(hasColor) {
            texture = new Tex2D(width, height, nullptr);
            texture->load();
            texture->setFilter(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
            texture->setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->data, 0);
        } else {
            texture = nullptr;
        }

        if(hasDepth || hasStencil) {
            glGenRenderbuffers(1, &render);
            glBindRenderbuffer(GL_RENDERBUFFER, render);

            glRenderbufferStorage(
                GL_RENDERBUFFER,
                (hasDepth && hasStencil) ? GL_DEPTH24_STENCIL8 :
                hasDepth ? GL_DEPTH_COMPONENT24 : GL_STENCIL_INDEX8,
                width, height
            );

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                (hasDepth && hasStencil) ? GL_DEPTH_STENCIL_ATTACHMENT :
                hasDepth ? GL_DEPTH_ATTACHMENT : GL_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER, render
            );
        } else {
            render = 0;
        }

        end();
    }

    void FrameBuffer::begin() {
        if(capturing) return;
        capturing = true;

        before = last;
        last = this;

        glBindFramebuffer(GL_FRAMEBUFFER, data);
        glViewport(0, 0, width, height);
    }

    void FrameBuffer::end() {
        if(!capturing) return;
        capturing = false;

        last = before;
        glBindFramebuffer(GL_FRAMEBUFFER, before == nullptr ? 0 : before->data);
        before = nullptr;
        glViewport(0, 0, App::instance->getWidth(), App::instance->getHeight());
    }
}
