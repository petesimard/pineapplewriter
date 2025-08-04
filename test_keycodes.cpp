#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

int main()
{
    Display *display = XOpenDisplay(nullptr);
    if (!display)
    {
        std::cerr << "Cannot open X display" << std::endl;
        return 1;
    }

    // Test different keysyms for Right Alt
    KeySym keysyms[] = {
        XK_ISO_Level3_Shift, // Right Alt (AltGr)
        XK_Alt_R,            // Right Alt
        XK_Alt_L,            // Left Alt
        XK_Alt               // Generic Alt
    };

    const char *names[] = {
        "XK_ISO_Level3_Shift",
        "XK_Alt_R",
        "XK_Alt_L",
        "XK_Alt"};

    for (int i = 0; i < 4; i++)
    {
        KeyCode keycode = XKeysymToKeycode(display, keysyms[i]);
        std::cout << names[i] << " -> keycode: " << keycode;
        if (keycode == 0)
        {
            std::cout << " (NOT FOUND)";
        }
        std::cout << std::endl;
    }

    // Test grabbing
    Window root = DefaultRootWindow(display);
    KeyCode keycode = XKeysymToKeycode(display, XK_ISO_Level3_Shift);
    if (keycode != 0)
    {
        int result = XGrabKey(display, keycode, AnyModifier, root, True,
                              GrabModeAsync, GrabModeAsync);
        std::cout << "XGrabKey result: " << result << std::endl;
        if (result == Success)
        {
            std::cout << "Successfully grabbed XK_ISO_Level3_Shift" << std::endl;
        }
        else
        {
            std::cout << "Failed to grab XK_ISO_Level3_Shift" << std::endl;
        }
    }

    XCloseDisplay(display);
    return 0;
}