#ifndef HTTP_BUFFER_H__
#define HTTP_BUFFER_H__

namespace Http {

  class HttpBuffer {
  public:
    ~HttpBuffer()
    {
      //TODO
    }
  public:
    std::vector<boost::asio::const_buffer> m_buffers;

  private:
    std::vector<std::string> m_bufs;
    std::vector<char*> m_bufs2;
    friend class HttpReply;
  };

}

#endif // HTTP_BUFFER_H__
