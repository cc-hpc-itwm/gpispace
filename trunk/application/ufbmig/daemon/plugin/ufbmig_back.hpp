#ifndef UFBMIG_BACKEND_HPP
#define UFBMIG_BACKEND_HPP 1

#include <string>

namespace ufbmig
{
  class Frontend;
  class Backend
  {
  public:
    virtual ~Backend() {}

    virtual int initialize(std::string const &) = 0;
    virtual int calculate() = 0;
    virtual int finalize() = 0;
    virtual int cancel() = 0;
    virtual void registerFrontend(Frontend*) = 0;
  };
}

#endif
