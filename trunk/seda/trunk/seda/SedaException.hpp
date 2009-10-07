#ifndef SEDA_EXCEPTION_HPP
#define SEDA_EXCEPTION_HPP 1

#include <exception>
#include <string>

namespace seda {
  class SedaException : public std::exception {
  public:
    explicit
    SedaException(const std::string& reason) :
      _reason(reason) {}
    virtual ~SedaException() throw() {}
    virtual const char* what() const throw() { return reason().c_str(); }
    virtual const std::string& reason() const { return _reason; }

  protected:
    std::string _reason;
  };
}

#endif // !SEDA_EXCEPTION_HPP
