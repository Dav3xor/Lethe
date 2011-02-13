#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"

TEST_CASE("waitSet/structor", "Test constructor/destructor")
{
  // No constructor parameters, so just create a few and see if we crash
  const uint32_t numSets(100);
  WaitSet waitSet[numSets];

  for(uint32_t i(0); i < numSets; ++i)
  {
    REQUIRE(waitSet[i].getSize() == 0);
  }
}

// Create an instantiable thread object (so we can add a thread to the WaitSet)
class WaitSetDummyThread : public Thread
{
public:
  WaitSetDummyThread() : Thread(0) { };
protected:
  void iterate(Handle handle) { handle = INVALID_HANDLE_VALUE; stop(); };
};

// Create an error condition with a custom WaitObject
class InvalidWaitObject : public WaitObject
{
public:
  InvalidWaitObject() : WaitObject(INVALID_HANDLE_VALUE) { };
};

TEST_CASE("waitSet/add", "Test adding WaitObjects")
{
  WaitSet waitSet;
  Mutex mutex(false);
  Event event(false, false);
  Timer timer;
  Semaphore semaphore(1, 1);
  WaitSetDummyThread thread;
  Pipe pipe;
  InvalidWaitObject invalid;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 1);

  waitSet.add(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 2);

  waitSet.add(timer);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 3);

  waitSet.add(semaphore);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 4);

  waitSet.add(thread);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 5);

  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 6);

  // Try to add everything twice, expect failures
  REQUIRE_FALSE(waitSet.add(mutex));
  REQUIRE_FALSE(waitSet.add(event));
  REQUIRE_FALSE(waitSet.add(timer));
  REQUIRE_FALSE(waitSet.add(semaphore));
  REQUIRE_FALSE(waitSet.add(thread));
  REQUIRE_FALSE(waitSet.add(pipe));
  REQUIRE_THROWS_AS(waitSet.add(invalid), std::exception);
  REQUIRE(waitSet.getSize() == 6);
}

TEST_CASE("waitSet/remove", "Test removing WaitObjects")
{
  WaitSet waitSet;
  Mutex mutex(false);
  Event event(false, false);
  Timer timer;
  Semaphore semaphore(1, 1);
  WaitSetDummyThread thread;
  Pipe pipe;
  InvalidWaitObject invalid;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(mutex);
  waitSet.add(event);
  waitSet.add(timer);
  waitSet.add(semaphore);
  waitSet.add(thread);
  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 6);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 6);

  waitSet.remove(pipe);
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_FALSE(waitSet.remove(pipe));
  REQUIRE(waitSet.getSize() == 5);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 5);

  waitSet.remove(mutex);
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_FALSE(waitSet.remove(mutex));
  REQUIRE(waitSet.getSize() == 4);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 4);

  waitSet.remove(semaphore);
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_FALSE(waitSet.remove(semaphore));
  REQUIRE(waitSet.getSize() == 3);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 3);

  waitSet.remove(event);
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_FALSE(waitSet.remove(event));
  REQUIRE(waitSet.getSize() == 2);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 2);

  waitSet.remove(thread);
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_FALSE(waitSet.remove(thread));
  REQUIRE(waitSet.getSize() == 1);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 1);

  waitSet.remove(timer);
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_FALSE(waitSet.remove(timer));
  REQUIRE(waitSet.getSize() == 0);
  REQUIRE_FALSE(waitSet.remove(invalid));
  REQUIRE(waitSet.getSize() == 0);
}

TEST_CASE("waitSet/waitAll", "Test waiting for all WaitObjects")
{
  // TODO: implement waitSet/waitAll
}

TEST_CASE("waitSet/waitAny", "Test waiting for any WaitObjects")
{
  // The waitAny function may throw exceptions, but I can't find a way to make it do so
  //  This would probably require an OS error to happen
  WaitSet waitSet;
  uint8_t buffer[5] = { "test" }; // Buffer for reading/writing on pipe
  Handle waitHandle;

  // Create wait objects initially unsignalled
  Event event(false, true);
  Timer timer;
  Semaphore semaphore(10, 0);
  WaitSetDummyThread thread;
  Pipe pipe;

  REQUIRE(waitSet.getSize() == 0);

  waitSet.add(event);
  waitSet.add(timer);
  waitSet.add(semaphore);
  waitSet.add(thread);
  waitSet.add(pipe);
  REQUIRE(waitSet.getSize() == 5);

  // Trigger each object, make sure we get exactly one at a time
  event.set();
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Event is reset here
  REQUIRE(waitHandle == event.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  timer.start(1);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == timer.getHandle());
  timer.clear(); // This may be changed if we support auto-reset in LinuxTimers
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  semaphore.unlock(1);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess); // Semaphore is relocked here
  REQUIRE(waitHandle == semaphore.getHandle());
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  thread.start();
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitSuccess); // WaitSetDummyThread may take a little time to exit
  REQUIRE(waitHandle == thread.getHandle());
  waitSet.remove(thread); // Thread is not resettable at the moment
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  pipe.send(buffer, 5);
  REQUIRE(waitSet.waitAny(0, waitHandle) == WaitSuccess);
  REQUIRE(waitHandle == pipe.getHandle());
  pipe.receive(buffer, 5);
  REQUIRE(waitSet.waitAny(100, waitHandle) == WaitTimeout);

  // Create a second thread since we used up the first
  WaitSetDummyThread thread2;
  waitSet.add(thread2);

  // Now try triggering all the objects at once and make sure that every object finishes
  thread2.start();
  timer.start(1);
  event.set();
  semaphore.unlock(1);
  pipe.send(buffer, 5);

  // Create a set to track the handles we still need to wait on
  std::set<Handle> unfinished;
  unfinished.insert(thread2.getHandle());
  unfinished.insert(timer.getHandle());
  unfinished.insert(event.getHandle());
  unfinished.insert(semaphore.getHandle());
  unfinished.insert(pipe.getHandle());

  while(unfinished.size() > 0)
  {
    REQUIRE(waitSet.waitAny(1, waitHandle) == WaitSuccess);

    // Special handling to reset timer, thread, and pipe handles
    if(waitHandle == timer.getHandle())
      timer.clear();
    else if(waitHandle == thread2.getHandle())
      waitSet.remove(thread2);
    else if(waitHandle == pipe.getHandle())
      pipe.receive(buffer, 5);

    REQUIRE(unfinished.erase(waitHandle) == 1);
  }

  // Try getting multiple events at once, then removing a wait objects
}

// TODO: add test for abandoned (deleted) WaitObjects
