#include "Main.h"
#include "core/Interceptor.h"
#include "core/Config.h"

#include "utils/Logger.h"
#include "cache/generic_cache.h"
#include "utils/Server.h"

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;

Main::Main()
  : m_cacheHandler(nullptr),
    m_subject(nullptr),
    m_monitor(nullptr),
    m_config(nullptr),
	m_nbThreads(0)
{}


Main::~Main()
{
  delete m_config;
  delete m_monitor;
  delete m_subject;
  delete m_cacheHandler;
}

bool Main::init(int argc, char** argv)
{
  if (!parsePO(argc, argv)) {
    return false;
  }

  if (!initCache()) {
    return false;
  }

  return true;
}

bool Main::parsePO(int argc, char** argv)
{
  std::string config_file;
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help", "produce help message")
  ("version,v", "print version string")
  ("config,c", po::value<std::string>(&config_file) , "input configuration file")
  ("threads,t", po::value<uint16_t>(&m_nbThreads), "number of threads to use");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return false;
  }

  if (vm.count("version")) {
    std::cout << Http::Server::getName() << " " << Http::Server::getVersion() <<
              std::endl;
    return false;
  }

  if (!vm.count("config")) {
    std::cout << "missing configuration file " << std::endl;
    return false;
  }

  m_config = new Config(config_file);

  if (!vm.count("threads")) {
    m_nbThreads = m_config->threads();
  }

  return true;
}

bool Main::initCache()
{
#ifdef ENABLE_LOCAL_CACHE
  m_subject = new Subject();
  m_cacheHandler = new CacheHandler(m_config->maxCacheSize(), *m_subject);
  m_monitor = new CacheMonitor(*m_subject);
  CacheListener* cacheListener = new CacheListener(dynamic_cast<CacheHandler*>
      (m_cacheHandler));
  MonitorListener* monitorListener = new MonitorListener(m_monitor);
  m_subject->addListener(cacheListener);
  m_subject->addListener(monitorListener);
  m_monitor->start();

#else
  m_cacheHandler = new BasicCacheHandler();
#endif //ENABLE_LOCAL_CACHE
  return true;
}

void Main::run()
{
  boost::asio::io_service ioService;

  for (const auto& serverConfig : m_config->serversConfig()) {
    std::shared_ptr<Interceptor> interceptor = std::make_shared<Interceptor>
        (serverConfig, m_cacheHandler, ioService);
    interceptor->init();
  }

  boost::thread_group tg;
  LOG_DEBUG("Current cwd is "  << boost::filesystem::current_path());
  LOG_INFO("using " << m_nbThreads  << " threads");

  for (unsigned i = 0; i < m_nbThreads; ++i) {
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
  }

  tg.join_all();

}


int main(int argc, char** argv)
{
  try {

    Main main;

    if (!main.init(argc, argv)) {
      return 1;
    }

    main.run();

  } catch (ConfigException& e) {
    LOG_ERROR("Exception raised " << e.what());
  } catch (std::exception& e) {
    LOG_ERROR("Exception raised " << e.what());
  }


}
