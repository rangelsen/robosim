#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <map>
#include <vector>
#include <SDL2/SDL.h>

#include <Graphics/Camera.hpp>
#include <Graphics/Display.hpp>

class InputHandler {

public:
    /**
     * @brief Read key presses and map these to the display and camera
     */
    static void HandleInputs(Display* display, Camera* camera);

private:
    /**
     * @brief Enumerate key presses and execute appropriate events
     */
    static void DetectAndExecuteEvents(std::map<SDL_Keycode, bool> key_buffer,
                                       Display* display, Camera* camera);
};

#endif // INPUT_HANDLER_H

/// @file

