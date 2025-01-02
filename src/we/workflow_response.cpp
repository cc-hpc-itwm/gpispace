// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/workflow_response.hpp>

#include <we/type/value/peek_or_die.hpp>
#include <we/type/value/poke.hpp>

#include <algorithm>

namespace we
{
  namespace
  {
    class is_field : public ::boost::static_visitor<bool>
    {
    public:
      is_field (std::string const& name, std::string const& type)
        : _name (name)
        , _type (type)
      {}
      bool operator() (std::pair<std::string, std::string> const& field) const
      {
        return field.first == _name && field.second == _type;
      }
      template<typename T>
        bool operator() (T const&) const
      {
        return false;
      }

    private:
      std::string const _name;
      std::string const _type;
    };

    class has_field : public ::boost::static_visitor<bool>
    {
    public:
      has_field (std::string const& name, std::string const& type)
        : _name (name)
        , _type (type)
      {}
      bool operator() (std::string const&) const
      {
        return false;
      }
      bool operator() (pnet::type::signature::structured_type const& s) const
      {
        return std::any_of
          ( s.second.begin(), s.second.end()
          , [this] (pnet::type::signature::field_type const& field)
            {
              return ::boost::apply_visitor (is_field (_name, _type), field);
            }
          );
      }

    private:
      std::string const _name;
      std::string const _type;
    };
  }

  bool is_response_description
    (pnet::type::signature::signature_type const& signature)
  {
    return ::boost::apply_visitor (has_field ("response_id", "string"), signature);
  }

  std::string get_response_id (pnet::type::value::value_type const& description)
  {
    return pnet::type::value::peek_or_die<std::string>
      (description, {"response_id"});
  }
  pnet::type::value::value_type make_response_description
    (std::string response_id, pnet::type::value::value_type const& value)
  {
    pnet::type::value::value_type description;
    pnet::type::value::poke ("value", description, value);
    pnet::type::value::poke ("response_id", description, response_id);
    return description;
  }
}
