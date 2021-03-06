#include "BaseThread.h"
#include "LetheFunctions.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include <sstream>

using namespace lethe;

BaseThread::BaseThread(uint32_t timeout) :
  WaitObject(INVALID_HANDLE_VALUE),
  m_running(false),
  m_exit(false),
  m_triggerEvent(false, true),
  m_stoppedEvent(true, false),
  m_exitedEvent(false, false),
  m_mutex(false),
  m_timeout(timeout)
{
  m_waitSet.add(m_triggerEvent);
  setHandle(m_stoppedEvent.getHandle());
}

BaseThread::~BaseThread()
{
  m_exit = true;

  try
  {
    stop();
    WaitForObject(m_exitedEvent, INFINITE);
  }
  catch(std::bad_syscall&)
  {
    // If the stop fails because the event is not valid for some reason, try more drastic measures
    // TODO: more drastic measures
    // Linux: pthread_cancel, pthread_kill?
    // Windows:
  }
}

void BaseThread::threadMain()
{
  bool initialized = false;
  Handle handle;

  try
  {
    while(!m_exit)
    {
      WaitForObject(m_triggerEvent, INFINITE);

      if(!initialized)
      {
        setup();
        initialized = true;
      }

      if(m_exit)
        break;

      if(m_running) // protection against stopping before actually starting
      {
        do
        {
          if(m_objectQueue.size() > 0)
            handleObjectQueue();

          switch(m_waitSet.waitAny(m_timeout, handle))
          {
          case WaitSuccess:
            if(handle == m_triggerEvent.getHandle())
              break;

          case WaitTimeout: // Fallthrough intentional
            iterate(handle);
            break;

          case WaitAbandoned:
            abandoned(handle);
            break;

          case WaitError:
            if(handle == m_triggerEvent.getHandle())
              throw std::logic_error("thread trigger event error");

            error(handle);
            break;

          default:
            throw std::logic_error("thread internal wait failed");
          }
        } while(m_running);

        m_stoppedEvent.set();
      }
    }
  }
  catch(std::exception& ex)
  {
    try
    {
      m_mutex.lock();
      m_error.assign(ex.what());

      if(m_error.empty()) // Don't allow an empty error string
        m_error.assign("empty exception text");

      m_mutex.unlock();
    }
    catch(std::runtime_error&)
    {
      // Problem with the mutex, possibly destroyed, abandon the operation
    }
  }
  catch(...)
  {
    m_mutex.lock();
    m_error.assign("Unrecognized exception");
    m_mutex.unlock();
  }

  // Make sure all the notifications are set
  m_running = false;
  m_exit = true;

  try
  {
    m_stoppedEvent.set();
  }
  catch(std::bad_syscall&)
  {
    // Thread may already be destroyed, ignore
  }

  try
  {
    m_exitedEvent.set();
  }
  catch(std::bad_syscall&)
  {
    // Thread may already be destroyed, ignore
  }
}

void BaseThread::setup()
{
  // Do nothing
}

// This function should never be called.  A user derived class should implement
//  iterate.  The reason this function is not left pure-virtual is so that during
//  destruction of a Thread object, a call to iterate from within ThreadMain does
//  not call the derived class's iterate after the derived destructor has completed
//  but before the base destructor has completed.
void BaseThread::iterate(Handle handle GCC_UNUSED)
{
  throw std::runtime_error("iterate called in BaseThread");
}

void BaseThread::start()
{
  // Check if there is a problem with the thread
  m_mutex.lock();
  bool errored = !m_error.empty();
  m_mutex.unlock();

  if(errored) throw std::runtime_error("thread exited with exception: " + m_error);
  if(m_exit) throw std::logic_error("thread is being destructed and cannot start");

  m_running = true;
  m_stoppedEvent.reset();
  m_triggerEvent.set();
}

void BaseThread::stop()
{
  m_running = false;
  m_triggerEvent.set();
}

bool BaseThread::isStopping() const
{
  return !m_running;
}

std::string BaseThread::getError()
{
  m_mutex.lock();
  std::string error(m_error);
  m_mutex.unlock();

  return error;
}

void BaseThread::addWaitObject(WaitObject& obj)
{
  m_mutex.lock();
  m_objectQueue.push(std::pair<bool, WaitObject*>(true, &obj));
  m_mutex.unlock();
  m_triggerEvent.set();
}

void BaseThread::removeWaitObject(WaitObject& obj)
{
  m_mutex.lock();
  m_objectQueue.push(std::pair<bool, WaitObject*>(false, &obj));
  m_mutex.unlock();
  m_triggerEvent.set();
}

void BaseThread::handleObjectQueue()
{
  m_mutex.lock();
  while(m_objectQueue.size() > 0)
  {
    if(m_objectQueue.front().first)
      m_waitSet.add(*m_objectQueue.front().second);
    else
      m_waitSet.remove(*m_objectQueue.front().second);

    m_objectQueue.pop();
  }
  m_mutex.unlock();
}

void BaseThread::setWaitTimeout(uint32_t timeout)
{
  m_timeout = timeout;
}

void BaseThread::error(Handle handle GCC_UNUSED)
{
  // Do nothing, optionally implemented by a derived class
}

void BaseThread::abandoned(Handle handle GCC_UNUSED)
{
  // Do nothing, optionally implemented by a derived class
}
