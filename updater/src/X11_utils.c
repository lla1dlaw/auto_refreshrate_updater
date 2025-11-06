#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cJSON/cJSON.h"

typedef struct {
  int width;
  int height;
  double rate;
} ScreenInfo;


void print_screen_info() {
  Screen *screen;
  Display *display = XOpenDisplay(NULL);
  int num_screens;

  if (display == NULL) {
    fprintf(stderr, "Could not open display server connection.\n");
    exit(1);
  }
  num_screens = ScreenCount(display);
  
  for (int i=0; i<num_screens; i++) {
    screen = ScreenOfDisplay(display, i);
    printf("%d: (%dx%d) @ %f hz\n", i, screen->width, screen->height, screen->);
  }



}
