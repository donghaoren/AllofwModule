#ifndef ALLOFW_OPENGL_H
#define ALLOFW_OPENGL_H

#include "common.h"
#include "math/geometry.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// OpenGL Window and Context Creation.

namespace ALLOFW_NS {

    class OpenGLWindow;
    typedef std::shared_ptr<OpenGLWindow> POpenGLWindow;

    class OpenGLWindow {
    public:

        struct Hint {
            static const int FULLSCREEN = -1;

            bool stereo;
            int width;
            int height;

            Hint() {
                stereo = false;
                width = 900;
                height = 600;
            }

            void fullscreen() {
                width = FULLSCREEN;
                height = FULLSCREEN;
            }
        };

        class Delegate {
        public:
            virtual void onMove(int x, int y);
            virtual void onResize(int width, int height);
            virtual void onClose();
            virtual void onRefresh();
            virtual void onFocus(int focused);
            virtual void onIconify(int iconified);
            virtual void onFramebufferSize(int width, int height);
            virtual ~Delegate() { }
        };

        virtual void setDelegate(Delegate* delegate) = 0;
        virtual void makeContextCurrent() = 0;
        virtual void swapBuffers() = 0;
        virtual void pollEvents() = 0;
        virtual void waitEvents() = 0;
        virtual Size2i getFramebufferSize() = 0;

        virtual ~OpenGLWindow() { }

        // Static methods to create a window.
        static OpenGLWindow* Create_(Hint hint, const char* title);
        static POpenGLWindow Create(Hint hint, const std::string& title) {
            return POpenGLWindow(Create_(hint, title.c_str()));
        }
    };
}

#endif
