#include "Interceptor.h"
#include "Logger.h"

#include <memory>


int main(int argc, char** argv) 
{
  std::unique_ptr<Interceptor> interceptor = std::make_unique<Interceptor>(8000);
  interceptor->init();
}
