#include <ventor/bumi_sysvideo.h>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    if (BUMI_Init(BUMI_INIT_VIDEO) != 0) {
        std::cout << "Test failed: Initialization error: " << BUMI_GetError() << std::endl;
        return 1;
    }

    BUMI_Window* window = BUMI_WindowCreate("Test Window", 100, 100, 800, 600, BUMI_WINDOW_CLEAR);
    if (!window) {
        std::cout << "Test failed: Window creation error: " << BUMI_GetError() << std::endl;
        BUMI_Quit();
        return 1;
    }

    BUMI_Renderer* renderer = BUMI_RendererCreate(window, -1, 0);
    if (!renderer) {
        std::cout << "Test failed: Renderer creation error: " << BUMI_GetError() << std::endl;
        BUMI_WindowDestroy(window);
        BUMI_Quit();
        return 1;
    }

    BUMI_Rect rect = {100, 100, 200, 200};
    bool resize_received = false;
    bool keydown_received = false;
    bool close_received = false;

    BUMI_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    BUMI_RenderClear(renderer);
    BUMI_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red rectangle
    BUMI_RenderFillRect(renderer, &rect);
    BUMI_RenderPresent(renderer);

    auto start = std::chrono::steady_clock::now();
    BUMI_Event event;
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(3)) {
        while (BUMI_PollEvent(&event)) {
            if (event.type == BUMI_WINDOWEVENT) {
                if (event.window.window_event == BUMI_WINDOWEVENT_RESIZED) {
                    resize_received = true;
                    rect.w = window->w / 4;
                    rect.h = window->h / 4;
                    if (rect.x > window->w - rect.w) rect.x = window->w - rect.w;
                } else if (event.window.window_event == BUMI_WINDOWEVENT_CLOSE) {
                    close_received = true;
                }
            }
            if (event.type == BUMI_KEYDOWN && event.key.keycode == BUMI_KEY_ESCAPE) {
                keydown_received = true;
            }
        }
        BUMI_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        BUMI_RenderClear(renderer);
        BUMI_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        BUMI_RenderFillRect(renderer, &rect);
        BUMI_RenderPresent(renderer);
        BUMI_Delay(16);
    }

    std::cout << "Test results:" << std::endl;
    std::cout << "Window created successfully: " << (window ? "PASS" : "FAIL") << std::endl;
    std::cout << "Renderer created successfully: " << (renderer ? "PASS" : "FAIL") << std::endl;
    std::cout << "Resize event received: " << (resize_received ? "PASS" : "SKIPPED (resize window during test)") << std::endl;
    std::cout << "Escape key event received: " << (keydown_received ? "PASS" : "SKIPPED (press Escape during test)") << std::endl;
    std::cout << "Close event received: " << (close_received ? "PASS" : "SKIPPED (close window during test)") << std::endl;

    BUMI_RendererDestroy(renderer);
    BUMI_WindowDestroy(window);
    BUMI_Quit();

    if (!window || !renderer) {
        return 1;
    }
    return 0;
}