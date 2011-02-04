#include <fhglog/minimal.hpp>
#include <gpi-space/pc/proto/message.hpp>

#include <string>

int main ()
{
  // use stdin / out for communication
  {
    std::string msg;
    std::getline (std::cin, msg);
    std::cout << msg;
  }
}
