#ifndef REDIRECTION_H__
#define REDIRECTION_H__

#include <string>

namespace Interceptor {

  class Redirection {
  public:
    enum Type {
      Redirect,
      Permanent
    };

    Redirection(const std::string& regex, const std::string& redirect, Type type);

    bool matches(const std::string& url) const;

    std::string redirectURL() const;

    Type type() const;

  private:
    std::string m_regex;
    std::string m_redirectURL;

    Type m_redirectionType;
  };

}

#endif // REDIRECTION_H__
