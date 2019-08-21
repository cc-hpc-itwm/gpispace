#include <we/plugin/Base.hpp>

#include <utility>

struct A : public ::gspc::we::plugin::Base
{
  GSPC_WE_PLUGIN_CONSTRUCTOR (A,,put_token)
    : _put_token (std::move (put_token))
  {}

  virtual void before_eval (::gspc::we::plugin::Context const& context) override
  {
    return after_eval (context);
  }
  virtual void after_eval (::gspc::we::plugin::Context const& context) override
  {
    return _put_token ("A", context.value ({"A"}));
  }

private:
  ::gspc::we::plugin::PutToken _put_token;
};

GSPC_WE_PLUGIN_CREATE (A)
