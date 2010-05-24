#include "uuidgen.hpp"
#include <uuid/uuid.h>

using namespace sdpa;

void uuidgen::generate(sdpa::uuid &uid, sdpa::uuid_version_t version) {
  switch (version) {
    case sdpa::uuid_random:
      uuid_generate_random(uid.data());
      break;
    case sdpa::uuid_time:
      uuid_generate_time(uid.data());
      break;
    default:
      uuid_generate(uid.data());
      break;
  }
}
