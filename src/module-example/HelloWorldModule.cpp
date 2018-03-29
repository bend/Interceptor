#include "HelloWorldModule.h"
#include <iostream>
#include "common/Buffer.h"

HelloWorld::HelloWorld()
{
  std::cout << "Im running now" << std::endl;
}

void HelloWorld::handleRequest(
  std::function<void(Interceptor::BufferPtr)> callback)
{
  Interceptor::BufferPtr buf = std::make_shared<Interceptor::Buffer>();
  buf->m_flags |= Interceptor::Buffer::Closing;
  buf->m_buffers.push_back(buf->buf("Hello world, this module is working\r\n"));
  callback(buf);
}
