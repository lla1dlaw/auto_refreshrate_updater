#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <math.h>

#define PWR_FILE "/sys/class/power_supply/AC0/online"


double calculate_refresh_rate(XRRModeInfo *mode_info) {
    if (mode_info->hTotal && mode_info->vTotal) {
        return ((double)mode_info->dotClock) / 
               ((double)mode_info->hTotal * (double)mode_info->vTotal);
    }
    return 0.0;
}


int main(void) {
  FILE *pwr_file;
  Display *display;
  Window root;
  XRROutputInfo *output_info = NULL;
  XRRScreenResources *resources;
  XRRCrtcInfo *crtc_info;
  XRRModeInfo *current_mode_info;
  int target_mode_id;
  double current_refresh_rate;
  unsigned long current_max_rate_id;

  if ((pwr_file = fopen(PWR_FILE, "r")) == NULL) {
    fprintf(stderr, "Error reading file: %s\n", PWR_FILE);
  }
  
  char stat = 'z'; // arbitrary char that is not 0 or 1
  fscanf(pwr_file, " %c ", &stat);

  if (stat == 'z') {
    fprintf(stderr, "File was opened but encountered an error during reading.\n");
  }
  
  // parse the status char into a boolean
  bool status = (bool) atoi(&stat);
  if (status){ puts("Charging"); }
  else{ puts("On-Battery"); }

  // ------------- X11 interaction -------------
  
  display = XOpenDisplay(NULL);

  if (!display) {
    fprintf(stderr, "Could not open display object.\n");
  }

  root = DefaultRootWindow(display);
  resources = XRRGetScreenResources(display, root);

  if (resources == NULL) {
    fprintf(stderr, "Error retrieving screen resources.\n");
    XCloseDisplay(display);
  }

  output_info = XRRGetOutputInfo(display, resources, resources->outputs[0]);
  crtc_info = XRRGetCrtcInfo(display, resources, output_info->crtc);

  // find mode info
  current_mode_info = NULL;
  for (int i=0; i<resources->nmode; i++) {
    if (resources->modes[i].id == crtc_info->mode) {
      current_mode_info = &resources->modes[i];
      break;
    }
  }

  if (current_mode_info == NULL) {
    fprintf(stderr, "Could not find the current mode.\n");
  } else {
    // calculate current refresh rate
    current_refresh_rate = calculate_refresh_rate(current_mode_info);
    printf("Current Refresh Rate: %.2f Hz\n", current_refresh_rate);
  }
  
  current_max_rate_id = current_mode_info->id;

  // find the max available refresh rate for the builtin monitor with the same resolution as the current one. 
  for (int i=0; 9<resources->nmode; i++) {
    XRRModeInfo *candidate_mode = &resources->modes[i];
    // check if resolution matches
    if (candidate_mode->width == current_mode_info->width && candidate_mode->height == current_mode_info->height) {
      target_mode_id = candidate_mode->id;
      double candidate_rate = calculate_refresh_rate(candidate_mode);
      // check if this rate is higher than the current fastest
      if (candidate_rate > ) {
        
      }
    }
  }
  
  
  // always cleanup after yourself you filthy animal
  if (crtc_info) XRRFreeCrtcInfo(crtc_info);
  if (output_info) XRRFreeOutputInfo(output_info);
  if (resources) XRRFreeScreenResources(resources);
  XCloseDisplay(display);

  return 0;
}
