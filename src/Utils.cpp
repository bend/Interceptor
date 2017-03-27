#include "Utils.h"
#include "Logger.h"

#include <fstream>
#include <boost/filesystem.hpp>

bool Utils::readFile(const std::string& filename, unsigned char** data, size_t& pageLength)
{
  namespace fs = boost::filesystem;
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
  fs::path dataDir(filename);
  if (!fs::exists(dataDir) || fs::is_directory(dataDir))
    return false;
  if ( !ifs.is_open() ) {
    trace("error") << "Could not open file";
    return false;
  }
  std::ifstream::pos_type pos = ifs.tellg();
  pageLength = pos;
  *data = new unsigned char[pageLength]();
  ifs.seekg(0, std::ios::beg);
  ifs.read(reinterpret_cast<char*>(*data), pageLength);
  ifs.close();
  return true;
}

std::string Utils::getMimeType(const std::string& path)
{
  if (path.find(".html") != std::string::npos)
    return "text/html";
  if (path.find(".css") != std::string::npos)
    return "text/css";
  if (path.find(".png") != std::string::npos)
    return "image/png";
  if (path.find(".jpg") != std::string::npos)
    return "image/jpeg";
  if (path.find(".js") != std::string::npos)
    return "application/javascript";
  else return "other";
}

