#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#define PWR_FILE "/sys/class/power_supply/AC0/online"



int main(void) {
  FILE *pwr_file;

  if ((pwr_file = fopen(PWR_FILE, "r")) == NULL) {
    printf("Error reading file: %s\n", PWR_FILE);
    exit(1);
  }
  
  char stat = 'z'; // arbitrary char that is not 0 or 1
  fscanf(pwr_file, " %c ", &stat);

  if (stat == 'z') {
    printf("File was opened but encountered an error during reading.\n");
    exit(1);
  }
  
  // parse the status char into a boolean
  bool status = (bool) atoi(&stat);
  if (status){ puts("Charging"); }
  else{ puts("On-Battery"); }

  // ------------- X11 interaction -------------
  
  Display *dpy = XOpenDisplay(NULL);

  if (!dpy) {
    puts("Could not open display object.");
    exit(1);
  }

  Window root = DefaultRootWindow(dpy);

  XRRScreenResources *screen_res = XRRGetScreenResources(dpy, root);


  return 0;
}
