#include "core/Interceptor.h"
#include "core/Config.h"

#include "utils/Logger.h"

#include <memory>
#include <boost/thread.hpp>


int main(int argc, char** argv)
{
  if (argc != 2) {
    return -1;
  }

  try {
    Config* config = new Config(std::string(argv[1]));
    boost::asio::io_service ioService;

    for (const auto& serverConfig : config->serversConfig()) {
      std::shared_ptr<Interceptor> interceptor = std::make_shared<Interceptor>(serverConfig, ioService);
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
