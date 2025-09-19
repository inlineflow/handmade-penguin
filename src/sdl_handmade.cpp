// #include "SDL_render.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <cstdint>
#include <stdint.h>
#include <sys/mman.h>

#define global_variable static
#define internal static

// typedef unsigned char uint8;
typedef uint8_t uint8;
typedef uint32_t uint32;

global_variable void *BitmapMemory;
global_variable SDL_Texture *Texture;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
const int BytesPerPixel = 4;

internal void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
  if (BitmapMemory) {
    munmap(BitmapMemory, height * width * BytesPerPixel);
  }

  BitmapHeight = height;
  BitmapWidth = width;

  if (Texture) {
    SDL_DestroyTexture(Texture);
  }

  Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);

  BitmapMemory = mmap(0, height * width * BytesPerPixel, PROT_WRITE | PROT_READ,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  int Pitch = width * BytesPerPixel;
  uint8 *Row = (uint8 *)BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y) {
    // uint32 *Pixel = (uint32 *)Row;
    uint8 *Pixel = (uint8 *)Row;
    for (int X = 0; X < BitmapWidth; ++X) {
      *Pixel = 255;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;
    }

    Row += Pitch;
  }
}

internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer) {
  SDL_UpdateTexture(Texture, 0, BitmapMemory, BitmapWidth * BytesPerPixel);

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
          SDLResizeTexture(Renderer, Event->window.data1, Event->window.data2);
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
