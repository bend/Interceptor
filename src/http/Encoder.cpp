#include "Encoder.h"

#include "common/Buffer.h"
#include "utils/Logger.h"

#include <sstream>

namespace Interceptor::Http {

#ifdef ENABLE_GZIP
  bool Encoder::initGzip()
  {
    LOG_DEBUG("CommonReply::initGzip()");
    m_gzip.zalloc = Z_NULL;
    m_gzip.zfree = Z_NULL;
    m_gzip.opaque = Z_NULL;
    m_gzip.next_in = Z_NULL;
    int r = 0;

    r = deflateInit2(&m_gzip, Z_DEFAULT_COMPRESSION,
                     Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

    m_gzipBusy = true;
    assert(r == Z_OK);
    return true;
  }

  bool Encoder::encode(BufferPtr buffer,
                       std::vector<boost::asio::const_buffer>& buffers, bool last, size_t& bytes)
  {
    LOG_DEBUG("CommonReply::encodeResponse()");
    std::vector<boost::asio::const_buffer> result;

    if (!m_gzipBusy) {
      initGzip();
    }

    unsigned int i = 0;

    for (auto& buf : buffers) {

      m_gzip.avail_in = boost::asio::buffer_size(buf);
      m_gzip.next_in = (unsigned char*) boost::asio::detail::buffer_cast_helper(
                         buf);

      char out[16 * 1024];

      do {
        m_gzip.next_out = (unsigned char*)out;
        m_gzip.avail_out = sizeof(out);

        int res = 0;
        res = deflate(&m_gzip,
                      (i == buffers.size() - 1 && last) ?
                      Z_FINISH : Z_NO_FLUSH);
        assert(res != Z_STREAM_ERROR);

        unsigned have = sizeof(out) - m_gzip.avail_out;
        bytes += have;

        if (have) {
          result.push_back(buffer->buf(std::string((char*)out, have)));
        }
      } while (m_gzip.avail_out == 0);

      ++i;

    }

    if (last) {
      deflateEnd(&m_gzip);
      m_gzipBusy = false;
    }

    buffers.clear();
    buffers.insert(buffers.begin(), result.begin(), result.end());

    return true;

  }
#endif // ENABLE_GZIP

  bool Encoder::chunk(BufferPtr buffer,
                      std::vector<boost::asio::const_buffer>& buffers, bool last)
  {
    LOG_DEBUG("CommonReply::chunkResponse()");
    size_t size = 0;

    // We cannot take the total Content Length here because the gzip compression changed that
    // Length, so we need to recalculate it
    for (auto& buffer : buffers) {
      size += boost::asio::buffer_size(buffer);
    }

    std::stringstream stream;
    stream << std::hex << size << "\r\n";

    char* header = new char[stream.str().length()]();
    memcpy(header, stream.str().data(), stream.str().length());

    buffers.insert(buffers.begin(), buffer->buf(header,
                   stream.str().length()));

    stream.str("\r\n");
    char* crlf = new char[stream.str().length()]();
    memcpy(crlf, stream.str().data(), stream.str().length());
    buffers.push_back(buffer->buf(crlf, stream.str().length()));

    if (last) {
      stream.str("0\r\n\r\n");
      char* footer = new char[stream.str().length()]();
      memcpy(footer, stream.str().data(), stream.str().length());
      buffers.push_back(buffer->buf(footer, stream.str().length()));
    }

    return true;

  }

}


