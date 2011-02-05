#include "linux/LinuxMutex.h"
#include "AbstractionFunctions.h"
#include "Exception.h"
#include <pthread.h>
#include "eventfd.h"

LinuxMutex::LinuxMutex(bool locked) :
  WaitObject(eventfd((locked ? 0 : 1), (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD))),
  m_ownerThread(locked ? pthread_self() : -1)
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw Exception("Failed to create mutex: " + lastError());
}

LinuxMutex::~LinuxMutex()
{
  if(close(getHandle()) != 0)
    throw Exception("Failed to close mutex: " + lastError());
}

void LinuxMutex::lock(uint32_t timeout)
{
  if(m_ownerThread != pthread_self())
  {
    if(WaitForObject(getHandle(), timeout) != WaitSuccess)
      throw Exception("Failed to lock mutex: " + lastError());
    m_ownerThread = pthread_self();
  }
}

void LinuxMutex::unlock()
{
  if(m_ownerThread == pthread_self())
  {
    uint64_t count(1);

    if(write(getHandle(), &count, sizeof(count)) != sizeof(count))
      throw Exception("Failed to unlock mutex: " + lastError());
  }
  else
    throw Exception("Failed to unlock mutex: Attempt to release mutex not owned by caller. ");
}

void LinuxMutex::postWaitCallback(WaitResult result)
{
  if(result == WaitSuccess)
  {
    m_ownerThread = pthread_self();
  }
}
