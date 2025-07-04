#include <gl/glew.h>

#define RGFW_IMPLEMENTATION
#include "RGFW.h"

int main(int argc, char** argv[])
{
    RGFW_window* win = RGFW_createWindow(
        "RGFW OpenGL Example",
        RGFW_RECT(0, 0, 640, 480),
        RGFW_windowCenter | RGFW_windowNoResize
    );

    if (!win) return -1;

    glewInit();

    while (!RGFW_window_shouldClose(win)) {
        while (RGFW_window_checkEvent(win)) {
            if (win->event.type == RGFW_quit)
                break;
        }

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glColor3f(1, 0, 0); glVertex2f(-0.5f, -0.5f);
        glColor3f(0, 1, 0); glVertex2f(0.5f, -0.5f);
        glColor3f(0, 0, 1); glVertex2f(0.0f, 0.5f);
        glEnd();

        RGFW_window_swapBuffers(win);
    }

    RGFW_window_close(win);
    return 0;
}
