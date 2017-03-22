#ifndef LOGGER_H__
#define LOGGER_H__

#include <boost/lexical_cast.hpp>

class LogEntry
{
public:
  LogEntry(const std::string &type);
  LogEntry(const LogEntry &o);

  ~LogEntry();

  LogEntry& operator<< (const char *);
  LogEntry& operator<< (const std::string&);
  LogEntry& operator<< (int);
  LogEntry& operator<< (double);

  template <typename T>
  LogEntry &operator<< (T t) {
    return (*this) << boost::lexical_cast<std::string>(t);
  }

private:
  mutable std::string preamble_;
  mutable std::stringstream ss_;
};

static inline LogEntry trace(const std::string& type) {
  return LogEntry(type);
}

#endif //LOGGER_H__
