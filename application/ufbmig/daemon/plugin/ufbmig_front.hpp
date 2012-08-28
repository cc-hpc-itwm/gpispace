#ifndef UFBMIG_FRONTEND_HPP
#define UFBMIG_FRONTEND_HPP 1

#include <string>

namespace ufbmig
{
  class Frontend
  {
  public:
    virtual ~Frontend() {}

    virtual int prepare_backend () = 0;
    virtual int initialize(std::string const &) = 0;
    virtual int update_salt_mask(const char *data, size_t len) = 0;
    virtual int calculate(std::string const &) = 0;
    virtual int finalize() = 0;
    virtual int cancel() = 0;

    virtual void prepare_backend_done (int, std::string const & msg) = 0;
    virtual void initialize_done (int, std::string const & msg) = 0;
    virtual void salt_mask_done (int, std::string const & msg) = 0;
    virtual void calculate_done (int, std::string const &msg) = 0;
    virtual void finalize_done (int, std::string const & msg) = 0;

    virtual void update_progress (int) = 0;
  };
}

#endif
