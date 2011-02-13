#include "AbstractionFunctions.h"
#include "AbstractionBasic.h"
#include "AbstractionException.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>

void Sleep(uint32_t timeout)
{
  usleep(timeout * 1000);
}

std::string lastError()
{
  int errorCode(errno);
  char buffer[200];

  return std::string(strerror_r(errorCode, buffer, 200));
}

uint32_t seedRandom(uint32_t seed)
{
  if(seed == 0)
  {
    timeval currentTime;
    gettimeofday(&currentTime, NULL);

    seed = currentTime.tv_usec | (currentTime.tv_sec << 20);
  }

  srand(seed);

  return seed;
}

void getFileList(const std::string& directory GCC_UNUSED,
                 std::vector<std::string>& fileList GCC_UNUSED)
{
  throw std::logic_error("LinuxFunctions::getFileList not yet implemented");
}

uint64_t getTime()
{
  timeval currentTime;

  if(gettimeofday(&currentTime, NULL) != 0)
    throw std::bad_syscall("gettimeofday", lastError());

  return (currentTime.tv_sec * 1000) + (currentTime.tv_usec / 1000);
}

std::string getTimeString()
{
  std::ostringstream stream;
  char buffer[100];
  struct tm* currentTime;
  struct timeval rawTime;

  gettimeofday(&rawTime, NULL);
  currentTime = localtime(&rawTime.tv_sec);
  strftime(buffer, sizeof(buffer), "%b %d %H:%M:%S.", currentTime);

  stream << buffer << std::setfill('0') << std::setw(3) << (rawTime.tv_usec / 1000);

  return stream.str();
}

WaitResult WaitForObject(WaitObject& obj, uint32_t timeout)
{
  WaitResult result = WaitSuccess;
  struct pollfd pollData;

  if(obj.preWaitCallback())
    return WaitSuccess;

  pollData.fd = obj.getHandle();
  pollData.events = POLLIN | POLLERR | POLLHUP;

  switch(poll(&pollData, 1, timeout))
  {
  case 1:
    if(pollData.revents & (POLLERR | POLLNVAL | POLLHUP))
      result = WaitAbandoned;

    obj.postWaitCallback(result);
    break;

  case 0:
    result = WaitTimeout;
    break;

  default:
    throw std::bad_syscall("poll", lastError());
  }

  return result;
}

