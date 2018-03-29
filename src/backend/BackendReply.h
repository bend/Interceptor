#ifndef BACKEND_REPLY_H__
#define BACKEND_REPLY_H__

#include <sstream>

namespace Interceptor::Backends {

  class BackendReply {
  public:
    BackendReply() = default;
    ~BackendReply() = default;

    void appendData(const char* data, size_t size);
    bool received() const;

  private:
    std::stringstream* m_stream;
  };

}

#endif
