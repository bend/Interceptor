#ifndef INTERCEPTOR_EXCEPTION_H__
#define INTERCEPTOR_EXCEPTION_H__

#include <exception>
#include <string>

namespace Interceptor {

  class InterceptorException : public std::exception {
  public:
    InterceptorException(const std::string& s)
      : m_what(s) {}

    virtual ~InterceptorException() noexcept {}

    virtual const char* what() const noexcept
    {
      return m_what.c_str();
    }

  private:
    std::string m_what;
  };

}

#endif // INTERCEPTOR_EXCEPTION_H__ 
