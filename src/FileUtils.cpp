#include "FileUtils.h"
#include "Logger.h"

#include <fstream>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <sys/stat.h>

bool FileUtils::readFile(const std::string& filename, unsigned char** data, size_t& pageLength)
{
  namespace fs = boost::filesystem;
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
  fs::path dataDir(filename);

  if (!fs::exists(dataDir) || fs::is_directory(dataDir)) {
    return false;
  }

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
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

  std::ifstream::pos_type pos = ifs.tellg();
  pageLength = pos;
  ifs.seekg(0, std::ios::beg);
  stream << ifs.rdbuf();
  ifs.close();
  return true;
}

bool FileUtils::readFile(const std::string& filename, const std::tuple<int64_t, int64_t>& bytes, std::stringstream& stream, std::vector<uint64_t>& sizes)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

  if ( !ifs.is_open() ) {
    trace("error") << "Could not open file";
    return false;
  }

  int64_t from = std::get<0>(bytes);
  int64_t to   = std::get<1>(bytes);

  std::ifstream::pos_type pos = ifs.tellg();

  if (from  == -1) {
    if (to == -1) {
      return false;  //Invalid request //TODO throw BAD request
    }

    from = pos - to;
    to = pos;
  } else if (to == -1) {
    to = (int)pos - 1;
  }

  ifs.seekg(from, std::ios::beg);
  char* buffer = new char[to - from + 1];
  ifs.read(buffer, to - from + 1);
  ifs.close();

  stream.write(buffer, to - from + 1);

  delete [] buffer;

  sizes.push_back(from);
  sizes.push_back(to);
  sizes.push_back(pos);

  return true;
}

bool FileUtils::fileSize(const std::string& filename, size_t& bytes)
{
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

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
  if (path.find(".html") != std::string::npos) {
    return "text/html";
  }

  if (path.find(".css") != std::string::npos) {
    return "text/css";
  }

  if (path.find(".png") != std::string::npos) {
    return "image/png";
  }

  if (path.find(".jpg") != std::string::npos) {
    return "image/jpeg";
  }

  if (path.find(".mp4") != std::string::npos) {
    return "video/mp4";
  }

  if (path.find(".ogg") != std::string::npos) {
    return "video/ogg";
  }

  if (path.find(".js") != std::string::npos) {
    return "application/javascript";
  } else {
    return "other";
  }
}

std::string FileUtils::extension(const std::string& filename)
{
  size_t pos = filename.rfind(".");

  if (pos == std::string::npos) {
    return "";
  }

  return filename.substr(pos + 1);
}

bool FileUtils::exists(const std::string& filename)
{
  namespace fs = boost::filesystem;
  fs::path dataDir(filename);

  if (!fs::exists(dataDir) || fs::is_directory(dataDir)) {
    return false;
  }

  return true;
}

std::tuple<std::string, std::string> FileUtils::generateCacheData(const std::string& path)
{
  struct stat st;

  if (stat(path.c_str(), &st) == 0) {
	char eTag[20];
    sprintf(eTag, "%d%d-%d", (int)st.st_ino, (int)st.st_mtime, (int)st.st_size);
    time_t lastMT = st.st_mtime;
    std::stringstream sstr;
    sstr << std::put_time(std::gmtime(&lastMT), "%a, %d %b %Y %T %Z");
    return std::make_tuple(eTag, sstr.str());
  }

  return {};
}

