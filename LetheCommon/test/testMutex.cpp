#include "Lethe.h"
#include "LetheException.h"
#include "LetheInternal.h"
#include "testCommon.h"
#include "catch/catch.hpp"

/**
 *
 * Test case: structor
 *
 */

using namespace lethe;

TEST_CASE("mutex/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numMutexes(1000);
  Mutex** mutexArray = new Mutex*[numMutexes];

  for(uint32_t i(0); i < numMutexes; ++i)
  {
    mutexArray[i] = new Mutex(!(i % 2));
  }

  for(uint32_t i(0); i < numMutexes; ++i)
  {
    delete mutexArray[i];
  }

  delete [] mutexArray;
}


/**
 *
 * Test case: autolock
 *
 */

class MutexTestThread : public Thread
{
public:
  MutexTestThread(Mutex& mutex, Event& event) :
    Thread(INFINITE),
    m_mutex(mutex),
    m_event(event),
    m_iterating(false)
  {
    addWaitObject(m_mutex);
  }

  bool isIterating() { return m_iterating; };

private:
  Mutex& m_mutex;
  Event& m_event;
  bool m_iterating;

protected:
  void iterate(Handle handle)
  {
    m_iterating = true;

    if(handle != m_mutex.getHandle())
    {
      m_iterating = false;
      throw std::invalid_argument("invalid handle");
    }

    if(WaitForObject(m_event, 2000) != WaitSuccess)
    {
      m_iterating = false;
      throw std::logic_error("thread did not receive secondary event");
    }

    stop();
    m_mutex.unlock();
    m_iterating = false;
  };

  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("abandoned mutex in MutexTestThread"); };
  void error(Handle handle GCC_UNUSED) { throw std::logic_error("errored mutex in MutexTestThread"); };
};

// This requres Thread to work, which requires Event and WaitSet to work
//  If this test case is failing, it could be due to a problem in Thread,
//  Event, or WaitSet.
TEST_CASE("mutex/autolock", "Test auto-lock and unlock with multiple waiting threads")
{
  const uint32_t threadCount(2); // Can't wait on more than 64 things at once on Windows
  MutexTestThread* threadArray[threadCount];
  WaitSet activeThreads;
  Mutex mutex(true);
  Event exitEvent(false, true);

  for(uint32_t i(0); i < threadCount; ++i)
  {
    threadArray[i] = new MutexTestThread(mutex, exitEvent);
    activeThreads.add(*threadArray[i]);
    threadArray[i]->start();
  }

  // Kick off the chain
  mutex.unlock();

  while(activeThreads.getSize() > 0)
  {
    uint32_t iterateCount(0);
    Handle exitedThread;

    // Wait some time to make sure no threads exit prematurely
    REQUIRE(activeThreads.waitAny(30, exitedThread) == WaitTimeout);

    // Loop through the threads, make sure exactly one is in iterate
    for(uint32_t i(0); i < threadCount; ++i)
      iterateCount += threadArray[i]->isIterating();
 
    REQUIRE(iterateCount == 1);

    // Set the event to let the thread exit
    exitEvent.set();

    // Check that the thread finishes
    REQUIRE(activeThreads.waitAny(1000, exitedThread) == WaitSuccess);
    activeThreads.remove(exitedThread);
  }

  for(uint32_t i(0); i < threadCount; ++i)
  {
    REQUIRE(threadArray[i]->isStopping() == true);
    REQUIRE(threadArray[i]->getError() == "");
    delete threadArray[i];
  }
}


/**
 *
 * Test case: exception
 *
 */

class ExceptionTestThread : public Thread
{
public:
  ExceptionTestThread(Mutex& mutex);

private:
  Mutex& m_mutex;

protected:
  void iterate(Handle handle)
  {
    if(handle != INVALID_HANDLE_VALUE)
      throw std::invalid_argument("invalid handle");

    m_mutex.unlock();
    stop();
  };

  void abandoned(Handle handle)
  {
    handle = INVALID_HANDLE_VALUE;
    throw std::logic_error("mutex abandoned");
  };
};

ExceptionTestThread::ExceptionTestThread(Mutex& mutex) :
  Thread(0), // call iterate as soon as the thread starts
  m_mutex(mutex)
{
  // Do nothing
}

// Helper function to cut down on copy-paste
void runExceptionThread(Mutex& mutex)
{
  ExceptionTestThread thread(mutex);
  thread.start();

  REQUIRE(WaitForObject(thread, 1000) == WaitSuccess);
  REQUIRE(thread.isStopping());

#if defined(__linux__) // Different error message on linux
  REQUIRE(thread.getError() == "eventfd write failed: Operation not permitted");
#elif defined(__WIN32__) || defined(_WIN32)
  REQUIRE(thread.getError() == "ReleaseMutex failed: Attempt to release mutex not owned by caller. ");
#endif
}

void runExceptionTest(bool initialState)
{
  // Start with a locked mutex
  Mutex mutex(initialState);
  runExceptionThread(mutex);

  if(initialState)
    mutex.unlock();
  runExceptionThread(mutex);

  mutex.lock();
  runExceptionThread(mutex);

  mutex.unlock();
  runExceptionThread(mutex);

  // Autolock tests
  WaitForObject(mutex);
  runExceptionThread(mutex);

  mutex.unlock();
  runExceptionThread(mutex);

  WaitForObject(mutex);
  runExceptionThread(mutex);

  // Manual lock tests again to make sure it isn't in a bad state
  mutex.unlock();
  runExceptionThread(mutex);

  mutex.lock();
  runExceptionThread(mutex);

  mutex.unlock();
  runExceptionThread(mutex);
}

TEST_CASE("mutex/exception", "Test that thread id is enforced")
{
  // Try the same test with both initial states
  runExceptionTest(true);
  runExceptionTest(false);
}

TEST_CASE("mutex/lock", "Test manual locking behavior")
{
  Mutex mutex1(true);

  mutex1.lock();
  mutex1.lock();
  mutex1.unlock();
  mutex1.unlock();
  mutex1.unlock();
  REQUIRE_THROWS_AS(mutex1.unlock(), std::bad_syscall);
  REQUIRE_THROWS_AS(mutex1.unlock(), std::bad_syscall);
  
  Mutex mutex2(false);

  mutex2.lock();
  mutex2.lock();
  mutex2.unlock();
  mutex2.unlock();
  REQUIRE_THROWS_AS(mutex2.unlock(), std::bad_syscall);
  REQUIRE_THROWS_AS(mutex2.unlock(), std::bad_syscall);
  
  // TODO: implement mutex/lock
}

TEST_CASE("mutex/timeout", "Test timeout behavior")
{
  // TODO: implement mutex/timeout
}

TEST_CASE("mutex/multilock", "Test multilock behavior")
{
  // TODO: test mutex/multilock
}
