#ifndef DISPLAY_INFO_H
#define DISPLAY_INFO_H

/**
 * @struct DisplayMode
 * @brief Holds information about a single available display mode.
 */
typedef struct {
    int width;
    int height;
    float refresh_rate;
} DisplayMode;

/**
 * @struct DisplayInfo
 * @brief Holds information about a currently active display.
 */
typedef struct {
    char* name;           // e.g., "DP-1", "HDMI-1"
    int width;            // Current resolution width
    int height;           // Current resolution height
    float refresh_rate;   // Current refresh rate
} DisplayInfo;

/**
 * @brief Retrieves information for all currently connected displays.
 *
 * @param[out] count Pointer to an integer that will be set to the number
 * of displays found.
 * @return A dynamically allocated array of DisplayInfo structs. The caller is
 * responsible for freeing this array and the 'name' field of each
 * element, or by using free_display_info().
 */
DisplayInfo* get_all_displays(int* count);

/**
 * @brief Frees the memory allocated by get_all_displays().
 *
 * @param displays The array of DisplayInfo structs.
 * @param count The number of elements in the array.
 */
void free_display_info(DisplayInfo* displays, int count);

/**
 * @brief Retrieves all available modes (resolution/refresh rate combos) for a
 * specific display.
 *
 * @param[in] display_name The name of the display (e.g., "DP-1").
 * @param[out] mode_count Pointer to an integer that will be set to the number
 * of modes found.
 * @return A dynamically allocated array of DisplayMode structs. The caller is
 * responsible for freeing this array using free_display_modes().
 */
DisplayMode* get_available_modes(const char* display_name, int* mode_count);

/**
 * @brief Frees the memory allocated by get_available_modes().
 *
 * @param modes The array of DisplayMode structs.
 */
void free_display_modes(DisplayMode* modes);

/**
 * @brief Sets the resolution and refresh rate for a specific display.
 *
 * This function attempts to find an existing mode that matches the parameters
 * and applies it.
 *
 * @param[in] display_name The name of the display to configure (e.g., "DP-1").
 * @param[in] width The desired width.
 * @param[in] height The desired height.
 * @param[in] refresh_rate The desired refresh rate.
 * @return 0 on success, -1 if the display is not found, -2 if the mode is
 * not found, and -3 on other X11/Xrandr errors.
 */
int set_display_mode(const char* display_name, int width, int height, float refresh_rate);

/**
 * @brief Sets a display to its "preferred" mode.
 *
 * This is typically the highest resolution and refresh rate the display supports.
 *
 * @param[in] display_name The name of the display to configure (e.g., "DP-1").
 * @return 0 on success, -1 if the display is not found, -2 if there is no
 * preferred mode, and -3 on other X11/Xrandr errors.
 */
int set_display_mode_auto(const char* display_name);

#endif // DISPLAY_INFO_H
