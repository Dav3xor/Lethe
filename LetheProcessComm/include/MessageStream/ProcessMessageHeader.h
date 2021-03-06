#ifndef _PROCESSMESSAGEHEADER_H
#define _PROCESSMESSAGEHEADER_H

#include "MessageStream/ProcessMessageList.h"
#include "MessageStream/ProcessMessageUnallocList.h"
#include "MessageStream/ProcessMessageReceiveList.h"
#include "MessageStream/ProcessMessage.h"
#include "Lethe.h"

namespace lethe
{
  class ProcessMessageHeader
  {
  public:
    ProcessMessageHeader(uint32_t size);
    ~ProcessMessageHeader();

    ProcessMessage& allocate(uint32_t size);
    void send(ProcessMessage* message);
    ProcessMessage* receive();
    bool release(ProcessMessage* message);

    uint32_t getSize() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    ProcessMessageHeader(const ProcessMessageHeader&);
    ProcessMessageHeader& operator = (const ProcessMessageHeader&);

    static const uint32_t s_firstBufferOffset;
    static const uint32_t s_secondBufferOffset;
    static const uint32_t s_thirdBufferOffset;

    uint32_t m_size;
    ProcessMessageReceiveList m_receiveList;
    ProcessMessageList m_releaseList;
    ProcessMessageUnallocList m_unallocList;
  };
}

#endif
