#include <we/plugin/Base.hpp>

#include <util-generic/unreachable.hpp>

#include <stdexcept>

struct D : public gspc::we::plugin::Base
{
  GSPC_WE_PLUGIN_CONSTRUCTOR (D,,)
  {
    throw std::runtime_error ("D::D()");
  }

  virtual void before_eval (::gspc::we::plugin::Context const&) override
  {
    FHG_UTIL_UNREACHABLE();
  }
  virtual void after_eval (::gspc::we::plugin::Context const&) override
  {
    FHG_UTIL_UNREACHABLE();
  }
};

GSPC_WE_PLUGIN_CREATE (D)
