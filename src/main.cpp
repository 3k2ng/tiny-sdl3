#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stddef.h>
#include <stdint.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

namespace {

constexpr const char* window_title = "tiny window";
constexpr int window_width = 960;
constexpr int window_height = 720;
constexpr float max_delta = 1.0f / 15.0f;

struct Input {
  bool pressed[SDL_SCANCODE_COUNT];
};

struct App {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* madoka_texture;
  Input input;
  uint64_t prev_ticks_ns;
};

inline bool key_held(SDL_Scancode scancode) {
  return SDL_GetKeyboardState(nullptr)[scancode];
}

inline bool key_pressed(const Input& input, SDL_Scancode scancode) {
  return input.pressed[scancode];
}

[[nodiscard]] bool asset_path(const char* relative, char* out_path, size_t out_path_capacity) {
  const char* base = SDL_GetBasePath();  // owned and cached by SDL; do not free
  if (base == nullptr) {
    SDL_Log("SDL_GetBasePath failed, falling back to the working directory: %s", SDL_GetError());
    base = "";
  }
  const int written = SDL_snprintf(out_path, out_path_capacity, "%s%s", base, relative);
  return written > 0 && static_cast<size_t>(written) < out_path_capacity;
}

[[nodiscard]] bool load_png_texture(SDL_Renderer* renderer, const char* path, SDL_Texture** out_texture) {
  SDL_Surface* surface = SDL_LoadPNG(path);
  if (surface == nullptr) {
    SDL_Log("SDL_LoadPNG(\"%s\") failed: %s", path, SDL_GetError());
    return false;
  }
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);
  if (texture == nullptr) {
    SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    return false;
  }
  *out_texture = texture;
  return true;
}

[[nodiscard]] bool app_init(App* app) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return false;
  }
  if (!SDL_CreateWindowAndRenderer(window_title, window_width, window_height, 0, &app->window, &app->renderer)) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    return false;
  }
  SDL_SetRenderVSync(app->renderer, 1);

  IMGUI_CHECKVERSION();
  if (ImGui::CreateContext() == nullptr) {
    SDL_Log("ImGui::CreateContext failed");
    return false;
  }
  ImGui::StyleColorsDark();
  if (!ImGui_ImplSDL3_InitForSDLRenderer(app->window, app->renderer)) {
    SDL_Log("ImGui_ImplSDL3_InitForSDLRenderer failed");
    return false;
  }
  if (!ImGui_ImplSDLRenderer3_Init(app->renderer)) {
    SDL_Log("ImGui_ImplSDLRenderer3_Init failed");
    return false;
  }

  SDL_srand(0);

  // init
  char madoka_path[1024];
  if (!asset_path("textures/madoka.png", madoka_path, sizeof(madoka_path))) {
    SDL_Log("asset path for textures/madoka.png does not fit in %zu bytes", sizeof(madoka_path));
    return false;
  }
  if (!load_png_texture(app->renderer, madoka_path, &app->madoka_texture)) {
    return false;
  }

  app->prev_ticks_ns = SDL_GetTicksNS();
  return true;
}

void app_shutdown(App* app) {
  // de-init
  if (app->madoka_texture != nullptr) {
    SDL_DestroyTexture(app->madoka_texture);
    app->madoka_texture = nullptr;
  }

  if (ImGui::GetCurrentContext() != nullptr) {
    const ImGuiIO& io = ImGui::GetIO();
    if (io.BackendRendererUserData != nullptr) {
      ImGui_ImplSDLRenderer3_Shutdown();
    }
    if (io.BackendPlatformUserData != nullptr) {
      ImGui_ImplSDL3_Shutdown();
    }
    ImGui::DestroyContext();
  }

  if (app->renderer != nullptr) {
    SDL_DestroyRenderer(app->renderer);
    app->renderer = nullptr;
  }
  if (app->window != nullptr) {
    SDL_DestroyWindow(app->window);
    app->window = nullptr;
  }
}

void app_update(App* app, float delta) {
  // update
}

void app_debug_ui(const App* app, float delta) {
  ImGui::Begin("debug");
  ImGui::Text("%.1f fps", delta > 0.0f ? 1.0f / delta : 0.0f);
  float w = 0.0f;
  float h = 0.0f;
  if (app->madoka_texture != nullptr && SDL_GetTextureSize(app->madoka_texture, &w, &h)) {
    ImGui::Text("madoka.png %.0fx%.0f", w, h);
    ImGui::Image(static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(app->madoka_texture)), ImVec2(w, h));
  }
  ImGui::End();
}

void app_render(const App* app) {
  SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->renderer);

  // render
  SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderDebugTextFormat(app->renderer, 0.0f, 0.0f, "test %d", 42);

  const SDL_FRect madoka_dstrect = {64.0f, 64.0f, 256.0f, 256.0f};
  SDL_RenderTexture(app->renderer, app->madoka_texture, nullptr, &madoka_dstrect);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), app->renderer);
  SDL_RenderPresent(app->renderer);
}

}  // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
  App* app = static_cast<App*>(SDL_calloc(1, sizeof(App)));
  if (app == nullptr) {
    SDL_Log("out of memory allocating App");
    return SDL_APP_FAILURE;
  }
  *appstate = app;
  if (!app_init(app)) {
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  App* app = static_cast<App*>(appstate);
  ImGui_ImplSDL3_ProcessEvent(event);
  SDL_ConvertEventToRenderCoordinates(app->renderer, event);
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
  if (event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat && event->key.scancode < SDL_SCANCODE_COUNT) {
    app->input.pressed[event->key.scancode] = true;
  }
  // event
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  App* app = static_cast<App*>(appstate);
  const uint64_t now_ns = SDL_GetTicksNS();
  const float raw_delta = static_cast<float>(now_ns - app->prev_ticks_ns) / 1e9f;
  app->prev_ticks_ns = now_ns;
  const float delta = raw_delta < max_delta ? raw_delta : max_delta;

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  app_update(app, delta);
  app_debug_ui(app, raw_delta);
  app_render(app);
  SDL_memset(app->input.pressed, 0, sizeof(app->input.pressed));
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  App* app = static_cast<App*>(appstate);
  if (app == nullptr) {
    return;
  }
  app_shutdown(app);
  SDL_free(app);
}
