// Copyright (C) 2012-2014,2016,2018,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/struct.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.hpp>

#include <gspc/we/type/signature/dump.hpp>
#include <gspc/we/type/signature/is_literal.hpp>
#include <gspc/we/type/signature/resolve.hpp>
#include <gspc/we/type/signature/specialize.hpp>

#include <gspc/util/xml.hpp>

#include <functional>
#include <unordered_map>
#include <optional>


  namespace gspc::xml::parse
  {
    namespace type
    {
      structure_type::structure_type
        ( util::position_type const& pod
        , gspc::pnet::type::signature::structured_type const& sig
        )
        : with_position_of_definition (pod)
        , _sig (sig)
      {}

      gspc::pnet::type::signature::structured_type const& structure_type::signature() const
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
        gspc::pnet::type::signature::specialize (_sig, m);
      }

      namespace
      {
        std::optional<gspc::pnet::type::signature::signature_type> get_assignment
          ( std::unordered_map<std::string, structure_type> const& m
          , std::string const& key
          )
        {
          auto
            pos (m.find (key));

          if (pos != m.end())
          {
            return gspc::pnet::type::signature::signature_type (pos->second.signature());
          }

          return {};
        }

        class get_struct
          : public ::boost::static_visitor<gspc::pnet::type::signature::structured_type>
        {
        public:
          gspc::pnet::type::signature::structured_type operator()
            (std::string const& s) const
          {
            throw std::runtime_error ("expected struct, got " + s);
          }
          gspc::pnet::type::signature::structured_type operator()
            (gspc::pnet::type::signature::structured_type const& s) const
          {
            return s;
          }
        };
      }

      void structure_type::resolve
        (std::unordered_map<std::string, structure_type> const& m)
      {
        gspc::pnet::type::signature::signature_type sign
          (gspc::pnet::type::signature::resolve
            (_sig, std::bind (get_assignment, m, std::placeholders::_1))
          );

        _sig = ::boost::apply_visitor (get_struct(), sign);
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , structure_type const& st
                  )
        {
          gspc::pnet::type::signature::dump_to (s, st.signature());
        }
      }
    }

    namespace structure_type_util
    {
      set_type make ( type::structs_type const& structs
                    , gspc::xml::parse::state::type const& state
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

        for (auto const& [_ignore, strct] : below)
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
