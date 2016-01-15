#pragma once

#include <algorithm>
#include <string>
#include <set>

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

  inline std::set<std::string> get_set_of_capability_names
    (capabilities_set_t const& cpb_set)
  {
    std::set<std::string> capability_names;
    std::transform ( cpb_set.begin()
                   , cpb_set.end()
                   , std::inserter ( capability_names
                                   , capability_names.begin()
                                   )
                   , [] (capability_t const& cpb)
                     {return cpb.name();}
                   );

    return capability_names;
  }
}
