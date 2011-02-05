#ifndef _LINUXMUTEX_H
#define _LINUXMUTEX_H

#include "WaitObject.h"
#include "AbstractionTypes.h"
#include <pthread.h>

/*
 * The LinuxMutex class is extremely similar to LinuxSemaphore, being a
 *  semaphore with a maximum value of 1.
 *
 */
class LinuxMutex : public WaitObject
{
public:
  LinuxMutex(bool locked = false);
  ~LinuxMutex();

  void lock(uint32_t timeout = INFINITE);
  void unlock();

protected:
  void postWaitCallback(WaitResult result);

private:
  pthread_t m_ownerThread;
};

#endif
