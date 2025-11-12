#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <upower.h>


bool on_bat() {
  UpClient *client = up_client_new();

  if (client == NULL) {
    fprintf(stderr, "Failed to connect to upower service.\n");
    return false;
  }
  bool status = up_client_get_on_battery(client);
  return status;
}


