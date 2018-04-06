#include "Main.h"
#include "common/Params.h"
#include "core/Server.h"
#include "core/Config.h"

#include "utils/Logger.h"
#include "utils/ServerInfo.h"
#include "cache/generic_cache.h"
#include "backend/BackendsPool.cpp"
#include "modules/ModulesLoader.h"
#include "authentication/AuthenticationLoader.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <csignal>
#include <functional>
#include <memory>

namespace po = boost::program_options;

namespace Interceptor {

  Main::Main()
    : m_cacheHandler(nullptr),
#ifdef ENABLE_LOCAL_CACHE
      m_subject(nullptr),
      m_monitor(nullptr),
#endif // ENABLE_LOCAL_CACHE
      m_config(nullptr),
      m_pool(nullptr),
      m_nbThreads(0)
  {}


  Main::~Main()
  {
    LOG_DEBUG("Main::~Main()");
    delete m_config;
#ifdef ENABLE_LOCAL_CACHE
    delete m_subject;
    delete m_monitor;
#endif // ENABLE_LOCAL_CACHE
    delete m_cacheHandler;
    delete m_pool;
    delete m_auths;
  }

  bool Main::init(int argc, char** argv)
  {
    if (!parsePO(argc, argv)) {
      return false;
    }

    if (!initCache()) {
      return false;
    }

    if (!initBackendsPool()) {
      return false;
    }

    if (!initModules()) {
      return false;
    }

    if (!initAuthentication()) {
      return false;
    }

    return true;
  }

  bool Main::reinit()
  {
    LOG_DEBUG("Main::reinit()");

    if (!initCache()) {
      return false;
    }

    if (!initBackendsPool()) {
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
    ("config,c", po::value<std::string>(&config_file), "input configuration file")
    ("threads,t", po::value<uint16_t>(&m_nbThreads), "number of threads to use");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return false;
    }

    if (vm.count("version")) {
      std::cout << ServerInfo::name() << " " << ServerInfo::version() <<
                " (build " << ServerInfo::build() << " - " << ServerInfo::buildDate() << ")" <<
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
    m_subject = new Cache::Subject();
    m_cacheHandler = new Cache::CacheHandler(m_config->maxCacheSize(), *m_subject);
    m_monitor = new Cache::CacheMonitor(*m_subject);
    Cache::CacheListener* cacheListener = new Cache::CacheListener(
      dynamic_cast<Cache::CacheHandler*>
      (m_cacheHandler));
    Cache::MonitorListener* monitorListener = new Cache::MonitorListener(m_monitor);
    m_subject->addListener(cacheListener);
    m_subject->addListener(monitorListener);
    m_monitor->start();

#else
    m_cacheHandler = new Cache::BasicCacheHandler();
#endif //ENABLE_LOCAL_CACHE
    return true;
  }

  bool Main::initBackendsPool()
  {
    m_pool = new Backends::BackendsPool(m_ioService);

    std::vector<BackendCPtr> backends;

    for (auto& kv : m_config->backends()) {
      backends.push_back(std::const_pointer_cast<const Backends::Backend>(kv.second));
    }

    for (auto& kv : m_config->connectors()) {
      backends.push_back(std::const_pointer_cast<const Backends::Connector>
                         (kv.second));
    }

    return m_pool->initPool(backends);
  }

  bool Main::initModules()
  {
    m_modules = new Modules::ModulesLoader();
    std::vector<ModuleCPtr> modules;

    for (auto& kv : m_config->modules()) {
      modules.push_back(std::const_pointer_cast<const Modules::Module>
                        (kv.second));
    }

    return m_modules->loadModules(modules);
  }

  bool Main::initAuthentication()
  {
    m_auths = new Authentication::AuthenticationLoader();
    std::vector<AuthenticationCPtr> auths;

    for (auto& kv : m_config->authentications()) {
      auths.push_back(std::const_pointer_cast<const Authentication::Authentication>
                      (kv.second));
    }

    return m_auths->loadAuthentications(auths);
  }

  void Main::run()
  {
    LOG_DEBUG("Main::run()");


    for (const auto& serverConfig : m_config->serversConfig()) {
      ParamsPtr params = std::make_shared<Params>(serverConfig, m_cacheHandler,
                         m_pool, m_modules, m_auths);
      std::shared_ptr<Server> interceptor = std::make_shared<Server>(params,
                                            m_ioService);
      interceptor->init();
      m_servers.push_back(interceptor);
    }

    LOG_DEBUG("CWD is "  << boost::filesystem::current_path());
    LOG_INFO("using " << m_nbThreads  << " threads");
    std::vector<std::thread> threads;

    for (unsigned i = 0; i < m_nbThreads; ++i) {
      std::packaged_task<void()> task([ = ] {
        m_ioService.run();
      });

      m_futures.push_back(task.get_future());
      threads.push_back(std::thread(std::move(task)));
    }

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
  }

  void Main::stop()
  {
    LOG_DEBUG("Main::stop()");
    m_servers.clear();
    m_ioService.stop();
    m_futures.front().wait();
    LOG_DEBUG("Stopped");
  }

}

std::unique_ptr<Interceptor::Main> mainHandler;

void signalHandler(int signum)
{
  using namespace Interceptor;

  switch (signum) {
    case SIGINT:
      LOG_INFO("Stopping Server ...");
      mainHandler->stop();
      break;

    case SIGHUP:
      LOG_INFO("Reloading configuration...");
      LOG_ERROR("Not yet implemented");
      break;
  }
}


int main(int argc, char** argv)
{

  using namespace Interceptor;
  signal(SIGINT, signalHandler);

  signal(SIGHUP, signalHandler);

  try {
    mainHandler = std::make_unique<Main>();

    if (!mainHandler->init(argc, argv)) {
      return EXIT_FAILURE;
    }

    mainHandler->run();

  } catch (ConfigException& e) {
    LOG_ERROR("Configuration error " << e.what());
  } catch (std::exception& e) {
    LOG_ERROR("Exception raised " << e.what());
  }

  return EXIT_SUCCESS;
}
