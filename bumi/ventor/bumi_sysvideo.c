#include "bumi_sysvideo.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glx.h>

typedef struct {
    Display* dpy;
    int screen;
    Window root;
    int ref_count;
    Atom wm_delete;
    BUMI_Window* windows;
} BUMI_X11Context;

static BUMI_X11Context* ctx = NULL;
static char bumi_error[256] = "";

static void set_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(bumi_error, sizeof(bumi_error), fmt, args);
    va_end(args);
}

const char* BUMI_GetError(void) {
    return bumi_error;
}

void BUMI_ClearError(void) {
    bumi_error[0] = '\0';
}

static BUMI_Window* find_window(Window x11_window) {
    BUMI_Window* window = ctx ? ctx->windows : NULL;
    while (window) {
        if ((Window)(uintptr_t)window->backend_data == x11_window) {
            return window;
        }
        window = window->next;
    }
    return NULL;
}

static int bumi_init_ctx() {
    if (ctx) {
        ctx->ref_count++;
        return 1;
    }

    ctx = (BUMI_X11Context*) malloc(sizeof(BUMI_X11Context));
    if (!ctx) {
        set_error("Failed to allocate X11 context");
        return 0;
    }

    ctx->dpy = XOpenDisplay(NULL);
    if (!ctx->dpy) {
        free(ctx);
        ctx = NULL;
        set_error("Failed to open X11 display");
        return 0;
    }

    ctx->screen = DefaultScreen(ctx->dpy);
    ctx->root = RootWindow(ctx->dpy, ctx->screen);
    ctx->ref_count = 1;
    ctx->wm_delete = XInternAtom(ctx->dpy, "WM_DELETE_WINDOW", False);
    ctx->windows = NULL;
    return 1;
}

static void bumi_deinit_ctx() {
    if (!ctx || --ctx->ref_count > 0) return;

    if (ctx->dpy) {
        XCloseDisplay(ctx->dpy);
    }
    free(ctx);
    ctx = NULL;
}

int BUMI_Init(uint32_t flags) {
    BUMI_ClearError();

    if (flags & BUMI_INIT_VIDEO) {
        if (!bumi_init_ctx()) {
            return -1;
        }
    } else {
        set_error("No valid subsystems specified");
        return -1;
    }

    return 0;
}

void BUMI_Quit(void) {
    BUMI_ClearError();
    bumi_deinit_ctx();
}

BUMI_Window* BUMI_WindowCreate(const char* title, int x, int y, int w, int h, uint32_t flags) {
    BUMI_ClearError();

    if (!ctx && !bumi_init_ctx()) {
        return NULL;
    }

    BUMI_Window* window = (BUMI_Window*) malloc(sizeof(BUMI_Window));
    if (!window) {
        set_error("Failed to allocate window");
        return NULL;
    }

    memset(window, 0, sizeof(BUMI_Window));
    window->title = strdup(title ? title : "Bumi Window");
    if (!window->title) {
        free(window);
        set_error("Failed to allocate window title");
        return NULL;
    }
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
    window->flags = flags;
    window->display_scale = 1.0f;
    window->id = (BUMI_WindowID)(uintptr_t)window;
    window->last_pixel_w = w;
    window->last_pixel_h = h;
    window->min_w = window->min_h = 0;
    window->max_w = window->max_h = 0;
    window->min_aspect = window->max_aspect = 0.0f;
    window->renderers = NULL;
    window->next = ctx->windows;
    ctx->windows = window;

    XSetWindowAttributes attrs;
    attrs.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | Expose;
    Window x11_window = XCreateWindow(
        ctx->dpy, ctx->root, x, y, w, h,
        0, CopyFromParent, InputOutput, CopyFromParent,
        CWEventMask, &attrs
    );

    if (!x11_window) {
        free(window->title);
        free(window);
        set_error("Failed to create X11 window");
        return NULL;
    }

    XStoreName(ctx->dpy, x11_window, window->title);
    XSetWMProtocols(ctx->dpy, x11_window, &ctx->wm_delete, 1);

    XMapWindow(ctx->dpy, x11_window);

    if (flags & BUMI_WINDOW_CLEAR) {
        GC gc = XCreateGC(ctx->dpy, x11_window, 0, NULL);
        XSetForeground(ctx->dpy, gc, BlackPixel(ctx->dpy, ctx->screen));
        XFillRectangle(ctx->dpy, x11_window, gc, 0, 0, w, h);
        XFreeGC(ctx->dpy, gc);
    }

    XFlush(ctx->dpy);

    window->backend_data = (void*)(uintptr_t)x11_window;
    return window;
}

