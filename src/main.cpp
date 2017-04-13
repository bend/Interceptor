#include "core/Interceptor.h"
#include "core/Config.h"

#include "utils/Logger.h"
#include "cache/generic_cache.h"

#include <memory>
#include <boost/thread.hpp>


int main(int argc, char** argv)
{

  //TODO handle params
  if (argc != 2) {
    return -1;
  }

  try {
    Config* config = new Config(std::string(argv[1]));
    AbstractCacheHandler* cache;
#ifdef ENABLE_LOCAL_CACHE
    cache = new CacheHandler(config->maxCacheSize());
#else
    cache = new BasicCacheHandler();
#endif //ENABLE_LOCAL_CACHE
    boost::asio::io_service ioService;

    for (const auto& serverConfig : config->serversConfig()) {
      std::shared_ptr<Interceptor> interceptor = std::make_shared<Interceptor>
          (serverConfig, cache, ioService);
      interceptor->init();
    }

    boost::thread_group tg;

    for (unsigned i = 0; i < config->threads(); ++i) {
      tg.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
    }

    tg.join_all();

  } catch (ConfigException& e) {
    LOG_ERROR("Exception raised " << e.what());
  }

}
