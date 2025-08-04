
#include "pushtotalk.h"
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <chrono>

PushToTalk::PushToTalk()
{
    Display *display = XOpenDisplay(nullptr);
    if (!display)
    {
        qWarning() << "Cannot open X display";
        return;
    }

    m_display = display;
    Window root = DefaultRootWindow(display);
    m_root = (void *)root;

    // Create the thread using unique_ptr
    listenThread = std::make_unique<std::thread>(&PushToTalk::checkKeyPress, this);
}

PushToTalk::~PushToTalk()
{
    // Signal the thread to stop
    stopThread = true;

    // Wait for the thread to finish if it exists
    if (listenThread && listenThread->joinable())
    {
        listenThread->join();
    }

    if (m_display)
    {
        Display *display = (Display *)m_display;
        Window root = (Window)m_root;
        XUngrabKey(display, AnyKey, AnyModifier, root);
        XCloseDisplay(display);
    }
}

void PushToTalk::setCodeCode(int keyCode)
{
    if (m_keyCode != 0)
        XUngrabKey((Display *)m_display, m_keyCode, AnyModifier, (Window)m_root);

    m_keyCode = keyCode;

    KeyCode keycode = XKeysymToKeycode((Display *)m_display, m_keyCode);

    // Grab the key globally
    XGrabKey((Display *)m_display, keycode, AnyModifier, (Window)m_root, True,
             GrabModeAsync, GrabModeAsync);
    XSelectInput((Display *)m_display, (Window)m_root, KeyPressMask | KeyReleaseMask);

    m_isActive = false;
}

void PushToTalk::checkKeyPress()
{
    if (!m_display)
        return;

    Display *display = (Display *)m_display;
    bool wasPressed = false;

    while (!stopThread)
    {
        while (XPending(display))
        {
            XEvent ev;
            XNextEvent(display, &ev);
            if (ev.type == KeyPress || ev.type == KeyRelease)
            {
                XKeyEvent *key = (XKeyEvent *)&ev;
                KeySym keysym = XLookupKeysym(key, 0);
                if (keysym == m_keyCode)
                {
                    if (ev.type == KeyPress && !wasPressed)
                    {
                        wasPressed = true;
                        m_isActive = true;
                    }
                    else if (ev.type == KeyRelease && wasPressed)
                    {
                        wasPressed = false;
                        m_isActive = false;
                    }
                }
            }
        }

        // Sleep 10 ms to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
