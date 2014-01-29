#ifndef ABSTRACTTHREAD_H
#define ABSTRACTTHREAD_H

/** AbstractThread serves as abstract common base class for BasicThread and SocketThread. It's only purpose is to
 * define a common interface so both BasicThread and SocketThread can be used in the same way. */
class AbstractThread
{
  public:
    /// Creates a BasicThread object. The thread is not yet running. Call run() to start it.
    AbstractThread()           {}
    virtual ~AbstractThread()  {}

    /// Runs a BasicThread.
    virtual bool run() = 0;

    /// Tells the thread to stop. The thread will stop as soon as it sees the message.
    virtual void cancel() = 0;

    /// Call this after cancel() to wait that the thread has stopped.
    virtual void waitForExit() = 0;

    /** Every \a _uMillisecs milliseconds the virtual function handleTimeout() is called. The default
     * implementation does nothing. Using setTimeout() you can adjust the interval in which handleTimeout()
     * is executed.  */
    virtual void setTimeout(unsigned int _uMillisecs) = 0;

  protected:
    /** This is called if the timeout has passed and nothing happened. The default implementation is empty.
     * In order to do something, derive a class from BasicThread and reimplement handleTimeout(). */
    virtual void handleTimeout() {}
};

#endif


