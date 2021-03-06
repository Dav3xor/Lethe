#ifndef _LOG_H
#define _LOG_H

#include "StaticSingleton.h"
#include "LetheBasic.h"
#include <fstream>
#include <sstream>

/*
 * The Log class provides an ostream-style interface for writing log statements of
 *  various levels to different targets.  One target is selected at a time process-wide,
 *  and one log-level is selected, which may filter out statements of lower severity.
 *
 * This class is implemented as a static singleton, meaning that it is created during
 *  static initialization and destroyed when the process exits.  This means that as soon
 *  as the main() function begins, logging is available.  The default is to write to the
 *  console.  It is suggested to only use the Log* macros when writing a statement.
 *  Using the Log class directly is rather cumbersome and not encouraged.
 *
 * Example:
 *  LogInfo("text: " << value << ", more text");
 *  LogError("panic");
 *
 * The Log class uses LogHandler subclasses to define the behavior in different modes.
 *  At the moment, there are only modes for writing to the console, writing to a file,
 *  or disabling all log statements outright.
 *
 * In the future, this may be extended to that a user may provide their own log handlers
 *  to redirect output elsewhere or do other specialized tasks.
 */

// The LOG_BASE may be overridden by a user who knows what they're doing (before including this file)
#if !defined(LOG_BASE)
  #define LOG_BASE(level) level << lethe::Log::Time << ", " << __FILE__ << ":" << __LINE__
#endif

#ifndef DISABLE_LOG_ERROR
  #define LogError(a) do                                             \
                      {                                              \
                        lethe::Log& log = lethe::Log::getInstance(); \
                        if(log.getLevel() >= lethe::Log::Error)      \
                        {                                            \
                          log.lock();                                \
                          log << LOG_BASE(lethe::Log::Error)         \
                              << " Error - " << a                    \
                              << lethe::Log::Commit;                 \
                          log.unlock();                              \
                        }                                            \
                      } while(0)
#else
  #define LogError(...) do { ; } while(0)
#endif

#ifndef DISABLE_LOG_INFO
  #define LogInfo(a)  do                                             \
                      {                                              \
                        lethe::Log& log = lethe::Log::getInstance(); \
                        if(log.getLevel() >= lethe::Log::Info)       \
                        {                                            \
                          log.lock();                                \
                          log << LOG_BASE(lethe::Log::Info)          \
                              << " Info - " << a                     \
                              << lethe::Log::Commit;                 \
                          log.unlock();                              \
                        }                                            \
                      } while(0)
#else
  #define LogInfo(...) do { ; } while(0)
#endif

#ifndef DISABLE_LOG_DEBUG
  #define LogDebug(a) do                                             \
                      {                                              \
                        lethe::Log& log = lethe::Log::getInstance(); \
                        if(log.getLevel() >= lethe::Log::Debug)      \
                        {                                            \
                          log.lock();                                \
                          log << LOG_BASE(lethe::Log::Debug)         \
                              << " Debug - " << a                    \
                              << lethe::Log::Commit;                 \
                          log.unlock();                              \
                        }                                            \
                      } while(0)
#else
  #define LogDebug(...) do { ; } while(0)
#endif

namespace lethe
{
  class Log : public StaticSingleton<Log>
  {
  public:

    enum Level
    {
      Disabled = 0,
      Error = 1,
      Info = 2,
      Debug = 3,
      NumLevels = 4
    };

    Level getLevel() const;
    void setLevel(Level level);

    void setStreamMode(std::ostream& out);
    void setFileMode(const std::string& filename);

    void lock();
    void unlock();

    template <typename T>
    Log& operator << (const T& data);

    enum Command
    {
      Commit = 0,
      Time = 1
    };

    Log& operator << (Command c);
    Log& operator << (Level level);

  private:
    friend class StaticSingleton<Log>;
    Log();
    ~Log();

    // Private, undefined copy constructor and assignment operator so they can't be used
    Log(const Log&);
    Log& operator = (const Log&);

    // Abstract base class for different styles of logging
    class LogHandler
    {
    public:
      virtual void write(const std::string& statement) = 0;
      virtual ~LogHandler() { };
    };

    // Class for logging to standard output
    class StreamLogHandler : public LogHandler
    {
    public:
      StreamLogHandler(std::ostream& out);

      void write(const std::string& statement);

    private:
      std::ostream& m_out;
    };

    // Class for logging to a file
    class FileLogHandler : public LogHandler
    {
    public:
      FileLogHandler(const std::string& filename);
      ~FileLogHandler();

      void write(const std::string& statement);

    private:
      const std::string m_filename;
      std::ofstream m_out;
    };

    Mutex m_mutex;
    Level m_logLevel;
    Level m_statementLevel;
    std::stringstream m_statement;
    LogHandler* m_handler;
  };

  // Note: locking should be done by the Log* macros (to keep lock/unlock cycles low)
  template <typename T>
  Log& Log::operator << (const T& data)
  {
    m_statement << data;
    return *this;
  }
}

#endif
