#include "FileBuffer.h"

#include "common/InterceptorException.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include <cassert>
#include <sstream>

FileBuffer::FileBuffer()
  : m_size(0),
  m_headersReceived(false)
{
  m_tmpfile = std::tmpfile();
  if(!m_tmpfile)
    throw InterceptorException("Could not open temp file");

  m_stream = std::ofstream(std::to_string(fileno(m_tmpfile)), std::ios::binary);

  if (m_stream.is_open()) {
    throw InterceptorException("Could not open stream to temp file");
  }
}

FileBuffer::~FileBuffer() noexcept
{
  if(m_stream)
	std::fclose(m_tmpfile);
  m_stream.close();
}

void FileBuffer::append(const unsigned char* data, size_t size)
{
  assert(m_stream.is_open());
  m_stream.write(reinterpret_cast<const char*>(data), size);
  m_size += size;
  if(!((m_headersReceived && m_lastcharIsCarriage && data[0] == '\n')
	  || StringUtils::containsString(data, "\r\n", size)))
	m_headersReceived = true;
  else if(data[size - 1] == '\r')
	m_lastcharIsCarriage = true;
  else 
	m_lastcharIsCarriage = false;

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
