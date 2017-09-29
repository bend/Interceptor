#ifndef HTTP_EXCEPTION__
#define HTTP_EXCEPTION__

#include "common/InterceptorException.h"
#include "Http.h"

namespace Interceptor::Http {

  class HttpException : public InterceptorException {
  public:
    HttpException(StatusCode code, bool closeConnection = false,
                  const std::string& msg = "")
      : InterceptorException(msg),
        m_code(code),
        m_close(closeConnection)
    {

    }

    StatusCode code()  const
    {
      return m_code;
    }

    bool closeConnection()  const
    {
      return m_close;
    }

  private:
    StatusCode m_code;
    bool m_close;
  };
}
#endif
