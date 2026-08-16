#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
Uint64 g_last_ticks = 0;
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
  if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!SDL_CreateWindowAndRenderer("tiny window", 800, 600, 0, &g_window, &g_renderer)) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  // init
  g_last_ticks = SDL_GetTicks();
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
  const float delta = (current_ticks - g_last_ticks) * 1e-3;
  g_last_ticks = current_ticks;
  SDL_RenderClear(g_renderer);
  // render
  SDL_RenderPresent(g_renderer);
  SDL_Delay(1);
  return SDL_APP_CONTINUE;
}
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  // de-init
  SDL_DestroyRenderer(g_renderer);
  SDL_DestroyWindow(g_window);
  SDL_Quit();
}
