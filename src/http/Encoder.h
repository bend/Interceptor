#ifndef ENCODER_H__
#define ENCODER_H__

#include "vars.h"

#include "common/defs.h"

#ifdef ENABLE_GZIP
#include <zlib.h>
#endif // ENABLE_GZIP

#include <boost/asio/buffer.hpp>


namespace Interceptor::Http {

  class Encoder {

  public:

#ifdef ENABLE_GZIP
    bool encode(BufferPtr buffer, std::vector<boost::asio::const_buffer>& buffers,
                bool last, size_t& bytes);
#endif // ENABLE_GZIP

    bool chunk(BufferPtr buffer, std::vector<boost::asio::const_buffer>& buffers,
               bool last);

  private:
#ifdef ENABLE_GZIP
    bool initGzip();
#endif // ENABLE_GZIP

  private:
#ifdef ENABLE_GZIP
    z_stream m_gzip;
#endif // ENABLE_GZIP
    bool m_gzipBusy;


  };


};

#endif // ENCODER_H__
