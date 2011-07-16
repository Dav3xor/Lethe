#ifndef _WINDOWSSHAREDMEMORY_H
#define _WINDOWSSHAREDMEMORY_H

#include "LetheTypes.h"
#include <string>

namespace lethe
{
  class WindowsSharedMemory
  {
  public:
    WindowsSharedMemory(const std::string& name, uint32_t size);
    ~WindowsSharedMemory();

    void* begin() const;
    void* end() const;

    uint32_t size() const;
    const std::string& name() const;

  private:
    // Private, undefined copy constructor and assignment operator so they can't be used
    WindowsSharedMemory(const WindowsSharedMemory&);
    WindowsSharedMemory& operator = (const WindowsSharedMemory&);

    static const std::string s_nameBase;
    const std::string m_name;
    Handle m_handle;
    void* m_data;
    uint32_t m_size;
  };
}

#endif