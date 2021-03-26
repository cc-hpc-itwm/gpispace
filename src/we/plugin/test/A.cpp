// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/plugin/Base.hpp>

#include <utility>

struct A : public ::gspc::we::plugin::Base
{
  A ( ::gspc::we::plugin::Context const&
    , ::gspc::we::plugin::PutToken put_token
    )
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
