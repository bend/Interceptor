#include "FileBuffer.h"

#include "common/InterceptorException.h"
#include "utils/Logger.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include <cassert>
#include <sstream>
#include <array>
#include <cstring>

FileBuffer::FileBuffer()
  : m_size(0),
  m_headersReceived(false)
{
  m_tmpfile = std::tmpfile();
  if(!m_tmpfile)
    throw InterceptorException("Could not open temp file");

  m_stream = std::ofstream(std::to_string(fileno(m_tmpfile)), std::ios::binary);

  if (!m_stream.is_open()) {
    throw InterceptorException("Could not open stream to temp file");
  }

  LOG_DEBUG("FileBuffer::FileBuffer() - temp file is  " << (std::to_string(fileno(m_tmpfile))));
}

FileBuffer::~FileBuffer() noexcept
{
  LOG_DEBUG("FileBuffer::~FileBuffer()");
  m_stream.close();
  if(m_tmpfile) 
	std::fclose(m_tmpfile);
  std::remove(std::to_string(fileno(m_tmpfile)).c_str());
}

void FileBuffer::append(const unsigned char* data, size_t size)
{
  assert(m_stream.is_open());
  m_stream.write(reinterpret_cast<const char*>(data), size);
  m_stream.flush();
  m_size += size;
  std::array<const unsigned char, 6> chr = {m_lastCharacters[0], m_lastCharacters[1], m_lastCharacters[2],
							  data[0], data[1], data[2]};

  if(!m_headersReceived && (StringUtils::containsString(chr.data(), "\r\n\r\n", 6)
	  || StringUtils::containsString(data, "\r\n\r\n", size))) {
	m_headersReceived = true;
	m_headersLength = m_size; //TODO fixme
  }  else  {
	std::memcpy(m_lastCharacters, &data[size - 3], 3);
  }
}

size_t FileBuffer::size() const 
{
  return m_size;
}

std::string FileBuffer::getData() const
{
  unsigned char* data;
  size_t size;
  FileUtils::readFile(std::to_string(fileno(m_tmpfile)), &data, size);

  if ( !data ) {
    throw InterceptorException("Could not read data");
  }

  return std::string(reinterpret_cast<char*>(data));
}

std::string FileBuffer::getData(uint64_t from, uint64_t to) const
{
  std::stringstream stream;
  size_t size;

  if (FileUtils::readFile(std::to_string(fileno(m_tmpfile)), from, to, stream, size) != Http::Code::Ok) {
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
