#ifndef SEDA_COMM_DECODEABLE_HPP
#define SEDA_COMM_DECODEABLE_HPP 1

#include <string>
#include <seda/comm/SedaCommError.hpp>

namespace seda {
namespace comm {
    class DecodingError : public SedaCommError {
    public:
        explicit
        DecodingError(const std::string &a_reason="error while decoding event")
        : SedaCommError(a_reason) {}

        virtual ~DecodingError() throw() {}
    };

    class Decodeable {
    public:
        virtual void decode(const std::string&) throw(DecodingError) = 0;
        virtual ~Decodeable() {}
    };
}}

#endif
