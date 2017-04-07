#ifndef LOGGER_H__
#define LOGGER_H__

#include <boost/lexical_cast.hpp>

#include "vars.h"

class LogEntry {
public:
  LogEntry(const std::string& type);
  LogEntry(const LogEntry& o);

  ~LogEntry();

  LogEntry& operator<< (const char*);
  LogEntry& operator<< (const std::string&);
  LogEntry& operator<< (int);
  LogEntry& operator<< (double);

  template <typename T>
  LogEntry& operator<< (T t)
  {
    return (*this) << boost::lexical_cast<std::string>(t);
  }

private:
  mutable std::string preamble_;
  mutable std::stringstream ss_;
};

static inline LogEntry trace(const std::string& type)
{
  return LogEntry(type);
}


#define LOG_INFO(A) trace("info") << A
#define LOG_WARN(A) trace("warn") << A
#define LOG_ERROR(A)  trace("err ") << A
#ifdef DEBUG_LOGGING
#define LOG_DEBUG(A) trace("dbg ") << A
#else
#define LOG_DEBUG(A)
#endif // ifdef LOG_DEBUG(A)

#endif //LOGGER_H__
