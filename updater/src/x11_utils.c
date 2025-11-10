#include "../include/x11_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

// Helper to calculate refresh rate from XRRModeInfo
static float calculate_refresh_rate(XRRModeInfo* mode) {
    if (!mode || mode->hTotal == 0 || mode->vTotal == 0) {
        return 0.0f;
    }
    return (float)mode->dotClock / ((float)mode->hTotal * (float)mode->vTotal);
}

DisplayInfo* get_all_displays(int* count) {
    *count = 0;
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Could not open X display\n");
        return NULL;
    }

    Window root = DefaultRootWindow(dpy);
    XRRScreenResources* res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) {
        fprintf(stderr, "Could not get screen resources\n");
        XCloseDisplay(dpy);
        return NULL;
    }

    DisplayInfo* displays = NULL;
    int connected_count = 0;

    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo* output_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
        if (!output_info) {
            continue;
        }

        if (output_info->connection == RR_Connected) {
            connected_count++;
            displays = realloc(displays, connected_count * sizeof(DisplayInfo));
            DisplayInfo* current_display = &displays[connected_count - 1];

            current_display->name = strdup(output_info->name);

            if (output_info->crtc) {
                XRRCrtcInfo* crtc_info = XRRGetCrtcInfo(dpy, res, output_info->crtc);
                if (crtc_info) {
                    current_display->width = crtc_info->width;
                    current_display->height = crtc_info->height;

                    // Find the mode info to calculate refresh rate
                    for (int j = 0; j < res->nmode; j++) {
                        if (res->modes[j].id == crtc_info->mode) {
                            current_display->refresh_rate = calculate_refresh_rate(&res->modes[j]);
                            break;
                        }
                    }
                    XRRFreeCrtcInfo(crtc_info);
                }
            } else {
                // Output is connected but not active (no CRTC)
                current_display->width = 0;
                current_display->height = 0;
                current_display->refresh_rate = 0.0f;
            }
        }
        XRRFreeOutputInfo(output_info);
    }

    *count = connected_count;
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return displays;
}

void free_display_info(DisplayInfo* displays, int count) {
    if (!displays) return;
    for (int i = 0; i < count; i++) {
        free(displays[i].name);
    }
    free(displays);
}

DisplayMode* get_available_modes(const char* display_name, int* mode_count) {
    *mode_count = 0;
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Could not open X display\n");
        return NULL;
    }

    Window root = DefaultRootWindow(dpy);
    XRRScreenResources* res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) {
        fprintf(stderr, "Could not get screen resources\n");
        XCloseDisplay(dpy);
        return NULL;
    }

    DisplayMode* modes = NULL;
    RROutput output_id = 0;

    // Find the output ID by name
    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo* output_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
        if (output_info && strcmp(output_info->name, display_name) == 0) {
            output_id = res->outputs[i];
            
            modes = malloc(output_info->nmode * sizeof(DisplayMode));
            *mode_count = output_info->nmode;

            for (int j = 0; j < output_info->nmode; j++) {
                // Get the mode ID from the output's list
                RRMode mode_id = output_info->modes[j];
                
                // Find the full XRRModeInfo from the resources
                for (int k = 0; k < res->nmode; k++) {
                    if (res->modes[k].id == mode_id) {
                        modes[j].width = res->modes[k].width;
                        modes[j].height = res->modes[k].height;
                        modes[j].refresh_rate = calculate_refresh_rate(&res->modes[k]);
                        break;
                    }
                }
            }
            XRRFreeOutputInfo(output_info);
            break; // Found our display
        }
        if (output_info) {
            XRRFreeOutputInfo(output_info);
        }
    }

    if (output_id == 0) {
        fprintf(stderr, "Could not find display: %s\n", display_name);
        free(modes);
        modes = NULL;
        *mode_count = 0;
    }

    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return modes;
}

void free_display_modes(DisplayMode* modes) {
    free(modes);
}

static int apply_mode(Display* dpy, XRRScreenResources* res, RROutput output_id, XRRCrtcInfo* crtc_info, RRMode mode_id) {
    Status status = XRRSetCrtcConfig(dpy,
                                     res,
                                     crtc_info->crtc,
                                     CurrentTime,
                                     crtc_info->x,
                                     crtc_info->y,
                                     mode_id,
                                     crtc_info->rotation,
                                     &output_id,
                                     1);
    
    return (status == RRSetConfigSuccess) ? 0 : -3;
}

