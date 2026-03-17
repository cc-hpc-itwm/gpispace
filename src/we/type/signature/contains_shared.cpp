// Copyright (C) 2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/signature/contains_shared.hpp>
#include <gspc/we/type/shared.hpp>

#include <boost/variant.hpp>

#include <algorithm>

namespace gspc::pnet::type::signature
{
  namespace
  {
    class contains_shared_visitor : public ::boost::static_visitor<bool>
    {
    public:
      bool operator() (std::string const& type_name) const
      {
        return we::type::shared::cleanup_place (type_name).has_value();
      }

      bool operator() (structured_type const& structured) const
      {
        auto const& fields {structured.second};

        return std::any_of
          ( std::cbegin (fields), std::cend (fields)
          , [this] (auto const& field)
            {
              return ::boost::apply_visitor (*this, field);
            }
          );
      }

      bool operator()
        ( std::pair<std::string, std::string> const& field
        ) const
      {
        return this->operator() (field.second);
      }
    };
  }

  bool contains_shared (signature_type const& signature)
  {
    return ::boost::apply_visitor (contains_shared_visitor(), signature);
  }
}
