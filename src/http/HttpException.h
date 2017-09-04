#ifndef HTTP_EXCEPTION__
#define HTTP_EXCEPTION__

#include "common/InterceptorException.h"
#include "Http.h"

namespace Interceptor::Http {

  class HttpException : public InterceptorException {
  public:
    HttpException(Code code, bool closeConnection = false,
                  const std::string& msg = "")
      : InterceptorException(msg),
        m_code(code),
        m_close(closeConnection)
    {

    }

    Code code()  const
    {
      return m_code;
    }

    bool closeConnection()  const
    {
      return m_close;
    }

  private:
    Code m_code;
    bool m_close;
  };
}
#endif
