#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

constexpr const char* kWindowTitle = "tiny window";
constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 720;

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
Uint64 g_prev_ticks_ns = 0;
bool g_key_pressed[SDL_SCANCODE_COUNT] = {};

bool KeyHeld(SDL_Scancode scancode) {
  return SDL_GetKeyboardState(nullptr)[scancode];
}

bool KeyPressed(SDL_Scancode scancode) {
  return g_key_pressed[scancode];
}

SDL_Texture* g_madoka_texture = nullptr;

void Update(float delta) {
  // update
}

void DebugUi(float delta) {
  ImGui::Begin("debug");
  ImGui::Text("%.1f fps", delta > 0.0f ? 1.0f / delta : 0.0f);
  float w = 0.0f, h = 0.0f;
  if (g_madoka_texture && SDL_GetTextureSize(g_madoka_texture, &w, &h)) {
    ImGui::Text("madoka.png %.0fx%.0f", w, h);
    ImGui::Image((ImTextureID)(intptr_t)g_madoka_texture, ImVec2(w, h));
  }
  ImGui::End();
}

void Render() {
  SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(g_renderer);

  // render
  SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderDebugTextFormat(g_renderer, 0, 0, "test %d", 42);

  SDL_FRect madoka_dstrect;
  madoka_dstrect.w = 256;
  madoka_dstrect.h = 256;
  madoka_dstrect.x = 64;
  madoka_dstrect.y = 64;
  SDL_RenderTexture(g_renderer, g_madoka_texture, nullptr, &madoka_dstrect);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
  SDL_RenderPresent(g_renderer);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!SDL_CreateWindowAndRenderer(kWindowTitle, kWindowWidth, kWindowHeight, 0, &g_window, &g_renderer)) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderVSync(g_renderer, 1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
  ImGui_ImplSDLRenderer3_Init(g_renderer);

  SDL_srand(0);

  // init
  SDL_Surface* madoka_surface = SDL_LoadPNG("./textures/madoka.png");
  if (!madoka_surface) {
    SDL_Log("SDL_LoadPNG failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  g_madoka_texture = SDL_CreateTextureFromSurface(g_renderer, madoka_surface);
  SDL_DestroySurface(madoka_surface);

  g_prev_ticks_ns = SDL_GetTicksNS();
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  ImGui_ImplSDL3_ProcessEvent(event);
  SDL_ConvertEventToRenderCoordinates(g_renderer, event);
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  const ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse && (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_MOTION)) {
    return SDL_APP_CONTINUE;
  }
  if (io.WantCaptureKeyboard && event->type == SDL_EVENT_KEY_DOWN) {
    return SDL_APP_CONTINUE;
  }
  if (event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat) {
    g_key_pressed[event->key.scancode] = true;
  }
  // event
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  const Uint64 now_ns = SDL_GetTicksNS();
  const float delta = static_cast<float>(now_ns - g_prev_ticks_ns) / 1e9f;
  g_prev_ticks_ns = now_ns;
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  Update(delta);
  DebugUi(delta);
  Render();
  SDL_memset(g_key_pressed, 0, sizeof(g_key_pressed));
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  // de-init
  SDL_DestroyTexture(g_madoka_texture);

  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}
