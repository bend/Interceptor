#ifndef HTTP_BUFFER_H__
#define HTTP_BUFFER_H__

#include "utils/Logger.h"

#include <list>
#include <memory>

#include <boost/asio/buffer.hpp>

namespace Interceptor {

  class Buffer {
  public:
    enum State {
      Closing = 0x01,
      HasMore = 0x02,
      InvalidRequest = 0x04
    };

    Buffer();

    ~Buffer();

    int flags() const;

    std::function<std::shared_ptr<Buffer>()> nextCall() const;

    boost::asio::const_buffer buf(const std::string& s);
    boost::asio::const_buffer buf(char* buf, size_t s);


  public:
    std::vector<boost::asio::const_buffer> m_buffers;
    int m_flags;
    std::function<std::shared_ptr<Buffer>()> m_nextCall;

  private:
    std::list<std::string> m_bufs;
    std::list<char*> m_bufs2;
  };

}

#endif // HTTP_BUFFER_H__
