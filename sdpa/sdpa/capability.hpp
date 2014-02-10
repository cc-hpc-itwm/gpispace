#ifndef SDPA_CAPABILITY_HPP_
#define SDPA_CAPABILITY_HPP_ 1

#include <string>
#include <set>
#include <iostream>
#include <boost/foreach.hpp>
#include <sdpa/id_generator.hpp>

namespace sdpa
{
namespace
{
  struct cap_id_tag
  {
    static const char *name ()
    {
      return "cap";
    }
  };
}

  // type, name
  class Capability
  {
  public:
    explicit Capability(const std::string& name = "",
                        const std::string& type = "",
                        const std::string& owner = "")
    : name_(name)
    , type_(type)
    , depth_(0)
    , owner_(owner)
    , uuid_(sdpa::id_generator::instance<cap_id_tag>().next())
    {}

    std::string name() const { return name_;}

    size_t depth() const { return depth_;}
    void setDepth(size_t depth) { depth_ = depth;}
    void incDepth() { depth_++; }

    std::string owner() const { return owner_; }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & name_;
      ar & type_;
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
    std::string type_;
    mutable size_t depth_;
    std::string owner_;
    std::string uuid_;
  };

  typedef Capability capability_t;

  typedef std::set<capability_t> capabilities_set_t;
}

#endif
