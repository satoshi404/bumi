#ifndef BUMI_EVENTS_H
#define BUMI_EVENTS_H

#include <stdint.h>

// Event types
#define BUMI_QUIT 0x100
#define BUMI_KEYDOWN 0x300
#define BUMI_KEYUP 0x301
#define BUMI_WINDOWEVENT 0x200
#define BUMI_WINDOWEVENT_CLOSE 4
#define BUMI_WINDOWEVENT_RESIZED 5

typedef uint32_t BUMI_WindowID;

typedef enum {
    BUMI_KEY_UNKNOWN = 0,
    BUMI_KEY_A = 'a',
    BUMI_KEY_B = 'b',
    BUMI_KEY_C = 'c',
    BUMI_KEY_D = 'd',
    BUMI_KEY_E = 'e',
    BUMI_KEY_F = 'f',
    BUMI_KEY_G = 'g',
    BUMI_KEY_H = 'h',
    BUMI_KEY_I = 'i',
    BUMI_KEY_J = 'j',
    BUMI_KEY_K = 'k',
    BUMI_KEY_L = 'l',
    BUMI_KEY_M = 'm',
    BUMI_KEY_N = 'n',
    BUMI_KEY_O = 'o',
    BUMI_KEY_P = 'p',
    BUMI_KEY_Q = 'q',
    BUMI_KEY_R = 'r',
    BUMI_KEY_S = 's',
    BUMI_KEY_T = 't',
    BUMI_KEY_U = 'u',
    BUMI_KEY_V = 'v',
    BUMI_KEY_W = 'w',
    BUMI_KEY_X = 'x',
    BUMI_KEY_Y = 'y',
    BUMI_KEY_Z = 'z',
    BUMI_KEY_0 = '0',
    BUMI_KEY_1 = '1',
    BUMI_KEY_2 = '2',
    BUMI_KEY_3 = '3',
    BUMI_KEY_4 = '4',
    BUMI_KEY_5 = '5',
    BUMI_KEY_6 = '6',
    BUMI_KEY_7 = '7',
    BUMI_KEY_8 = '8',
    BUMI_KEY_9 = '9',
    BUMI_KEY_RETURN = '\r',
    BUMI_KEY_ESCAPE = 27,
    BUMI_KEY_BACKSPACE = '\b',
    BUMI_KEY_TAB = '\t',
    BUMI_KEY_SPACE = ' ',
    BUMI_KEY_MINUS = '-',
    BUMI_KEY_EQUALS = '=',
    BUMI_KEY_LEFTBRACKET = '[',
    BUMI_KEY_RIGHTBRACKET = ']',
    BUMI_KEY_BACKSLASH = '\\',
    BUMI_KEY_SEMICOLON = ';',
    BUMI_KEY_APOSTROPHE = '\'',
    BUMI_KEY_GRAVE = '`',
    BUMI_KEY_COMMA = ',',
    BUMI_KEY_PERIOD = '.',
    BUMI_KEY_SLASH = '/',
    BUMI_KEY_F1 = 0x1000,
    BUMI_KEY_F2,
    BUMI_KEY_F3,
    BUMI_KEY_F4,
    BUMI_KEY_F5,
    BUMI_KEY_F6,
    BUMI_KEY_F7,
    BUMI_KEY_F8,
    BUMI_KEY_F9,
    BUMI_KEY_F10,
    BUMI_KEY_F11,
    BUMI_KEY_F12,
    BUMI_KEY_UP,
    BUMI_KEY_DOWN,
    BUMI_KEY_LEFT,
    BUMI_KEY_RIGHT,
    BUMI_KEY_LSHIFT,
    BUMI_KEY_RSHIFT,
    BUMI_KEY_LCTRL,
    BUMI_KEY_RCTRL,
    BUMI_KEY_LALT,
    BUMI_KEY_RALT
} BUMI_Keycode;

typedef struct {
    uint32_t type; // BUMI_KEYDOWN, BUMI_KEYUP
    uint32_t timestamp;
    BUMI_WindowID windowID;
    BUMI_Keycode keycode;
} BUMI_KeyEvent;

typedef struct {
    uint32_t type; // BUMI_WINDOWEVENT
    uint32_t timestamp;
    BUMI_WindowID windowID;
    uint8_t window_event; // BUMI_WINDOWEVENT_CLOSE, BUMI_WINDOWEVENT_RESIZED
} BUMI_WindowEvent;

typedef union {
    uint32_t type;
    BUMI_KeyEvent key;
    BUMI_WindowEvent window;
} BUMI_Event;

#endif