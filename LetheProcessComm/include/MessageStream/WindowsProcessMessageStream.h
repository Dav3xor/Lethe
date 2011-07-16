#ifndef _WINDOWSPROCESSMESSAGESTREAM_H
#define _WINDOWSPROCESSMESSAGESTREAM_H

#include "Lethe.h"

namespace lethe
{
  class ProcessMessageHeader;

  class WindowsProcessMessageStream : public MessageStream
  {
  public:
    WindowsProcessMessageStream(uint32_t remoteProcessId, uint32_t outgoingSize);
    ~WindowsProcessMessageStream();

    operator WaitObject&();
    Handle getHandle() const;

    void* allocate(uint32_t size);
    void send(void* buffer);
    void* receive();
    void release(void* buffer);

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsProcessMessageStream(const WindowsProcessMessageStream&);
    WindowsProcessMessageStream& operator = (const WindowsProcessMessageStream&);

    Semaphore* m_semaphoreIn;
    Semaphore* m_semaphoreOut;

    SharedMemory* m_shmIn;
    SharedMemory* m_shmOut;

    ProcessMessageHeader* m_headerIn;
    ProcessMessageHeader* m_headerOut;
  };
}

#endif