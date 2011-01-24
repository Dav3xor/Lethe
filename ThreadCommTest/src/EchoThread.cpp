#include "EchoThread.h"
#include "Exception.h"
#include "Log.h"

EchoThread::EchoThread(ThreadComm::Channel& channel) :
  m_channel(channel),
  m_chanHandle(m_channel.getHandle()),
  m_iterationCount(0),
  m_repliesToSend(0)
{
  LogInfo("Echo thread handle: " << (uint32_t)getHandle());
  addWaitObject(m_channel.getHandle());
  setWaitTimeout(200);
}

EchoThread::~EchoThread()
{
  LogInfo("Echo thread performed " << m_iterationCount << " iterations");
}

void EchoThread::iterate(Handle handle)
{
  if(handle == m_channel.getHandle())
    receiveMessage();
  if(handle == INVALID_HANDLE_VALUE)
    LogInfo("Timeout");

  sendReplies();
  ++m_iterationCount;
}

void EchoThread::receiveMessage()
{
  // Receive a message
  uint32_t* msg = reinterpret_cast<uint32_t*>(m_channel.receive());
  if(msg[0] != 1) throw Exception("Incorrect message data");
  m_channel.release(msg);
  ++m_repliesToSend;
}

void EchoThread::sendReplies()
{
  try
  {
    uint32_t* msg;

    while(m_repliesToSend > 0)
    {
      msg = reinterpret_cast<uint32_t*>(m_channel.allocate(sizeof(uint32_t)));
      msg[0] = 2;
      m_channel.send(msg);
      --m_repliesToSend;
    }
  }
  catch(Exception& ex)
  {
    if(ex.what() != "Semaphore full, other side needs to wait on it") throw;
  }
}

void EchoThread::abandoned(Handle handle)
{
  if(handle == m_channel.getHandle())
    throw Exception("Problem with the channel's handle");
  else
    LogError("Unrecognized handle abandoned");
}