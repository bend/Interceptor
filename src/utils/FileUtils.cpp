#include "FileUtils.h"
#include "Logger.h"

#include <fstream>
#include <iomanip>

#include <boost/filesystem.hpp>
#include <sys/stat.h>

namespace Interceptor::FileUtils {

  Http::Code readFile(const std::string& filename, char** data,
                      size_t& pageLength)
  {
    namespace fs = boost::filesystem;
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    fs::path dataDir(filename);

    if (!fs::exists(dataDir) || fs::is_directory(dataDir)) {
      return Http::Code::NotFound;
    }

    if ( !ifs.is_open() ) {
      LOG_ERROR("Could not open file");
      return Http::Code::NotFound;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    pageLength = pos;

    try {
      *data = new char[pageLength]();
    } catch (std::bad_alloc) {
      ifs.close();
      return Http::Code::BadRequest;
    }

    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(*data), pageLength);
    ifs.close();

    return Http::Code::Ok;
  }

  Http::Code readFile(const std::string& filename, std::stringstream& stream,
                      size_t& pageLength)
  {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

    if ( !ifs.is_open() ) {
      LOG_ERROR("Could not open file");
      return Http::Code::NotFound;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    pageLength = pos;
    ifs.seekg(0, std::ios::beg);
    stream << ifs.rdbuf();
    ifs.close();
    return Http::Code::Ok;
  }

  Http::Code readFile(const std::string& filename, size_t from, size_t to,
                      std::stringstream& stream, size_t& fileSize)
  {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    if ( !ifs.is_open() ) {
      LOG_ERROR("Could not open file");
      return Http::Code::NotFound;
    }

    if (to > (size_t)pos - 1) {
      ifs.close();
      return Http::Code::BadRequest;
    }

    ifs.seekg(from, std::ios::beg);
    char* buffer = nullptr;

    try {
      buffer = new char[to - from + 1];
    } catch (std::bad_alloc) {
      ifs.close();
      return Http::Code::RequestRangeNotSatisfiable;
    }

    ifs.read(buffer, to - from + 1);
    ifs.close();

    fileSize = pos;

    stream.write(buffer, to - from + 1);

    delete [] buffer;

    return Http::Code::Ok;
  }

  Http::Code calculateBounds(const std::string& filename, int64_t& from,
                             int64_t& to)
  {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

    if ( !ifs.is_open() ) {
      LOG_ERROR("Could not open file");
      return Http::Code::NotFound;
    }


    std::ifstream::pos_type pos = ifs.tellg();

    if (from  == -1) {
      if (to == -1) {
        ifs.close();
        return Http::Code::RequestRangeNotSatisfiable;
      }

      from = pos - to;
      to = pos;
    } else if (to == -1) {
      to = (int)pos - 1;
    }

    ifs.close();

    return Http::Code::Ok;
  }


  bool fileSize(const std::string& filename, size_t& bytes)
  {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

    if ( !ifs.is_open() ) {
      LOG_ERROR("Could not open file");
      return false;
    }

    std::ifstream::pos_type pos = ifs.tellg();
    bytes = pos;
    ifs.close();
    return true;
  }

  std::string mimeType(const std::string& path)
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

  std::string extension(const std::string& filename)
  {
    size_t pos = filename.rfind(".");

    if (pos == std::string::npos) {
      return "";
    }

    return filename.substr(pos + 1);
  }

  bool exists(const std::string& filename)
  {
    namespace fs = boost::filesystem;
    fs::path dataDir(filename);

    if (!fs::exists(dataDir) || fs::is_directory(dataDir)) {
      return false;
    }

    return true;
  }

  std::tuple<std::string, std::string> generateCacheData(
    const std::string& path)
  {
    struct stat st;

    if (stat(path.c_str(), &st) == 0) {
      char eTag[60];
      sprintf(eTag, "%d%d-%d", (int)st.st_ino, (int)st.st_mtime, (int)st.st_size);
      time_t lastMT = st.st_mtime;
      std::stringstream sstr;
      sstr << std::put_time(std::gmtime(&lastMT), "%a, %d %b %Y %T %Z");
      return std::make_tuple(eTag, sstr.str());
    }

    return {};
  }

  std::time_t lastModified(const std::string& path)
  {
    struct stat st;
	if(stat(path.c_str(), &st) == 0) {
	  return st.st_mtime;
	}
	return {};
  }

}

