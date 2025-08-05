#ifndef PUSHTOTALK_H
#define PUSHTOTALK_H
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

#define Shift_L 0xffe1    /* Left shift */
#define Shift_R 0xffe2    /* Right shift */
#define Control_L 0xffe3  /* Left control */
#define Control_R 0xffe4  /* Right control */
#define Caps_Lock 0xffe5  /* Caps lock */
#define Shift_Lock 0xffe6 /* Shift lock */

#define Meta_L 0xffe7  /* Left meta */
#define Meta_R 0xffe8  /* Right meta */
#define Alt_L 0xffe9   /* Left alt */
#define Alt_R 0xffea   /* Right alt */
#define Super_L 0xffeb /* Left super */
#define Super_R 0xffec /* Right super */
#define Hyper_L 0xffed /* Left hyper */
#define Hyper_R 0xffee /* Right hyper */

class PushToTalk
{

public:
    PushToTalk();
    ~PushToTalk();

    bool m_isActive = false;

    void setCodeCode(int keyCode);

private:
    int m_keyCode = 0;
    std::unique_ptr<std::thread> listenThread;
    std::atomic<bool> stopThread{false};

    void *m_display; // X11 Display pointer
    void *m_root;    // X11 Window pointer
    void checkKeyPress();
};

#endif // PUSHTOTALK_H