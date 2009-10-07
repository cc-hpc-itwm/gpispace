#ifndef SEDA_ERROR_EVENT_HPP
#define SEDA_ERROR_EVENT_HPP 1

#include <string>
#include <exception>

#include <seda/SystemEvent.hpp>

namespace seda 
{
    class ErrorEvent : public SystemEvent {
    public:
        ErrorEvent(const std::exception& ex);
        ErrorEvent(const std::string& reason);

        const std::exception& error() const;
    private:
        std::exception _ex;
    };    
}

#endif // ! SEDA_ERROR_EVENT
