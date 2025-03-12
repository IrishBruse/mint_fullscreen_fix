#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

/* Helper function to get an Atom from its name */
Atom get_atom(Display * display,
    const char * name) {
    return XInternAtom(display, name, False);
}

/* Get the active window using the _NET_ACTIVE_WINDOW property */
Window get_active_window(Display * display) {
    Atom net_active_window = get_atom(display, "_NET_ACTIVE_WINDOW");
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char * prop = NULL;
    Window active_window = 0;

    if (XGetWindowProperty(display, DefaultRootWindow(display), net_active_window,
            0, sizeof(Window) / 4, False, XA_WINDOW, &
            actual_type, & actual_format, & nitems, & bytes_after, &
            prop) == Success && prop) {
        active_window = * (Window * ) prop;
        XFree(prop);
    }
    return active_window;
}

/* Check if a window is fullscreen by looking for the _NET_WM_STATE_FULLSCREEN atom */
int is_fullscreen(Display * display, Window window) {
    Atom net_wm_state = get_atom(display, "_NET_WM_STATE");
    Atom net_wm_state_fullscreen = get_atom(display, "_NET_WM_STATE_FULLSCREEN");
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char * prop = NULL;
    int fullscreen = 0;

    if (XGetWindowProperty(display, window, net_wm_state, 0, 1024, False,
            XA_ATOM, & actual_type, & actual_format, &
            nitems, & bytes_after, & prop) == Success && prop) {
        Atom * atoms = (Atom * ) prop;
        for (unsigned long i = 0; i < nitems; i++) {
            if (atoms[i] == net_wm_state_fullscreen) {
                fullscreen = 1;
                break;
            }
        }
        XFree(prop);
    }
    return fullscreen;
}

typedef struct {
    int x, y;
    int width, height;
}
Geometry;

/* Get the monitor geometry under the pointer using XRandr */
Geometry get_monitor_geometry(Display * display, Window root, int pointer_x, int pointer_y) {
    Geometry geo = {
        0,
        0,
        0,
        0
    };
    XRRScreenResources * screen = XRRGetScreenResources(display, root);
    if (!screen)
        return geo;

    for (int i = 0; i < screen -> ncrtc; i++) {
        XRRCrtcInfo * crtc = XRRGetCrtcInfo(display, screen, screen -> crtcs[i]);
        if (crtc && crtc -> width && crtc -> height) {
            /* Check if pointer is within this monitor (CRTC) */
            if (pointer_x >= crtc -> x && pointer_x < crtc -> x + crtc -> width &&
                pointer_y >= crtc -> y && pointer_y < crtc -> y + crtc -> height) {
                geo.x = crtc -> x;
                geo.y = crtc -> y;
                geo.width = crtc -> width;
                geo.height = crtc -> height;
                XRRFreeCrtcInfo(crtc);
                break;
            }
            XRRFreeCrtcInfo(crtc);
        }
    }
    XRRFreeScreenResources(screen);
    return geo;
}

/* Force the given window to cover the monitor where the pointer is located */
void force_fullscreen_on_cursor(Display * display, Window win) {
    Window root = DefaultRootWindow(display);
    Window ret_root, ret_child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    if (XQueryPointer(display, root, & ret_root, & ret_child, &
            root_x, & root_y, & win_x, & win_y, & mask)) {
        Geometry geo = get_monitor_geometry(display, root, root_x, root_y);
        if (geo.width && geo.height) {
            printf("Forcing window %lu to fullscreen on monitor at (%d, %d) size %dx%d\n",
                win, geo.x, geo.y, geo.width, geo.height);
            XMoveResizeWindow(display, win, geo.x, geo.y, geo.width, geo.height);
            XFlush(display);
        } else {
            printf("No monitor geometry found for pointer at (%d, %d)\n", root_x, root_y);
        }
    } else {
        printf("Unable to query pointer position.\n");
    }
}

int main() {
    Display * display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    Window root = DefaultRootWindow(display);
    Atom net_active_window = get_atom(display, "_NET_ACTIVE_WINDOW");

    /* Listen for property change events on the root window */
    XSelectInput(display, root, PropertyChangeMask);
    printf("Listening for fullscreen events...\n");

    Window last_window = 0;
    int last_fullscreen = 0;

    while (1) {
        XEvent event;
        XNextEvent(display, & event);

        /* Detect when the active window changes */
        if (event.type == PropertyNotify && event.xproperty.atom == net_active_window) {
            Window active_window = get_active_window(display);
            if (active_window != last_window) {
                last_window = active_window;
                last_fullscreen = is_fullscreen(display, active_window); // Initialize with current fullscreen state
            }
        }

        /* Check for changes in the fullscreen state */
        if (last_window && is_fullscreen(display, last_window) != last_fullscreen) {
            last_fullscreen = !last_fullscreen;
            if (last_fullscreen) {
                printf("Window %lu is now FULLSCREEN\n", last_window);
                /* Intercept the fullscreen event and force it to the monitor under the cursor */
                force_fullscreen_on_cursor(display, last_window);
            } else {
                printf("Window %lu is now NORMAL\n", last_window);
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}
