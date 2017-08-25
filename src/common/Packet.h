#ifndef PACKET_H__
#define PACKET_H__

namespace Interceptor {

  struct Packet {

    Packet(char* data, size_t len)
      : m_data(data),
        m_size(len)
    {}

    ~Packet()
    {
      delete[] m_data;
    }

    char* m_data;
    size_t m_size;
  };

}

#endif //PACKET_H__
