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

    while (!RGFW_window_shouldClose(win)) {
        while (RGFW_window_checkEvent(win)) {
            if (win->event.type == RGFW_quit)
                break;
        }

        RGFW_window_swapBuffers(win);
    }

    RGFW_window_close(win);
    return 0;
}
