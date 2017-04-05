#ifndef ABSTRACT_CACHE_HANDLER_H__
#define ABSTRACT_CACHE_HANDLER_H__

class AbstractCacheHandler : public std::shared_from_this<AbstractCacheHandler> {

  public:
  AbstractCacheHandler(size_t maxCacheSize)
	: m_maxCacheSize(maxCacheSize) {};

  ~AbstractCacheHandler() = default;

  virtual const std::string& eTag(const std::string& file) = 0;

  virtual const std::string& lastModified(const std::string& file) = 0;
  
  virtual const size_t size(const std::string& file) = 0;

  protected:
  virtual bool cacheSize() const = 0;

  protected:
	size_t m_maxCacheSize;

};

#endif // ABSTRACT_CACHE_HANDLER_H__
