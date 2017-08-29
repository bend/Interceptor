#ifndef LOGGER_H__
#define LOGGER_H__

#include "vars.h"
#include <boost/lexical_cast.hpp>
#include <fstream>

namespace Interceptor {

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
    mutable std::string m_preamble;
    mutable std::stringstream m_ss;
  };

  static inline LogEntry trace(const std::string& type)
  {
    return LogEntry(type);
  }

  static inline void dumpToFile(const std::string& file, const std::string& data)
  {
	std::ofstream out(file.c_str(), std::ios::app);
	out << data;
  }

}


#define LOG_INFO(A) trace("info") << A
#define LOG_WARN(A) trace("warn") << A
#define LOG_ERROR(A)  trace("err ") << A

#ifdef DEBUG_LOGGING
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_DEBUG(A) trace("dbg ") << __FILENAME__ << ":" << __LINE__ << " - " << A

#ifdef DUMP_NETWORK
#define LOG_NETWORK(A, B) trace("net") << __FILENAME__ << ":" << __LINE__ <<" - " << A << "\n" << B
#else
#define LOG_NETWORK(A, B)
#endif // DUMP_NETWORK

#else
#define LOG_DEBUG(A)
#define LOG_NETWORK(A, B)
#endif // DEBUG_LOGGING

#endif //LOGGER_H__
