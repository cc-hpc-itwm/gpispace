// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/struct.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

#include <we/type/signature/dump.hpp>
#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/resolve.hpp>
#include <we/type/signature/specialize.hpp>

#include <fhg/util/xml.hpp>

#include <boost/range/adaptor/map.hpp>

#include <functional>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      structure_type::structure_type
        ( util::position_type const& pod
        , pnet::type::signature::structured_type const& sig
        )
        : with_position_of_definition (pod)
        , _sig (sig)
      {}

      pnet::type::signature::structured_type const& structure_type::signature() const
      {
        return _sig;
      }

      std::string const& structure_type::name() const
      {
        return _sig.first;
      }

      void structure_type::specialize
        (std::unordered_map<std::string, std::string> const& m)
      {
        pnet::type::signature::specialize (_sig, m);
      }

      namespace
      {
        ::boost::optional<pnet::type::signature::signature_type> get_assignment
          ( std::unordered_map<std::string, structure_type> const& m
          , std::string const& key
          )
        {
          auto
            pos (m.find (key));

          if (pos != m.end())
          {
            return pnet::type::signature::signature_type (pos->second.signature());
          }

          return ::boost::none;
        }

        class get_struct
          : public ::boost::static_visitor<pnet::type::signature::structured_type>
        {
        public:
          pnet::type::signature::structured_type operator()
            (std::string const& s) const
          {
            throw std::runtime_error ("expected struct, got " + s);
          }
          pnet::type::signature::structured_type operator()
            (pnet::type::signature::structured_type const& s) const
          {
            return s;
          }
        };
      }

      void structure_type::resolve
        (std::unordered_map<std::string, structure_type> const& m)
      {
        pnet::type::signature::signature_type sign
          (pnet::type::signature::resolve
            (_sig, std::bind (get_assignment, m, std::placeholders::_1))
          );

        _sig = ::boost::apply_visitor (get_struct(), sign);
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , structure_type const& st
                  )
        {
          pnet::type::signature::dump_to (s, st.signature());
        }
      }
    }

    namespace structure_type_util
    {
      set_type make ( type::structs_type const& structs
                    , xml::parse::state::type const& state
                    )
      {
        set_type set;

        for (auto const& pos : structs)
          {
            const set_type::const_iterator old (set.find (pos.name()));

            if (old != set.end())
              {
                if (old->second.signature() == pos.signature())
                {
                  state.warn (warning::struct_redefined (old->second, pos));
                }
                else
                {
                  throw error::struct_redefined (old->second, pos);
                }
              }

            set.emplace (pos.name(), pos);
          }

        return set;
      }

      set_type join (set_type const& above, set_type const& below)
      {
        set_type joined (above);

        for ( xml::parse::type::structure_type const& strct
            : below | ::boost::adaptors::map_values
            )
        {
          set_type::const_iterator const old (joined.find (strct.name()));

          if (old != joined.end()
             && !(old->second.signature() == strct.signature())
             )
          {
            throw error::struct_redefined (old->second, strct);
          }

          joined.emplace (strct.name(), strct);
        }

        return joined;
      }
    }
  }
}
