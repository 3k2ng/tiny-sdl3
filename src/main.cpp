#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
Uint64 last_ticks = 0;
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_CreateWindowAndRenderer("tiny window", 800, 600, 0, &window, &renderer);
  // init
  last_ticks = SDL_GetTicks();
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  // event
  return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppIterate(void* appstate) {
  const Uint64 current_ticks = SDL_GetTicks();
  const float delta = (current_ticks - last_ticks) * 1e-3;
  last_ticks = current_ticks;
  SDL_RenderClear(renderer);
  // render
  SDL_RenderPresent(renderer);
  SDL_Delay(1);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void* appstate, SDL_AppResult result) { SDL_Quit(); }
