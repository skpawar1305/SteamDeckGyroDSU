#ifndef _KMICKI_LOG_LOG_H_
#define _KMICKI_LOG_LOG_H_

#include <sstream>
#include <string>

namespace kmicki::log {
enum LogLevel {
  LogLevelNone = 0,
  LogLevelDefault = 1,
  LogLevelDebug = 2,
  LogLevelTrace = 3
};

extern LogLevel currentLogType;

void SetLogLevel(LogLevel type);

LogLevel const &GetLogLevel();

// Log a string message
void Log(std::string message, LogLevel type = LogLevelDefault);

// class for logging formatted message
// Behaves like output stream and message gets logged on destruction.
// Usage: { LogF() << "This is an example message number " << nr << "!"; }
class LogF : protected std::ostringstream {
public:
  LogF(LogLevel type = LogLevelDefault);
  ~LogF();

  template <class T> LogF &operator<<(T const &val) {
    if (logType <= currentLogType)
      *((std::ostringstream *)this) << val;
    return *this;
  }

  // Log message without desctruction.
  void LogNow();

private:
  LogLevel logType;
};
} // namespace kmicki::log

#endif