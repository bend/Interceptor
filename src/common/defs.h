#ifndef DEFS_H__

#include <memory>

#define MAX_CHUNK_SIZE 1024 * 1024

namespace Interceptor {

  class Params;
  class Buffer;

  typedef std::shared_ptr<Buffer> BufferPtr;
  typedef std::shared_ptr<Params> ParamsPtr;


}

#endif //DEFS_H__