static XRRCrtcInfo* get_crtc_for_output(Display* dpy, XRRScreenResources* res, XRROutputInfo* output_info) {
    if (output_info->crtc) {
        return XRRGetCrtcInfo(dpy, res, output_info->crtc);
    }
    // If no CRTC is assigned, try to find a free one
    for (int i = 0; i < res->ncrtc; i++) {
        XRRCrtcInfo* crtc = XRRGetCrtcInfo(dpy, res, res->crtcs[i]);
        if (crtc && crtc->mode == None) {
            // Found a free CRTC
            return crtc;
        }
        if (crtc) XRRFreeCrtcInfo(crtc);
    }
    return NULL;
}

int set_display_mode(const char* display_name, int width, int height, float refresh_rate) {
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return -3;

    Window root = DefaultRootWindow(dpy);
    XRRScreenResources* res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) {
        XCloseDisplay(dpy);
        return -3;
    }

    RROutput output_id = 0;
    XRROutputInfo* output_info = NULL;
    RRMode target_mode_id = 0;
    int result = -1; // -1 = display not found

    // 1. Find the output
    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo* temp_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
        if (temp_info && strcmp(temp_info->name, display_name) == 0) {
            output_id = res->outputs[i];
            output_info = temp_info;
            break;
        }
        if (temp_info) XRRFreeOutputInfo(temp_info);
    }

    if (!output_info) goto cleanup;

    // 2. Find the mode
    result = -2; // -2 = mode not found
    for (int i = 0; i < res->nmode; i++) {
        XRRModeInfo* mode = &res->modes[i];
        if (mode->width == width && mode->height == height) {
            float rr = calculate_refresh_rate(mode);
            // Use an epsilon for float comparison
            if (fabs(rr - refresh_rate) < 0.01) {
                target_mode_id = mode->id;
                break;
            }
        }
    }

    if (target_mode_id == 0) goto cleanup;

    // 3. Find the CRTC
    XRRCrtcInfo* crtc_info = get_crtc_for_output(dpy, res, output_info);
    if (!crtc_info) {
        result = -3; // No CRTC available
        goto cleanup;
    }

    // 4. Apply the mode
    result = apply_mode(dpy, res, output_id, crtc_info, target_mode_id);
    
    XRRFreeCrtcInfo(crtc_info);

cleanup:
    if (output_info) XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return result;
}

int set_display_mode_auto(const char* display_name) {
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy) return -3;

    Window root = DefaultRootWindow(dpy);
    XRRScreenResources* res = XRRGetScreenResourcesCurrent(dpy, root);
    if (!res) {
        XCloseDisplay(dpy);
        return -3;
    }

    RROutput output_id = 0;
    XRROutputInfo* output_info = NULL;
    RRMode preferred_mode_id = 0;
    int result = -1; // -1 = display not found

    // 1. Find the output
    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo* temp_info = XRRGetOutputInfo(dpy, res, res->outputs[i]);
        if (temp_info && strcmp(temp_info->name, display_name) == 0) {
            output_id = res->outputs[i];
            output_info = temp_info;
            break;
        }
        if (temp_info) XRRFreeOutputInfo(temp_info);
    }

    if (!output_info) goto cleanup;

    // 2. Find the preferred mode
    result = -2; // -2 = no preferred mode
    if (output_info->npreferred > 0) {
        preferred_mode_id = output_info->modes[0]; // First mode is preferred
    }

    if (preferred_mode_id == 0) goto cleanup;

    // 3. Find the CRTC
    XRRCrtcInfo* crtc_info = get_crtc_for_output(dpy, res, output_info);
    if (!crtc_info) {
        result = -3; // No CRTC available
        goto cleanup;
    }

    // 4. Apply the mode
    result = apply_mode(dpy, res, output_id, crtc_info, preferred_mode_id);

    XRRFreeCrtcInfo(crtc_info);

cleanup:
    if (output_info) XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(res);
    XCloseDisplay(dpy);
    return result;
}
