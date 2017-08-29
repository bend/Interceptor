#include "BackendReply.h"

namespace Interceptor {

  void BackendReply::appendData(const char* data, size_t size)
  {
    m_stream->write(data, size);
    //  need to set the var headerReceived
  }

  bool BackendReply::received() const
  {
    // needs to check either the header Content-Length
    // or parse for chunking (check for \r\n0\r\n delimiter)
    return true;
  }

}
