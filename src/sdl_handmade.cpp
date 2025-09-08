// #include "SDL_render.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>

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
        case SDL_WINDOWEVENT_RESIZED: {
          printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1,
                 Event->window.data2);
        } break;
        case SDL_WINDOWEVENT_EXPOSED: {
          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);
          static bool IsWhite = true;
          if (IsWhite == true) {
            SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
            IsWhite = false;
          } else {
            SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
            IsWhite = true;
          }
          SDL_RenderClear(Renderer);
          SDL_RenderPresent(Renderer);
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
