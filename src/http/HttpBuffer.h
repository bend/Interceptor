#ifndef HTTP_BUFFER_H__
#define HTTP_BUFFER_H__

#include "utils/Logger.h"

namespace Http {

  class HttpBuffer {
  public:
    enum State {
      Closing = 0x01,
      HasMore = 0x02,
      InvalidRequest = 0x04
    };

    HttpBuffer()
      : m_flags(0)
    {}


    ~HttpBuffer()
    {
      LOG_DEBUG("HttpBuffer::~HttpBuffer()");

      for (auto& b : m_bufs2) {
        delete [] b;
      }
    }

    int flags() const
    {
      return m_flags;
    }

    auto nextCall() const
    {
      return m_nextCall;
    }

  public:
    std::vector<boost::asio::const_buffer> m_buffers;

  private:
    std::vector<std::string> m_bufs;
    std::vector<char*> m_bufs2;
    friend class HttpReply;
    int m_flags;
    std::function<bool()> m_nextCall;

  };

}

#endif // HTTP_BUFFER_H__
