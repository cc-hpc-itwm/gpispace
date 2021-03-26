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

#include <we/signature_of.hpp>

#include <we/type/value/name_of.hpp>

namespace pnet
{
  namespace
  {
    type::signature::structured_type
      structured (const std::string&, const type::value::structured_type&);

    class visitor_structured
      : public boost::static_visitor<type::signature::field_type>
    {
    public:
      visitor_structured (const std::string& name)
        : _name (name)
      {}

      type::signature::field_type
        operator() (const type::value::structured_type& v) const
      {
        return structured (_name, v);
      }

      template<typename T>
        type::signature::field_type operator() (const T& x) const
      {
        return std::make_pair (_name, type::value::name_of (x));
      }

    private:
      const std::string _name;
    };

    type::signature::structured_type
      structured ( const std::string& name
                 , const type::value::structured_type& v
                 )
    {
      type::signature::structure_type s;

      for (const type::value::structured_type::value_type& f : v)
      {
        s.emplace_back (boost::apply_visitor ( visitor_structured (f.first)
                                             , f.second
                                             )
                       );
      }

      return std::make_pair (name, s);
    }

    class visitor_signature
      : public boost::static_visitor<type::signature::signature_type>
    {
    public:
      type::signature::signature_type
        operator() (const type::value::structured_type& v) const
      {
        return structured ("struct", v);
      }

      template<typename T>
        type::signature::signature_type operator() (const T& x) const
      {
        return type::value::name_of (x);
      }
    };
  }

  type::signature::signature_type
    signature_of (const type::value::value_type& value)
  {
    return boost::apply_visitor (visitor_signature(), value);
  }
}
