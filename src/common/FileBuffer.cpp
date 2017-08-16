#include "FileBuffer.h"

#include "InterceptorException.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include <cassert>
#include <sstream>
#include <array>
#include <cstring>

namespace Interceptor {

  FileBuffer::FileBuffer()
    : m_size(0),
	  m_tmpname("/tmp/interceptor_tmp_XXXXXX"),
      m_headersReceived(false),
	  m_headersLength(0)
  {
    m_tmpfile = mkstemp(&m_tmpname[0]);

    if (!m_tmpfile) {
      throw InterceptorException("Could not open temp file");
    }

    m_stream = std::ofstream(m_tmpname, std::ios::binary);

    if (!m_stream.is_open()) {
      throw InterceptorException("Could not open stream to temp file");
    }

    LOG_DEBUG("FileBuffer::FileBuffer() - temp file is  " << m_tmpname);
  }

  FileBuffer::~FileBuffer() noexcept
  {
    LOG_DEBUG("FileBuffer::~FileBuffer()");
    m_stream.close();
    close(m_tmpfile);
    unlink(m_tmpname.c_str());
  }

  void FileBuffer::append(const char* data, size_t size)
  {
    LOG_DEBUG("FileBuffer::append()");
    assert(m_stream.is_open());
    m_stream.write(reinterpret_cast<const char*>(data), size);
    m_stream.flush();
    std::array<const char, 6> chr = {m_lastCharacters[0], m_lastCharacters[1], m_lastCharacters[2],
                                     data[0], data[1], data[2]
                                    };
    int64_t ret1 = -1, ret2 = -1;

    if ((!m_headersReceived
         && ((ret1 = StringUtils::findString(chr.data(), "\r\n\r\n", 6)) > -1))
        || (ret2 = StringUtils::findString(data, "\r\n\r\n", size)) > -1) {
      m_headersReceived = true;
      // set correct header length based on the position of the double CRLF
      m_headersLength = m_size + (ret1 > -1  ?  ret1 - 2 : ret2 + 1);
    }  else  {
      std::memcpy(m_lastCharacters, &data[size - 3], 3);
    }

    m_size += size;
  }

  size_t FileBuffer::size() const
  {
    return m_size;
  }

  std::string FileBuffer::getData() const
  {
    char* data;
    size_t size;
    FileUtils::readFile(m_tmpname, &data, size);

    if ( !data ) {
      throw InterceptorException("Could not read data");
    }

    return std::string(reinterpret_cast<char*>(data));
  }

  std::string FileBuffer::getData(uint64_t from, uint64_t to) const
  {
    std::stringstream stream;
    size_t size;

    if (FileUtils::readFile(m_tmpname, from, to, stream,
                            size) != Http::Code::Ok) {
      throw InterceptorException("Could not read data");
    }

    return stream.str();
  }

  bool FileBuffer::headersReceived() const
  {
    return m_headersReceived;
  }

  std::string FileBuffer::headersData() const
  {
    return getData(0, m_headersLength - 1);
  }

}
