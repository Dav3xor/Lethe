#include "Abstraction.h"
#include "Exception.h"
#include "catch.hpp"
#include "testCommon.h"

TEST_CASE("semaphore/structor", "Test construction/destruction")
{
  // Create a lot of mutexes
  const uint32_t numSemaphores(1000);
  Semaphore** semaphoreArray = new Semaphore*[numSemaphores];

  for(uint32_t i(0); i < numSemaphores; ++i)
  {
    // Use 'i' to determine the parameters
    semaphoreArray[i] = new Semaphore(i, i & bit1);
  }

  // Put the Semaphores into some different states
  for(uint32_t i(0); i < numSemaphores; ++i)
  {
//    if(i & bit2)
//      semaphoreArray[i]->lock();
//    if(i & bit3)
//      semaphoreArray[i]->release();
//    if(i & bit4)
//      semaphoreArray[i]->lock();
//    if(i & bit5)
//      semaphoreArray[i]->release();
  }

  // Destroy
  for(uint32_t i(0); i < numSemaphores; ++i)
  {
    delete semaphoreArray[i];
  }

  delete [] semaphoreArray;
}

TEST_CASE("semaphore/autolock", "Test semaphore autolock behavior")
{
  // TODO: implement semaphore/autolock
}

TEST_CASE("semaphore/lock", "Test semaphore lock behavior")
{
  // TODO: implement semaphore/lock
}

TEST_CASE("semaphore/release", "Test semaphore release behavior")
{
  // TODO: implement semaphore/release
}

TEST_CASE("semaphore/exception", "Test semaphore exception behavior")
{
  // TODO: implement semaphore/exception
}

