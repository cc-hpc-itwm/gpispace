#ifndef SEDA_ENCODER_HPP
#define SEDA_ENCODER_HPP

namespace seda { namespace comm {
  class Encoder {
  public:
    template <typename T> static std::string encode(const T &thing) {
      return thing.encode();
    }
  };
}}

#endif
