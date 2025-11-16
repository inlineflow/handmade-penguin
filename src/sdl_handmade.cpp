// #include "SDL_render.h"
#include "SDL_audio.h"
#include "SDL_events.h"
#include "SDL_gamecontroller.h"
#include "SDL_haptic.h"
#include "SDL_joystick.h"
#include "SDL_render.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"
#include <SDL.h>
#include <cstdint>
#include <stdint.h>
#include <sys/mman.h>

#define global_variable static
#define internal static
#define MAX_CONTROLLERS 4
SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
SDL_Haptic *RumbleHandles[MAX_CONTROLLERS];

// typedef unsigned char uint8;
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef int32_t int32;
typedef int16_t int16;
struct sdl_offscreen_buffer {
  // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
  void *Memory;
  SDL_Texture *Texture;
  int Width;
  int Height;
  int Pitch;
};

internal void RenderWeirdGradient(sdl_offscreen_buffer buf, int XOffset,
                                  int YOffset) {
  int Width = buf.Width;
  int Height = buf.Height;
  uint8 *Row = (uint8 *)buf.Memory;
  // int Pitch = Width * buf.BytesPerPixel;
  int Pitch = buf.Pitch;
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

struct sdl_window_dimension {
  int width;
  int height;
};

sdl_window_dimension SDLGetWindowDimension(SDL_Window *Window) {
  sdl_window_dimension Result;

  SDL_GetWindowSize(Window, &Result.width, &Result.height);
  return Result;
}

internal void SDLResizeTexture(sdl_offscreen_buffer *buf,
                               SDL_Renderer *renderer, int width, int height) {

  const int BytesPerPixel = 4;
  if (buf->Memory) {
    munmap(buf->Memory, buf->Height * buf->Width * BytesPerPixel);
  }
  if (buf->Texture) {
    SDL_DestroyTexture(buf->Texture);
  }

  buf->Height = height;
  buf->Width = width;
  buf->Pitch = width * BytesPerPixel;

  buf->Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);

  buf->Memory = mmap(0, height * width * BytesPerPixel, PROT_WRITE | PROT_READ,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // RenderWeirdGradient(0, 0);
}

internal void SDLUpdateWindow(sdl_offscreen_buffer *buf, SDL_Window *Window,
                              SDL_Renderer *Renderer) {
  SDL_UpdateTexture(buf->Texture, 0, buf->Memory, buf->Pitch);

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

void setup_controllers(void) {
  int CountJoysticks = SDL_NumJoysticks();
  int controller_index = 0;
  for (int joystick_index = 0; joystick_index < CountJoysticks;
       ++joystick_index) {
    if (!SDL_IsGameController(joystick_index)) {
      continue;
    }

    if (controller_index >= MAX_CONTROLLERS) {
      break;
    }
    ControllerHandles[controller_index] =
        SDL_GameControllerOpen(joystick_index);
    RumbleHandles[controller_index] = SDL_HapticOpen(joystick_index);
    SDL_Haptic *rumble = RumbleHandles[controller_index];
    if (rumble && SDL_HapticRumbleInit(rumble) != 0) {
      SDL_HapticClose(rumble);
      rumble = 0;
    }

    controller_index++;
  }
}

internal void SDLAudioCallback(void *UserData, Uint8 *AudioData, int Length) {
  memset(AudioData, 0, Length);
}

void setup_audio(int32 SamplesPerSecond, int32 BufferSize) {
  SDL_AudioSpec audio_settings = {0};
  audio_settings.freq = SamplesPerSecond;
  audio_settings.format = AUDIO_S16;
  audio_settings.channels = 2;
  audio_settings.samples = BufferSize;
  audio_settings.callback = &SDLAudioCallback;

  SDL_OpenAudio(&audio_settings, 0);
}

void cleanup() {
  SDL_Quit();
  SDL_CloseAudio();
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC |
           SDL_INIT_AUDIO);

  SDL_Window *Window =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);

  if (!Window) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }

  sdl_window_dimension dimension = SDLGetWindowDimension(Window);

  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);
  if (!Renderer) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }

  struct sdl_offscreen_buffer buf{};
  SDLResizeTexture(&buf, Renderer, dimension.width, dimension.height);
  SDL_ShowWindow(Window);
  setup_controllers();
  setup_audio(48000, 4096);

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

    for (int ControllerIndex = 0; ControllerIndex < MAX_CONTROLLERS;
         ++ControllerIndex) {

      SDL_GameController *controller = ControllerHandles[ControllerIndex];

      if (controller != 0 && SDL_GameControllerGetAttached(controller)) {
        int16 StickX =
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);

        int16 StickY =
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        XOffset += 4 * (double)StickX / 32767.0;
        YOffset += 4 * ((double)StickY / 32767.0);

        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
          SDL_Haptic *rumble = RumbleHandles[ControllerIndex];
          if (rumble) {
            SDL_HapticRumblePlay(rumble, 0.5f, 2000);
          }
        }
      }

      RenderWeirdGradient(buf, XOffset, YOffset);
      SDLUpdateWindow(&buf, Window, Renderer);
    }
  }

  for (int i = 0; i < MAX_CONTROLLERS; i++) {
    if (ControllerHandles[i]) {
      SDL_GameControllerClose(ControllerHandles[i]);
    }
  }

  cleanup();
  return (0);
}
