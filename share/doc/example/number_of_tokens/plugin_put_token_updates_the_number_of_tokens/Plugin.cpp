// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Base.hpp>

#include <stdexcept>

namespace gspc::test::number_of_tokens
{
  struct Plugin : public ::gspc::we::plugin::Base
  {
    Plugin ( gspc::we::plugin::Context const&
           , gspc::we::plugin::PutToken put_token
           )
      : _put_token {std::move (put_token)}
    {}

    void before_eval (gspc::we::plugin::Context const&) override
    {
      throw std::runtime_error {"Unexpected call to 'before_eval'"};
    }

    void after_eval (gspc::we::plugin::Context const& context) override
    {
      _put_token ("value", context.value ({"m"}));
    }

  private:
    gspc::we::plugin::PutToken _put_token;
  };
}

GSPC_WE_PLUGIN_CREATE (gspc::test::number_of_tokens::Plugin)
