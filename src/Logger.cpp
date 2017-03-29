#include "Logger.h"

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

LogEntry::LogEntry(const std::string& type)
{
  std::stringstream ss;
  ss << '[' <<
     boost::posix_time::to_simple_string(
       boost::posix_time::microsec_clock::local_time()
     ) << ']' << ' ' << type << ' ';
  preamble_ = ss.str();
}

LogEntry::LogEntry(const LogEntry& o)
  : preamble_(o.preamble_)
{
  if (preamble_.empty()) return;

  ss_ << o.ss_.str();
  o.preamble_ = "";
  o.ss_.clear();
}

LogEntry::~LogEntry()
{
  if (preamble_.empty()) return;

  std::string s = ss_.str();

  if (!s.empty()) {
    std::stringstream ss;
    ss << preamble_ << s << "\n";
    std::cerr << ss.str() << std::flush;
  }
}

LogEntry& LogEntry::operator <<(const char* s)
{
  if (preamble_.empty()) return *this;

  ss_ << s;
  return *this;
}

LogEntry& LogEntry::operator <<(const std::string& s)
{
  if (preamble_.empty()) return *this;

  ss_ << s;
  return *this;
}

LogEntry& LogEntry::operator <<(int i)
{
  if (preamble_.empty()) return *this;

  ss_ << i;
  return *this;
}

LogEntry& LogEntry::operator <<(double d)
{
  if (preamble_.empty()) return *this;

  ss_ << d;
  return *this;
}
