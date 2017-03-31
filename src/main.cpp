#include "core/Interceptor.h"
#include "Logger.h"
#include "Config.h"

#include <memory>


int main(int argc, char** argv)
{

  if (argc != 2)
    return -1;

  try {
    Config* config = new Config(std::string(argv[1]));
    boost::asio::io_service ioService;

    for (const auto& serverConfig : config->serversConfig()) {
      std::shared_ptr<Interceptor> interceptor = std::make_shared<Interceptor>(serverConfig, ioService);
      interceptor->init();
    }

    boost::asio::io_service::work work(ioService);
    ioService.run();

  } catch (ConfigException& e) {
    trace("error") << "Exception raised " << e.what();
  }

}
