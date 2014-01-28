#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <list>

#include <pthread.h>

#include "AbstractThread.h"

/** SocketThread implements the AbstractThread interface and can be used the same way as BasicThread.
 * While the mainloop of BasicThread is waiting on a pthread condition variable, the mainloop of SocketThread
 * is waiting on a set of file descriptors, typically sockets. This makes it suitable for threads which
 * have to react to IO via sockets, pipes or similar things.
 */
class SocketThread: public AbstractThread
{
  public:
    enum
    {
      DO_NOTHING = 0,
      DO_CANCEL_THREAD = 1,
      DO_ERROR_CANCEL_THREAD = 2
    };

    SocketThread();
    ~SocketThread();

    virtual bool run();

    /// Tells the thread to cancel.
    virtual void cancel();

    virtual void waitForExit();

    virtual void setTimeout(unsigned int _uMillisecs);

    /** A freshly constructed SocketThread object is waiting only on its internal pipe file descriptor. This alone
     * is not very useful. To make a SocketThread actually wait on some real sockets, pipes, etc., call
     * addFd() with the respective file descriptor as argument. Then the mainloop will also wait for
     * read/close events on this file descriptor. If the file descriptor becomes readable, the virtual
     * function receiveData(_fd) is called, which you have to reimplement and handle the reading there.
     */
    void addFd(int _fd);

    void removeFd(int _fd);
  protected:
    int postCommandByte(const char _cmd);

    /** This function is called when one of the file descriptors added via addFd() becomes readable. You have to
     * reimplement this function to handle your file descriptors. If you read 0 byte, then the
     * file descriptor/socket/pipe has been closed on the other side and you have to return false.
     * In all other cases return true. */
    virtual bool receiveData(int _fd);

    /// mainloop implementation for the SocketThread. You don't have to modify it in general.
    virtual void mainloop();

    virtual void processWork(char _cmd)
    {
    }

    /// Helper function for starting the thread.
    static void* mainloopHelper(void* _pArg);

    int m_commandPipe[2];
    std::list<int> m_fds;

    pthread_t m_thread;
    pthread_mutex_t m_mutex;
    bool m_bThreadStarted;
    unsigned int m_uTimeoutMillisecs;
};

#endif

