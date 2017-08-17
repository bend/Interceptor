#include "Logger.h"

#include <iostream>
#include <thread>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Interceptor {
  LogEntry::LogEntry(const std::string& type)
  {
    std::stringstream ss;
    ss << '[' <<
       boost::posix_time::to_simple_string(
         boost::posix_time::microsec_clock::local_time()
       ) << ']' << '[' << std::this_thread::get_id() << "] " << type << ' ';
    m_preamble = ss.str();
  }

  LogEntry::LogEntry(const LogEntry& o)
    : m_preamble(o.m_preamble)
  {
    if (m_preamble.empty()) {
      return;
    }

    m_ss << o.m_ss.str();
    o.m_preamble = "";
    o.m_ss.clear();
  }

  LogEntry::~LogEntry()
  {
    if (m_preamble.empty()) {
      return;
    }

    std::string s = m_ss.str();

    if (!s.empty()) {
      std::stringstream ss;
      ss << m_preamble << s << "\n";
      std::cerr << ss.str() << std::flush;
    }
  }

  LogEntry& LogEntry::operator <<(const char* s)
  {
    if (m_preamble.empty()) {
      return *this;
    }

    m_ss << s;
    return *this;
  }

  LogEntry& LogEntry::operator <<(const std::string& s)
  {
    if (m_preamble.empty()) {
      return *this;
    }

    m_ss << s;
    return *this;
  }

  LogEntry& LogEntry::operator <<(int i)
  {
    if (m_preamble.empty()) {
      return *this;
    }

    m_ss << i;
    return *this;
  }

  LogEntry& LogEntry::operator <<(double d)
  {
    if (m_preamble.empty()) {
      return *this;
    }

    m_ss << d;
    return *this;
  }

}
