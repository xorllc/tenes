#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include "nespal.h"
#include "nes.h"
#include "config.h"

void sys_framesync (void)
{
    struct timeval tv;
    long long start = time_frame_start.tv_sec * 1000000 + time_frame_start.tv_usec;
    long long target = start  + (1000000 / 60);
    long long now;

    do {
        if (gettimeofday(&tv, 0)) return;
        now = tv.tv_sec * 1000000 + tv.tv_usec;
        if (target-now > 10000) usleep(5000);
    } while (now < target);
    
    // if (now-target) printf("framesync: off by %lli\n", now - target);
}

SDL_Color palette[128];
SDL_Joystick *joystick[4];
int numsticks = 0;

void sys_init (void)
{
  int i, tmp;
  if (SDL_Init (SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
    printf ("Could not initialize SDL!\n");
    exit (1);
  }
  printf ("SDL initialized.\n");

  if (vid_filter!=NULL) init_scaling();

  window_width=max(window_width,vid_width);
  window_height=max(window_height,vid_height);

  printf ("Initializing video... \n");
  window_surface = SDL_SetVideoMode (window_width, window_height, 8, SDL_DOUBLEBUF | SDL_SWSURFACE | SDL_HWPALETTE | (vid_fullscreen ? SDL_FULLSCREEN : 0));
  if (window_surface == NULL) {
    printf ("Could not set desired display mode!\n");
    exit (1);
  }
  
  if (surface==NULL) surface=window_surface;
  if (post_surface==NULL) post_surface=window_surface;

  SDL_WM_SetCaption (nes.rom.title, nes.rom.title);
  printf ("SDL video window created.\n");

  for (i = 0; i < 128; i++) {
    palette[i].r = nes_palette[(i&63) * 3 + 0];
    palette[i].g = nes_palette[(i&63) * 3 + 1];
    palette[i].b = nes_palette[(i&63) * 3 + 2];
    palette[i].unused = 0;
  }
  SDL_SetColors(window_surface, palette, 0, 128);
  SDL_SetColors(post_surface, palette, 0, 128);  
  SDL_SetColors(surface, palette, 0, 128);  
  SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0, 0, 0));
  SDL_FillRect(post_surface, NULL, SDL_MapRGB(window_surface->format, 0, 0, 0));
  SDL_FillRect(surface, NULL, SDL_MapRGB(window_surface->format, 0, 0, 0));
  SDL_Flip (window_surface);

  tmp = SDL_NumJoysticks();
  if (!cfg_disable_joysticks) {
    printf ("Found %i joystick(s).\n", tmp);
    //  for (i=0; i<tmp; i++) printf ("  %i: %s\n",i,SDL_JoystickName(i));
    if (tmp) {
      numsticks = (tmp>4)?4:tmp;
      for (i=0; i<numsticks; i++) {
        joystick[i] = SDL_JoystickOpen (i);
        if (!joystick[i]) printf ("Could not open joystick %i \n", i);
        else {
          int j, js_mapping=-1;
          for (j=0; j<4; j++) 
            {
              if (cfg_jsmap[j]==i)
                {
                  js_mapping=j;
                  break;
                }
            }
          printf ("  %i: %s (%i buttons)   ", i, SDL_JoystickName(i), SDL_JoystickNumButtons(joystick[i]));
          if (js_mapping==-1) printf("[unmapped]\n");
          else printf("[joypad %i]\n", js_mapping);
        }      
      }
      SDL_JoystickEventState (SDL_ENABLE);
    }
  } else printf("Joysticks are disabled.\n");
}

void sys_shutdown (void)
{
  int i;
  
  for (i=0; i<numsticks; i++) {
    if (joystick[i]) SDL_JoystickClose (joystick[i]);
  }

  SDL_SetTimer (0, NULL);
  SDL_FreeSurface (window_surface);
  SDL_Quit (); 
}
