#ifndef BUMI_SYSVIDEO_H
#define BUMI_SYSVIDEO_H

// === SYSTEM VIDEO BUMI LIBRARY LIKE SDL2 ===

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h> 

// define XLIB_OFFICIAL to 1 to use the official X11 library
// define XLIB_OFFICIAL to 0 to use the custom X11 library
#define XLIB_OFFICIAL 1

// include the appropriate X11 library
#if XLIB_OFFICIAL
    #include <X11/Xlib.h>
    #include <X11/Xutil.h> // For XSetWindowAttributes
    #include <X11/keysym.h>
    #include <X11/X.h>
#else
    #include "backend/x11.h"
#endif

#include "bumi_syskey.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialization flags (like SDL_INIT_VIDEO)
#define BUMI_INIT_VIDEO 0x00000001u
#define BUMI_WINDOW_CLEAR 0x00000001u // Flag to clear window to black

typedef uint32_t BUMI_WindowID;
typedef uint32_t BUMI_DisplayID;
typedef uint64_t BUMI_WindowFlags;

struct BUMI_Window;

typedef struct BUMI_Renderer { 
    struct BUMI_Window* window; 
    struct BUMI_Renderer* next; 
    struct BUMI_Renderer* previous; 
    void* renderer_data; 
    //GLX context 
    float draw_color[4]; // RGBA draw color 
} BUMI_Renderer;


typedef struct BUMI_Window {
    BUMI_WindowID id;
    char* title;
    int x, y;
    int w, h;
    int min_w, max_w;
    int min_h, max_h;
    float min_aspect, max_aspect;
    BUMI_DisplayID last_fullscreen_exclusive_display;
    BUMI_DisplayID last_displayID;
    int last_pixel_w, last_pixel_h;
    BUMI_WindowFlags flags, pending_flags;
    float display_scale;
    bool external_graphics_context;
    bool fullscreen_exclusive;
    BUMI_Renderer* renderers; // Linked list head
    struct BUMI_Window* keyboard_focus;
    struct BUMI_Window* next;
    struct BUMI_Window* prev;
    struct BUMI_Window* parent;
    struct BUMI_Window* first_child;
    struct BUMI_Window* prev_sibling;
    struct BUMI_Window* next_sibling;
    void* backend_data; // X11 window handle
} BUMI_Window;

typedef struct { 
    int x, y; 
    int w, h; 
} BUMI_Rect;

// Initialize the Bumi system (like SDL_Init)
int BUMI_Init(
    uint32_t             // flags
);

// Clean up the Bumi system (like SDL_Quit)
void BUMI_Quit(void);

// Get the last error message (like SDL_GetError)
const char* BUMI_GetError(void);

// Clear the last error
void BUMI_ClearError(void);

void BUMI_ClearError(void);

BUMI_Window* BUMI_WindowCreate(
    const char*,                     // title
    int,                             // x
    int,                             // y
    int,                             // w
    int,                             // h
    uint32_t                         // flags
);
void BUMI_WindowDestroy(
    BUMI_Window*                     // window
);
BUMI_Renderer* BUMI_RendererCreate(
    BUMI_Window*,                    // window
    int,                             // index
    uint32_t                         // flags
);
void BUMI_RendererDestroy(
    BUMI_Renderer*                   //renderer
);

void BUMI_Quit();

int BUMI_PollEvent(
    BUMI_Event*                     // event
); 
int BUMI_WaitEvent(
    BUMI_Event*                     // event
);

int BUMI_SetRenderDrawColor(BUMI_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a); 
int BUMI_RenderClear(BUMI_Renderer* renderer); 
int BUMI_RenderFillRect(BUMI_Renderer* renderer, const BUMI_Rect* rect); \
void BUMI_RenderPresent(BUMI_Renderer* renderer); 
void BUMI_Delay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif