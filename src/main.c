#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../updater/include/ac_utils.h"  
#include "../updater/include/x11_utils.h"  


void print_displays(DisplayInfo *displays, int count) {
  puts("Connected Displays:");
  for (int i=0; i<count; i++) {
    printf("%s: %dx%d @ %dHz\n", displays[i].name, displays[i].width, displays[i].height, (int)displays[i].refresh_rate);
  }
  puts("");
}

void print_modes(char *name) {
  // pull available modes for the display name
  int num_modes;
  DisplayMode *modes = get_available_modes(name, &num_modes);

  printf("Available Modes For: %s\n", name);
  for (int i=0; i<num_modes; i++) { 
    printf("%d) %dx%d @ %dHz\n", i, modes[i].width, modes[i].height, (int)modes[i].refresh_rate);
  }
}

void print_all_modes(DisplayInfo *displays, int count) {
  for (int i=0; i<count; i++) {
    print_modes(displays[i].name);
  }
}

void track_battery() {
  bool on_battery = on_bat();
  bool last_battery_status = !on_battery;
  
  while (true) {
    if (on_battery != last_battery_status) {
      if (on_battery) {
        puts("On Battery");
      } else {
        puts("Charging");
      }
    }
    last_battery_status = on_battery;
    on_battery = on_bat();
  } 
}


int main(void) {
  int display_count = 0; 
  DisplayInfo *connected_displays = get_all_displays(&display_count);
  
  print_displays(connected_displays, display_count);
  print_all_modes(connected_displays, display_count);
  
  track_battery();

  return 0;
}