void BUMI_WindowDestroy(BUMI_Window* window) {
    if (!window) return;

    BUMI_ClearError();

    BUMI_Renderer* renderer = window->renderers;
    while (renderer) {
        BUMI_Renderer* next = renderer->next;
        BUMI_RendererDestroy(renderer);
        renderer = next;
    }

    Window x11_window = (Window)(uintptr_t)window->backend_data;
    if (x11_window) {
        BUMI_Window* prev = NULL;
        BUMI_Window* current = ctx->windows;
        while (current && current != window) {
            prev = current;
            current =  current->next;
        }
        if (current) {
            if (prev) {
                prev->next = current->next;
            } else {
                ctx->windows = current->next;
            }
        }

        XDestroyWindow(ctx->dpy, x11_window);
        XFlush(ctx->dpy);
    }

    free(window->title);
    free(window);
}

BUMI_Renderer* BUMI_RendererCreate(BUMI_Window* window, int index, uint32_t flags) {
    BUMI_ClearError();

    if (!window) {
        set_error("Invalid window for renderer creation");
        return NULL;
    }

    BUMI_Renderer* renderer = (BUMI_Renderer*) malloc(sizeof(BUMI_Renderer));
    if (!renderer) {
        set_error("Failed to allocate renderer");
        return NULL;
    }

    renderer->window = window;
    renderer->next = window->renderers;
    renderer->previous = NULL;
    renderer->draw_color[0] = 0.0f; // Default black
    renderer->draw_color[1] = 0.0f;
    renderer->draw_color[2] = 0.0f;
    renderer->draw_color[3] = 1.0f;

    int attribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
    XVisualInfo* vi = glXChooseVisual(ctx->dpy, ctx->screen, attribs);
    if (!vi) {
        free(renderer);
        set_error("Failed to choose GLX visual");
        return NULL;
    }

    renderer->renderer_data = glXCreateContext(ctx->dpy, vi, NULL, True);
    XFree(vi);
    if (!renderer->renderer_data) {
        free(renderer);
        set_error("Failed to create GLX context");
        return NULL;
    }

    glXMakeCurrent(ctx->dpy, (Window)(uintptr_t)window->backend_data, (GLXContext) renderer->renderer_data);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glXSwapBuffers(ctx->dpy, (Window)(uintptr_t)window->backend_data);

    if (window->renderers) {
        window->renderers->previous = renderer;
    }
    window->renderers = renderer;

    return renderer;
}

void BUMI_RendererDestroy(BUMI_Renderer* renderer) {
    if (!renderer) return;

    BUMI_ClearError();

    if (renderer->previous) {
        renderer->previous->next = renderer->next;
    } else if (renderer->window) {
        renderer->window->renderers = renderer->next;
    }
    if (renderer->next) {
        renderer->next->previous = renderer->previous;
    }

    if (renderer->renderer_data) {
        glXDestroyContext(ctx->dpy, (GLXContext) renderer->renderer_data);
    }
    free(renderer);
}

int BUMI_SetRenderDrawColor(BUMI_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    BUMI_ClearError();

    if (!renderer || !renderer->renderer_data) {
        set_error("Invalid renderer for setting draw color");
        return -1;
    }

    renderer->draw_color[0] = r / 255.0f;
    renderer->draw_color[1] = g / 255.0f;
    renderer->draw_color[2] = b / 255.0f;
    renderer->draw_color[3] = a / 255.0f;
    return 0;
}

