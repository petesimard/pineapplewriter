#ifndef PUSHTOTALK_H
#define PUSHTOTALK_H
#include <thread>
#include <memory>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

#define Alt_L 0xffe9 /* Left alt */
#define Alt_R 0xffea /* Right alt */

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