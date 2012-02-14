#ifndef SDPA_UUID_GEN_HPP
#define SDPA_UUID_GEN_HPP 1

#include <sdpa/uuid.hpp>

namespace sdpa {
  class uuidgen {
  public:
    void generate(uuid &uid);
    void operator()(uuid &uid){
      return generate(uid);
    }
    uuid operator()();
  };
}

#endif
