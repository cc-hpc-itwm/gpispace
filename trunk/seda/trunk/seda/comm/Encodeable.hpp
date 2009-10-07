#ifndef SEDA_COMM_ENCODEABLE_HPP
#define SEDA_COMM_ENCODEABLE_HPP 1

#include <string>
#include <seda/comm/SedaCommError.hpp>

namespace seda {
namespace comm {
    class EncodingError : public SedaCommError {
    public:
        explicit
        EncodingError(const std::string &reason="error while encoding event")
        : SedaCommError(reason) {}

        virtual ~EncodingError() throw() {}
    };

    class Encodeable {
    public:
        virtual const std::string &encode() const throw(EncodingError) = 0;
    };
}}

#endif
