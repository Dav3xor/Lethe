#ifndef _LINUXTHREAD_H
#define _LINUXTHREAD_H

#include "BaseThread.h"
#include <pthread.h>
#include <set>

/*
 * The LinuxThread class is meant to be used as a base class for multi-threading.
 *  The user should derive from this and implement the iterate (and possibly
 *  abandon) functions.
 *
 * iterate() is called every time the thread loops.  The thread only begins looping
 *  after start() has been called (this may be done in the derived class's constructor
 *  to always begin immediately).  If pause() is called, the thread may be restarted
 *  with start().  At any point, the thread may be closed with stop(), and which point
 *  it cannot be restarted.
 *
 * If the thread is interrupted by an unhandled exception or encounters an internal
 *  error, the error string may be obtained through getError().
 *
 * Thread loop rate may be controlled by a few factors.  First, a timeout may be set
 *  through the constructor, which gives the base time between loops (in ms).  Secondly,
 *  the thread may provide wait objects to wake the thread up if one becomes triggered.
 *  These file descriptors (called handles here) are the same that can be obtained from
 *  the synchronization classes Mutex, Semaphore, Event, Timer, Pipe, and even other
 *  Threads.
 *
 * The thread handle triggers when the thread exits and does not reset.
 *
 * If the thread is destroyed while running, it will stop the thread and wait for it to
 *  exit before returning from the destructor.
 *
 * abandoned (optionally implemented in derived classes) is called when there is a
 *  problem with a user-provided handle.  Thread will not automatically remove this handle,
 *  but the user needs to fix or remove it, or abandoned will keep getting called.
 */
class LinuxThread : private BaseThread
{
public:
  LinuxThread(uint32_t timeout = 0);
  virtual ~LinuxThread();
   
  using BaseThread::start;
  using BaseThread::pause;
  using BaseThread::stop;
   
  using BaseThread::isStopping;
  using BaseThread::getHandle;
  using BaseThread::getError;
   
protected:
  using BaseThread::iterate;
  using BaseThread::abandoned;
  
  using BaseThread::addWaitObject;
  using BaseThread::removeWaitObject;
  using BaseThread::setWaitTimeout;

private:
  static void* threadHook(void*);

  pthread_t m_thread;      
};

#endif