int BUMI_RenderClear(BUMI_Renderer* renderer) {
    BUMI_ClearError();

    if (!renderer || !renderer->renderer_data || !renderer->window) {
        set_error("Invalid renderer for clearing");
        return -1;
    }

    glXMakeCurrent(ctx->dpy, (Window)(uintptr_t)renderer->window->backend_data, (GLXContext) renderer->renderer_data);
    glClearColor(renderer->draw_color[0], renderer->draw_color[1], renderer->draw_color[2], renderer->draw_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

int BUMI_RenderFillRect(BUMI_Renderer* renderer, const BUMI_Rect* rect) {
    BUMI_ClearError();

    if (!renderer || !renderer->renderer_data || !renderer->window) {
        set_error("Invalid renderer for drawing rectangle");
        return -1;
    }

    glXMakeCurrent(ctx->dpy, (Window)(uintptr_t)renderer->window->backend_data, (GLXContext) renderer->renderer_data);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, renderer->window->w, renderer->window->h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float x = rect ? rect->x : 0;
    float y = rect ? rect->y : 0;
    float w = rect ? rect->w : renderer->window->w;
    float h = rect ? rect->h : renderer->window->h;

    glColor4fv(renderer->draw_color);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();

    return 0;
}

void BUMI_RenderPresent(BUMI_Renderer* renderer) {
    BUMI_ClearError();

    if (!renderer || !renderer->renderer_data || !renderer->window) {
        set_error("Invalid renderer for presenting");
        return;
    }

    glXMakeCurrent(ctx->dpy, (Window)(uintptr_t)renderer->window->backend_data, (GLXContext) renderer->renderer_data);
    glXSwapBuffers(ctx->dpy, (Window)(uintptr_t)renderer->window->backend_data);
}

void BUMI_Delay(uint32_t ms) {
    usleep(ms * 1000);
}

static BUMI_Keycode x11_to_bumi_keycode(KeySym keysym) {
    switch (keysym) {
        case XK_a: return BUMI_KEY_A;
        case XK_b: return BUMI_KEY_B;
        case XK_c: return BUMI_KEY_C;
        case XK_d: return BUMI_KEY_D;
        case XK_e: return BUMI_KEY_E;
        case XK_f: return BUMI_KEY_F;
        case XK_g: return BUMI_KEY_G;
        case XK_h: return BUMI_KEY_H;
        case XK_i: return BUMI_KEY_I;
        case XK_j: return BUMI_KEY_J;
        case XK_k: return BUMI_KEY_K;
        case XK_l: return BUMI_KEY_L;
        case XK_m: return BUMI_KEY_M;
        case XK_n: return BUMI_KEY_N;
        case XK_o: return BUMI_KEY_O;
        case XK_p: return BUMI_KEY_P;
        case XK_q: return BUMI_KEY_Q;
        case XK_r: return BUMI_KEY_R;
        case XK_s: return BUMI_KEY_S;
        case XK_t: return BUMI_KEY_T;
        case XK_u: return BUMI_KEY_U;
        case XK_v: return BUMI_KEY_V;
        case XK_w: return BUMI_KEY_W;
        case XK_x: return BUMI_KEY_X;
        case XK_y: return BUMI_KEY_Y;
        case XK_z: return BUMI_KEY_Z;
        case XK_0: return BUMI_KEY_0;
        case XK_1: return BUMI_KEY_1;
        case XK_2: return BUMI_KEY_2;
        case XK_3: return BUMI_KEY_3;
        case XK_4: return BUMI_KEY_4;
        case XK_5: return BUMI_KEY_5;
        case XK_6: return BUMI_KEY_6;
        case XK_7: return BUMI_KEY_7;
        case XK_8: return BUMI_KEY_8;
        case XK_9: return BUMI_KEY_9;
        case XK_Return: return BUMI_KEY_RETURN;
        case XK_Escape: return BUMI_KEY_ESCAPE;
        case XK_BackSpace: return BUMI_KEY_BACKSPACE;
        case XK_Tab: return BUMI_KEY_TAB;
        case XK_space: return BUMI_KEY_SPACE;
        case XK_minus: return BUMI_KEY_MINUS;
        case XK_equal: return BUMI_KEY_EQUALS;
        case XK_bracketleft: return BUMI_KEY_LEFTBRACKET;
        case XK_bracketright: return BUMI_KEY_RIGHTBRACKET;
        case XK_backslash: return BUMI_KEY_BACKSLASH;
        case XK_semicolon: return BUMI_KEY_SEMICOLON;
        case XK_apostrophe: return BUMI_KEY_APOSTROPHE;
        case XK_grave: return BUMI_KEY_GRAVE;
        case XK_comma: return BUMI_KEY_COMMA;
        case XK_period: return BUMI_KEY_PERIOD;
        case XK_slash: return BUMI_KEY_SLASH;
        case XK_F1: return BUMI_KEY_F1;
        case XK_F2: return BUMI_KEY_F2;
        case XK_F3: return BUMI_KEY_F3;
        case XK_F4: return BUMI_KEY_F4;
        case XK_F5: return BUMI_KEY_F5;
        case XK_F6: return BUMI_KEY_F6;
        case XK_F7: return BUMI_KEY_F7;
        case XK_F8: return BUMI_KEY_F8;
        case XK_F9: return BUMI_KEY_F9;
        case XK_F10: return BUMI_KEY_F10;
        case XK_F11: return BUMI_KEY_F11;
        case XK_F12: return BUMI_KEY_F12;
        case XK_Up: return BUMI_KEY_UP;
        case XK_Down: return BUMI_KEY_DOWN;
        case XK_Left: return BUMI_KEY_LEFT;
        case XK_Right: return BUMI_KEY_RIGHT;
        case XK_Shift_L: return BUMI_KEY_LSHIFT;
        case XK_Shift_R: return BUMI_KEY_RSHIFT;
        case XK_Control_L: return BUMI_KEY_LCTRL;
        case XK_Control_R: return BUMI_KEY_RCTRL;
        case XK_Alt_L: return BUMI_KEY_LALT;
        case XK_Alt_R: return BUMI_KEY_RALT;
        default: return BUMI_KEY_UNKNOWN;
    }
}

int BUMI_PollEvent(BUMI_Event* event) {
    BUMI_ClearError();

    if (!ctx || !event) {
        set_error("Event polling requires initialized context and valid event pointer");
        return 0;
    }

    if (!XPending(ctx->dpy)) {
        return 0;
    }

    XEvent xevent;
    XNextEvent(ctx->dpy, &xevent);

    memset(event, 0, sizeof(BUMI_Event));
    event->key.timestamp = (uint32_t)(time(NULL) * 1000);
    event->key.windowID = (BUMI_WindowID)(uintptr_t)xevent.xany.window;

    BUMI_Window* window = find_window(xevent.xany.window);

    if (xevent.type == ClientMessage && xevent.xclient.data.l[0] == ctx->wm_delete) {
        event->type = BUMI_WINDOWEVENT;
        event->window.window_event = BUMI_WINDOWEVENT_CLOSE;
        return 1;
    } else if (xevent.type == KeyPress) {
        event->type = BUMI_KEYDOWN;
        event->key.keycode = x11_to_bumi_keycode(XKeycodeToKeysym(ctx->dpy, xevent.xkey.keycode, 0));
        return 1;
    } else if (xevent.type == KeyRelease) {
        event->type = BUMI_KEYUP;
        event->key.keycode = x11_to_bumi_keycode(XKeycodeToKeysym(ctx->dpy, xevent.xkey.keycode, 0));
        return 1;
    } else if (xevent.type == ConfigureNotify) {
        if (window) {
            window->w = xevent.xconfigure.width;
            window->h = xevent.xconfigure.height;
            window->x = xevent.xconfigure.x;
            window->y = xevent.xconfigure.y;
            event->type = BUMI_WINDOWEVENT;
            event->window.window_event = BUMI_WINDOWEVENT_RESIZED;
            return 1;
        }
        return 0;
    } else if (xevent.type == Expose) {
        if (window) {
            if (window->renderers) {
                BUMI_Renderer* renderer = window->renderers;
                BUMI_RenderClear(renderer);
                BUMI_RenderFillRect(renderer, NULL);
                BUMI_RenderPresent(renderer);
            } else if (window->flags & BUMI_WINDOW_CLEAR) {
                GC gc = XCreateGC(ctx->dpy, xevent.xexpose.window, 0, NULL);
                XSetForeground(ctx->dpy, gc, BlackPixel(ctx->dpy, ctx->screen));
                XFillRectangle(ctx->dpy, xevent.xexpose.window, gc, 0, 0, window->w, window->h);
                XFreeGC(ctx->dpy, gc);
                XFlush(ctx->dpy);
            }
        }
        return 0;
    }

    return 0;
}

int BUMI_WaitEvent(BUMI_Event* event) {
    BUMI_ClearError();

    if (!ctx || !event) {
        set_error("Event waiting requires initialized context and valid event pointer");
        return 0;
    }

    XEvent xevent;
    XNextEvent(ctx->dpy, &xevent);

    memset(event, 0, sizeof(BUMI_Event));
    event->key.timestamp = (uint32_t)(time(NULL) * 1000);
    event->key.windowID = (BUMI_WindowID)(uintptr_t)xevent.xany.window;

    BUMI_Window* window = find_window(xevent.xany.window);

    if (xevent.type == ClientMessage && xevent.xclient.data.l[0] == ctx->wm_delete) {
        event->type = BUMI_WINDOWEVENT;
        event->window.window_event = BUMI_WINDOWEVENT_CLOSE;
        return 1;
    } else if (xevent.type == KeyPress) {
        event->type = BUMI_KEYDOWN;
        event->key.keycode = x11_to_bumi_keycode(XKeycodeToKeysym(ctx->dpy, xevent.xkey.keycode, 0));
        return 1;
    } else if (xevent.type == KeyRelease) {
        event->type = BUMI_KEYUP;
        event->key.keycode = x11_to_bumi_keycode(XKeycodeToKeysym(ctx->dpy, xevent.xkey.keycode, 0));
        return 1;
    } else if (xevent.type == ConfigureNotify) {
        if (window) {
            window->w = xevent.xconfigure.width;
            window->h = xevent.xconfigure.height;
            window->x = xevent.xconfigure.x;
            window->y = xevent.xconfigure.y;
            event->type = BUMI_WINDOWEVENT;
            event->window.window_event = BUMI_WINDOWEVENT_RESIZED;
            return 1;
        }
        return 0;
    } else if (xevent.type == Expose) {
        if (window) {
            if (window->renderers) {
                BUMI_Renderer* renderer = window->renderers;
                BUMI_RenderClear(renderer);
                BUMI_RenderFillRect(renderer, NULL);
                BUMI_RenderPresent(renderer);
            } else if (window->flags & BUMI_WINDOW_CLEAR) {
                GC gc = XCreateGC(ctx->dpy, xevent.xexpose.window, 0, NULL);
                XSetForeground(ctx->dpy, gc, BlackPixel(ctx->dpy, ctx->screen));
                XFillRectangle(ctx->dpy, xevent.xexpose.window, gc, 0, 0, window->w, window->h);
                XFreeGC(ctx->dpy, gc);
                XFlush(ctx->dpy);
            }
        }
        return 0;
    }

    return 0;
}