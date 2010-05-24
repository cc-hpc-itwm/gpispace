#ifndef SDPA_UUID_GEN_HPP
#define SDPA_UUID_GEN_HPP 1

#include <sdpa/uuid.hpp>

namespace sdpa {
  enum uuid_version_t {
    uuid_default,
    uuid_random,
    uuid_time
  };

  class uuidgen {
  public:
    void generate(uuid &uid, uuid_version_t version = uuid_default);
    void operator()(uuid &uid, uuid_version_t version = uuid_default){
      return generate(uid, version);
    }
  };
}

#endif
