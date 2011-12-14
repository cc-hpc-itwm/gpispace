#ifndef UFBMIG_BACKEND_HPP
#define UFBMIG_BACKEND_HPP 1

#include <stdint.h>
#include <string>

namespace ufbmig
{
  class Frontend;
  class Backend
  {
  public:
    virtual ~Backend() {}

    virtual int initialize(std::string const &) = 0;
    virtual int update_salt_mask(const char *data, size_t len) = 0;
    virtual int calculate(std::string const &xml) = 0;
    virtual int finalize() = 0;
    virtual int cancel() = 0;
    virtual void registerFrontend(Frontend*) = 0;
    virtual int stop () = 0;

    // TODO move to stream interface
    virtual int open (std::string const &) = 0; // current paths: meta, output
    virtual int close (int) = 0;
    virtual int seek (const int, const uint64_t off, const int whence, uint64_t * o) = 0;
    virtual int read (int, char *buffer, size_t len, size_t & num_read) = 0;
    virtual int write (int, const char *buffer, size_t len, size_t & num_written) = 0;
  };
}

#endif
