#ifndef SDPA_UUID_HPP
#define SDPA_UUID_HPP 1

#include <string>

namespace sdpa {
  class uuid {
  public:
    typedef unsigned char uuid_t[16];

    uuid(const uuid &other);

    explicit
    uuid(const uuid_t &data);

    uuid();

    ~uuid();

    const uuid &operator=(const uuid &rhs);

    const uuid_t &data() const { return uuid_; }
    uuid_t &data() { dirty_ = true; return uuid_; }

    const std::string &str() const;
  private:
    void set(const uuid_t &data);

    uuid_t uuid_;
    mutable bool dirty_;
    mutable std::string str_tmp_;
  };
}

#endif
