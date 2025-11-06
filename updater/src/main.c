#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

double calculate_refresh_rate(XRRModeInfo *mode_info) {
    if (mode_info->hTotal && mode_info->vTotal) {
        return ((double)mode_info->dotClock) / 
               ((double)mode_info->hTotal * (double)mode_info->vTotal);
    }
    return 0.0;
}

int main() {
    Display *display;
    Window root;
    XRRScreenResources *res;
    XRROutputInfo *output_info = NULL;
    XRRCrtcInfo *crtc_info;
    XRRModeInfo *mode_info;
    RROutput target_output = None;

    display = XOpenDisplay(NULL);
    if (display == NULL) return 1;
    root = RootWindow(display, DefaultScreen(display));
    res = XRRGetScreenResources(display, root);
    if (res == NULL) { XCloseDisplay(display); return 1; }

    for (int i = 0; i < res->noutput; i++) {
        output_info = XRRGetOutputInfo(display, res, res->outputs[i]);
        if (output_info == NULL || output_info->connection != RR_Connected) {
            if (output_info) XRRFreeOutputInfo(output_info);
            continue;
        }
        if (strncmp(output_info->name, "eDP", 3) == 0 || 
            strncmp(output_info->name, "LVDS", 4) == 0) {
            target_output = res->outputs[i];
            break; 
        }
        XRRFreeOutputInfo(output_info); 
        output_info = NULL;
    }

    if (target_output == None) {
        XRRFreeScreenResources(res);
        XCloseDisplay(display);
        return 1;
    }

    if (output_info->crtc == None) {
        XRRFreeOutputInfo(output_info);
        XRRFreeScreenResources(res);
        XCloseDisplay(display);
        return 1;
    }
    crtc_info = XRRGetCrtcInfo(display, res, output_info->crtc);
    if (crtc_info == NULL) {
        XRRFreeOutputInfo(output_info);
        XRRFreeScreenResources(res);
        XCloseDisplay(display);
        return 1;
    }

    mode_info = NULL;
    for (int i = 0; i < res->nmode; i++) {
        if (res->modes[i].id == crtc_info->mode) {
            mode_info = &res->modes[i];
            break;
        }
    }

    if (mode_info != NULL) {
        double rate = calculate_refresh_rate(mode_info);
        printf("%.2f", rate); // Only print the number
    }

    XRRFreeCrtcInfo(crtc_info);
    XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(res);
    XCloseDisplay(display);
    return 0;
}
