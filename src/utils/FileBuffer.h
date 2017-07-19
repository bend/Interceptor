#ifndef FILE_BUFFER_H__
#define FILE_BUFFER_H__

#include <iostream>
#include <fstream>

class FileBuffer {

public:
  FileBuffer();
  ~FileBuffer() noexcept;

  void append(const char* data, size_t size);

  size_t size() const;

  std::string getData() const;

  std::string getData(uint64_t from, uint64_t to) const;

  bool headersReceived() const;

  std::string headersData() const;

private:
  size_t m_size;
  int m_tmpfile;
  char* m_tmpname;
  std::ofstream m_stream;
  bool m_headersReceived;
  size_t m_headersLength;
  char m_lastCharacters[3];
};

#endif
