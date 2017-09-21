#include "Redirection.h"

#include "utils/StringUtils.h"

namespace Interceptor {

  Redirection::Redirection(const std::string& regex,
                           const std::string& redirect, Type type)
    : m_regex(regex),
      m_redirectURL(redirect),
      m_redirectionType(type)
  {}

  bool Redirection::matches(const std::string& url) const
  {
    return StringUtils::regexMatch(m_regex, url);
  }

  std::string Redirection::redirectURL() const
  {
    return m_redirectURL;
  }

  Redirection::Type Redirection::type() const
  {
    return m_redirectionType;
  }
};
