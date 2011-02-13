#include "linux/LinuxSemaphore.h"
#include "AbstractionTypes.h"
#include "AbstractionFunctions.h"
#include "AbstractionException.h"
#include "eventfd.h"

LinuxSemaphore::LinuxSemaphore(uint32_t maxCount GCC_UNUSED,
                               uint32_t initialCount) :
  WaitObject(eventfd(initialCount, (EFD_NONBLOCK | EFD_SEMAPHORE | EFD_WAITREAD)))
{
  if(getHandle() == INVALID_HANDLE_VALUE)
    throw std::bad_syscall("eventfd", lastError());
}

LinuxSemaphore::~LinuxSemaphore()
{
  close(getHandle());
}

void LinuxSemaphore::lock(uint32_t timeout)
{
  if(WaitForObject(*this, timeout) != WaitSuccess)
    throw std::runtime_error("failed to wait for semaphore");
}

void LinuxSemaphore::unlock(uint32_t count)
{
  uint64_t internalCount(count);

  if(write(getHandle(), &internalCount, sizeof(internalCount)) != sizeof(internalCount))
    throw std::bad_syscall("write to eventfd", lastError());
}
