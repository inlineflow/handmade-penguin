// #include "SDL_render.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <sys/mman.h>

#define global_variable static
#define internal static

global_variable void *pixels;
global_variable SDL_Texture *Texture;
global_variable int TextureWidth;

internal void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
  if (pixels) {
    munmap(pixels, 4096);
  }

  if (Texture) {
    SDL_DestroyTexture(Texture);
  }

  SDL_Texture *Texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, width, height);

  void *pixels = mmap(nullptr, 4096, PROT_WRITE | PROT_READ,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}

internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer) {
  SDL_UpdateTexture(Texture, 0, pixels, TextureWidth * 4);

  SDL_RenderCopy(Renderer, Texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

// mine
bool HandleEvent(SDL_Event *Event) {
  bool ShouldQuit = false;
  switch (Event->type) {
    case SDL_QUIT: {
      printf("SDL_QUIT\n");
      ShouldQuit = true;
    } break;
    case SDL_WINDOWEVENT: {
      switch (Event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);
          SDLUpdateWindow(Window, Renderer);
        } break;
        case SDL_WINDOWEVENT_EXPOSED: {
          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);
          SDLUpdateWindow(Window, Renderer);
        } break;
      }

    } break;
  }
  return (ShouldQuit);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *Window =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);

  if (!Window) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  if (!Renderer) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }

  SDL_ShowWindow(Window);

  for (;;) {
    SDL_Event Event;
    SDL_WaitEvent(&Event);
    if (HandleEvent(&Event)) {
      break;
    }
  }

  SDL_Quit();
  return (0);
}
