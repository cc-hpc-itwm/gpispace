#ifndef SDPA_SDPAEXCEPTION_HPP
#define SDPA_SDPAEXCEPTION_HPP 1

#include <string>
#include <exception>

namespace sdpa
{
    class SDPAException : public std::exception {
    public:
        explicit
        SDPAException(const std::string& a_reason)
            : _reason(a_reason) {}
        virtual ~SDPAException() throw() {}

        virtual const char* what() const throw() { return reason().c_str(); }
        virtual const std::string& reason() const { return _reason; }

    private:
        std::string _reason;
    };    
}

#endif // ! SDPA_SDPAEXCEPTION
