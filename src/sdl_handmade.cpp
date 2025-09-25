// #include "SDL_render.h"
#include "SDL_events.h"
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

struct sdl_offscreen_buffer {
  void *Memory;
  SDL_Texture *Texture;
  int Width;
  int Height;
  int BytesPerPixel;
};

internal void RenderWeirdGradient(sdl_offscreen_buffer buf, int XOffset,
                                  int YOffset) {
  int Width = buf.Width;
  int Height = buf.Height;
  uint8 *Row = (uint8 *)buf.Memory;
  int Pitch = Width * buf.BytesPerPixel;
  for (int Y = 0; Y < buf.Height; ++Y) {
    uint32 *Pixel = (uint32 *)Row;
    // uint8 *Pixel = (uint8 *)Row;
    for (int X = 0; X < buf.Width; ++X) {
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);

      *Pixel++ = ((Green << 8) | Blue);
    }

    Row += Pitch;
  }
}

internal void SDLResizeTexture(sdl_offscreen_buffer *buf,
                               SDL_Renderer *renderer, int width, int height) {

  if (buf->Memory) {
    munmap(buf->Memory, buf->Height * buf->Width * buf->BytesPerPixel);
  }
  if (buf->Texture) {
    SDL_DestroyTexture(buf->Texture);
  }

  buf->Height = height;
  buf->Width = width;

  buf->Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);

  buf->Memory =
      mmap(0, height * width * buf->BytesPerPixel, PROT_WRITE | PROT_READ,
           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // RenderWeirdGradient(0, 0);
}

internal void SDLUpdateWindow(sdl_offscreen_buffer *buf, SDL_Window *Window,
                              SDL_Renderer *Renderer) {
  SDL_UpdateTexture(buf->Texture, 0, buf->Memory,
                    buf->Width * buf->BytesPerPixel);

  SDL_RenderCopy(Renderer, buf->Texture, 0, 0);

  SDL_RenderPresent(Renderer);
}

bool HandleEvent(SDL_Event *Event, sdl_offscreen_buffer *buf) {
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
          // SDLResizeTexture(buf, Renderer, Event->window.data1,
          //                  Event->window.data2);
        } break;
        case SDL_WINDOWEVENT_EXPOSED: {
          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);
          SDLUpdateWindow(buf, Window, Renderer);
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

  int Width, Height;
  SDL_GetWindowSize(Window, &Width, &Height);

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  if (!Renderer) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }

  struct sdl_offscreen_buffer buf{
      .BytesPerPixel = 4,
  };
  int Pitch = Width * buf.BytesPerPixel;
  SDLResizeTexture(&buf, Renderer, Width, Height);
  SDL_ShowWindow(Window);

  bool Running = true;

  int XOffset = 0;
  int YOffset = 0;
  while (Running) {
    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
      if (HandleEvent(&Event, &buf)) {
        Running = false;
      }
    }

    RenderWeirdGradient(buf, XOffset, YOffset);
    SDLUpdateWindow(&buf, Window, Renderer);
    XOffset += 12;
  }

  SDL_Quit();
  return (0);
}
