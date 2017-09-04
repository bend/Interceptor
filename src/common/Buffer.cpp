#include "Buffer.h"

namespace Interceptor {


  Buffer::Buffer()
    : m_flags(0)
  {}

  Buffer::~Buffer()
  {
    LOG_DEBUG("Http::Buffer::~Buffer()");

    for (auto& b : m_bufs2) {
      delete [] b;
    }
  }
  int Buffer::flags() const
  {
    return m_flags;
  }

  std::function<std::shared_ptr<Buffer>()> Buffer::nextCall() const
  {
    return m_nextCall;
  }

  boost::asio::const_buffer Buffer::buf(const std::string& s)
  {
    m_bufs.push_back(s);
    return boost::asio::buffer(m_bufs.back());
  }

  boost::asio::const_buffer Buffer::buf(char* buf,
                                        size_t s)
  {
    m_bufs2.push_back(buf);
    return boost::asio::buffer(m_bufs2.back(), s);
  }


};
