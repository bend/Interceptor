#ifndef ABSTRACT_DATABASE_H__
#define ABSTRACT_DATABASE_H__

#include <map>

namespace Interceptor::Cache {

  class AbstractDatabase {
  public:
    virtual void purge(const std::string& path) = 0;
    virtual size_t size() const = 0;

  protected:
    template <class C>
    using MetaDataMap = std::map<std::string, C>;

  };

}

#endif
