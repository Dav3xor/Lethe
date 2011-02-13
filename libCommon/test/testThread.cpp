#include "Abstraction.h"
#include "AbstractionException.h"
#include "catch.hpp"
#include "testCommon.h"

// TestThreadDummy thread loops until manually stop()ed
class TestThreadDummyThread : public Thread
{
public:
  TestThreadDummyThread() : Thread(0) { };
  ~TestThreadDummyThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { /* Do nothing */ };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("TestThreadDummyThread received abandoned event"); };
};

// StopThread stops itself after the given timeout
class StopThread : public Thread
{
public:
  StopThread(uint32_t timeout) : Thread(timeout) { };
  ~StopThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { stop(); };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("StopThread received abandoned event"); };
};

// WaitThread waits until the given object triggers, then stops
class WaitThread : public Thread
{
public:
  WaitThread(WaitObject& obj) : Thread(INFINITE) { addWaitObject(obj); };
  ~WaitThread() { };

protected:
  void iterate(Handle handle GCC_UNUSED) { stop(); };
  void abandoned(Handle handle GCC_UNUSED) { throw std::logic_error("WaitThread received abandoned event"); };
};

TEST_CASE("thread/structor", "Test construction/destruction")
{
  uint32_t startTime;

  for(uint32_t i = 0; i < 256; ++i)
  {
    // Create one of each type of thread
    Thread* dThread = new TestThreadDummyThread();
    Thread* sThread = new StopThread(10);
    Thread* wThread = new WaitThread(*dThread);

    if(i & bit1)
      dThread->start();
    if(i & bit2)
      sThread->start();
    if(i & bit3)
      wThread->start();

    if(i & bit4)
      dThread->stop();
    if(i & bit5)
      sThread->stop();
    if(i & bit6)
      wThread->stop();

    REQUIRE(dThread->getError() == "");
    REQUIRE(sThread->getError() == "");
    REQUIRE(wThread->getError() == "");

    // Make sure the threads destruct successfully in a timely fashion
    startTime = getTime();
    delete sThread;
    dThread->stop();
    delete wThread;
    delete dThread;
    // This is set high because the first run is rather slow with valgrind
    REQUIRE(getTime() - startTime < 400);
  }
}

TEST_CASE("thread/run", "Test running threads")
{
  // TODO: implement thread/run
}

TEST_CASE("thread/exception", "Test thread exception handling")
{
  // TODO: implement thread/exception
}

TEST_CASE("thread/pause", "Test pausing threads")
{
  // TODO: implement thread/pause
}

TEST_CASE("thread/stop", "Test stopping threads")
{
  // TODO: implement thread/stop
}
