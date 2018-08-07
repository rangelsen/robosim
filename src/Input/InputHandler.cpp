#include <iostream>

#include "InputHandler.hpp"
#include "Keyboard.hpp"

#define MOVEMENT_SPEED 0.03f
#define ROTATION_SPEED 0.020f

////////////////////////////////////////////////////////////////////////////////
void InputHandler::HandleInputs(Display* display, Camera* camera) {

    SDL_PumpEvents();
    std::map<SDL_Keycode, bool> key_buffer = Keyboard::GetKeyBuffer();

    DetectAndExecuteEvents(key_buffer, display, camera);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {

            display->Quit();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void InputHandler::DetectAndExecuteEvents(
    std::map<SDL_Keycode, bool> key_buffer,Display* display, Camera* camera) {
    
    for (std::map<SDL_Keycode, bool>::iterator it = key_buffer.begin();
         it != key_buffer.end(); it++) {


        if(it->second) {

            switch(it->first) {

                case SDLK_a:
                    camera->MoveRight(-MOVEMENT_SPEED);
                    break;

                case SDLK_w:
                    camera->MoveForward(MOVEMENT_SPEED);
                    break;

                case SDLK_s:
                    camera->MoveForward(-MOVEMENT_SPEED);
                    break;

                case SDLK_d:
                    camera->MoveRight(MOVEMENT_SPEED);
                    break;

                case SDLK_RIGHT:
                    camera->RotateVertical(-ROTATION_SPEED);
                    break;

                case SDLK_LEFT:
                    camera->RotateVertical(ROTATION_SPEED);
                    break;

                case SDLK_UP:
                    camera->RotateHorizontal(ROTATION_SPEED);
                    break;

                case SDLK_DOWN:
                    camera->RotateHorizontal(-ROTATION_SPEED);
                    break;

                case SDLK_SPACE:
                    camera->MoveUp(MOVEMENT_SPEED);
                    break;

                case SDLK_LCTRL:
                    camera->MoveUp(-MOVEMENT_SPEED);
                    break;

                case SDLK_ESCAPE:
                    display->Quit();
                    break;
            }
        }
    }
}

/// @file

