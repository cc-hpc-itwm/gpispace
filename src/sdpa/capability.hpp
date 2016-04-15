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
    Capability with_increased_depth() const
    {
      return {name_, depth_ + 1, owner_, uuid_};
    }

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
    Capability ( std::string name
               , size_t depth
               , std::string owner
               , std::string uuid
               )
      : name_ (std::move (name))
      , depth_ (depth)
      , owner_ (std::move (owner))
      , uuid_ (std::move (uuid))
    {}

    std::string name_;
    size_t depth_;
    std::string owner_;
    std::string uuid_;
  };

  typedef Capability capability_t;

  typedef std::set<capability_t> capabilities_set_t;
}
