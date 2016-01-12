#pragma once

#include <string>
#include <set>
#include <iostream>

namespace sdpa
{
  // type, name
  class Capability
  {
  public:
    Capability() = default;
    explicit Capability(const std::string& name,
                        const std::string& owner);

    std::string name() const { return name_;}

    size_t depth() const { return depth_;}
    void incDepth() { depth_++; }

    std::string owner() const { return owner_; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & name_;
      ar & depth_;
      ar & owner_;
      ar & uuid_;
    }

    bool operator<(const Capability& b) const
    {
      return uuid_ < b.uuid_;
    }

    bool operator==(const Capability& b) const
    {
      // ignore depth
      return uuid_ == b.uuid_;
    }

    private:
    std::string name_;
    size_t depth_;
    std::string owner_;
    std::string uuid_;
  };

  typedef Capability capability_t;

  typedef std::set<capability_t> capabilities_set_t;
}
