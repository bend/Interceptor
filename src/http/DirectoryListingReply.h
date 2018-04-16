#ifndef DIRECTORY_LISTING_REPLY_H__
#define DIRECTORY_LISTING_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class DirectoryListingReply : public CommonReply {
  public:
    DirectoryListingReply(HttpRequestPtr request, const SiteConfig* site,
                          const std::string& path);
    virtual BufferPtr buildReply() override;
    size_t size();

  protected:
    virtual void serialize(std::stringstream& stream) override {};

  private:
    std::string m_path;

  };

}

#endif // DIRECTORY_LISTING_REPLY_H__
