#include "core/Interceptor.h"
#include "core/Config.h"

#include "utils/Logger.h"
#include "utils/Server.h"

#include <memory>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
  try {
    std::string config_file;
    uint16_t nb_threads;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("version,v", "print version string")
    ("config,c", po::value<std::string>(&config_file) , "input configuration file")
    ("threads,t", po::value<uint16_t>(&nb_threads), "number of threads to use")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    if (vm.count("version")) {
      std::cout << Http::Server::getName() << " " << Http::Server::getVersion() <<
                std::endl;
      return 1;
    }

    if (!vm.count("config")) {
      std::cout << "missing configuration file " << std::endl;
      return 0;
    }

    Config* config = new Config(config_file);

    if (!vm.count("threads")) {
      nb_threads = config->threads();
    }

    boost::asio::io_service ioService;

    for (const auto& serverConfig : config->serversConfig()) {
      std::shared_ptr<Interceptor> interceptor = std::make_shared<Interceptor>
          (serverConfig, ioService);
      interceptor->init();
    }

    boost::thread_group tg;

    LOG_INFO("using " << nb_threads  << " threads");

    for (unsigned i = 0; i < nb_threads; ++i) {
      tg.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
    }

    tg.join_all();

  } catch (ConfigException& e) {
    LOG_ERROR("Exception raised " << e.what());
  } catch (std::exception& e) {
    LOG_ERROR("Exception raised " << e.what());
  }


}
