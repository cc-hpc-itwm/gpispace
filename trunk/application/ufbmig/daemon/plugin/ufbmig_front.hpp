#ifndef UFBMIG_FRONTEND_HPP
#define UFBMIG_FRONTEND_HPP 1

namespace ufbmig
{
  class Frontend
  {
  public:
    virtual ~Frontend() {}

    virtual int initialize() = 0;
    virtual int calculate() = 0;
    virtual int finalize() = 0;
    virtual int cancel() = 0;

    virtual void initialize_done (int) = 0;
    virtual void calculate_done (int) = 0;
    virtual void finalize_done (int) = 0;
    virtual void cancel_done (int) = 0;
  };
}

#endif
