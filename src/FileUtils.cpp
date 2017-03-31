#include "FileUtils.h"
#include "Logger.h"

#include <fstream>
#include <boost/filesystem.hpp>

bool FileUtils::readFile(const std::string& filename, unsigned char** data, size_t& pageLength)
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

bool FileUtils::readFile(const std::string& filename, std::stringstream& stream, size_t& pageLength)
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
  ifs.seekg(0, std::ios::beg);
  stream << ifs.rdbuf();
  ifs.close();
  return true;

}

bool FileUtils::fileSize(const std::string& filename, size_t& bytes)
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
  bytes = pos;
  ifs.close();
  return true;
}

std::string FileUtils::mimeType(const std::string& path)
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

std::string FileUtils::extension(const std::string& filename)
{
  size_t pos = filename.rfind(".");

  if (pos == std::string::npos)
    return "";

  return filename.substr(pos + 1);
}

