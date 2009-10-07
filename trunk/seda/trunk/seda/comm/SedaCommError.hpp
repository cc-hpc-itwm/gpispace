#ifndef SEDA_COMM_ERROR_HPP
#define SEDA_COMM_ERROR_HPP 1

#include <seda/SedaException.hpp>

namespace seda {
namespace comm {
    class SedaCommError : public seda::SedaException {
    public:
        explicit
        SedaCommError(const std::string &errorMessage)
            : seda::SedaException(errorMessage) {}

        virtual ~SedaCommError() throw () {}
    };
}}

#endif
