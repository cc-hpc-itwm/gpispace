// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/struct.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/state.hpp>

#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/specialize.hpp>
#include <we/type/signature/resolve.hpp>
#include <we/type/signature/dump.hpp>

#include <fhg/util/xml.hpp>

#include <boost/unordered_map.hpp>
#include <boost/range/adaptor/map.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      structure_type::structure_type
        ( ID_CONS_PARAM(structure)
        , PARENT_CONS_PARAM(function)
        , const util::position_type& pod
        , const pnet::type::signature::structured_type& sig
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _sig (sig)
      {
        _id_mapper->put (_id, *this);
      }

      const pnet::type::signature::structured_type& structure_type::signature() const
      {
        return _sig;
      }

      namespace
      {
        class visitor_get_name
          : public boost::static_visitor<const std::string&>
        {
        public:
          const std::string&
            operator() (const std::pair< std::string
                                       , pnet::type::signature::structure_type
                                       >& s
                       ) const
          {
            return s.first;
          }
        };
      }
      const std::string& structure_type::name() const
      {
        return boost::apply_visitor (visitor_get_name(), _sig);
      }

      void structure_type::specialize
        (const boost::unordered_map<std::string, std::string>& m)
      {
        pnet::type::signature::specialize (_sig, m);
      }

      namespace
      {
        boost::optional<pnet::type::signature::signature_type> get_assignment
          ( const boost::unordered_map<std::string, structure_type>& m
          , const std::string& key
          )
        {
          boost::unordered_map<std::string, structure_type>::const_iterator
            pos (m.find (key));

          if (pos != m.end())
          {
            return pnet::type::signature::signature_type (pos->second.signature());
          }

          return boost::none;
        }

        class get_struct
          : public boost::static_visitor<pnet::type::signature::structured_type>
        {
        public:
          pnet::type::signature::structured_type operator()
            (const std::string& s) const
          {
            throw std::runtime_error ("expected struct, got " + s);
          }
          pnet::type::signature::structured_type operator()
            (const pnet::type::signature::structured_type& s) const
          {
            return s;
          }
        };
      }

      void structure_type::resolve
        (const boost::unordered_map<std::string, structure_type>& m)
      {
        pnet::type::signature::signature_type sign
          (pnet::type::signature::resolve (_sig
                                          , boost::bind (get_assignment, m, _1)
                                          )
          );

        _sig = boost::apply_visitor (get_struct(), sign);
      }

      id::ref::structure structure_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return structure_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _sig
          ).make_reference_id();
      }

      bool operator == (const structure_type & a, const structure_type & b)
      {
        return (a.name() == b.name()) && (a.signature() == b.signature());
      }

      bool operator != (const structure_type & a, const structure_type & b)
      {
        return !(a == b);
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const structure_type & st
                  )
        {
          pnet::type::signature::dump_to (s, st.signature());
        }
      }
    }

    namespace structure_type_util
    {
      set_type make (const type::structs_type & structs)
      {
        set_type set;

        for ( type::structs_type::const_iterator pos (structs.begin())
            ; pos != structs.end()
            ; ++pos
            )
          {
            const set_type::const_iterator old (set.find (pos->name()));

            if (old != set.end())
              {
                throw error::struct_redefined (old->second, *pos);
              }

            set.insert (std::make_pair (pos->name(), *pos));
          }

        return set;
      }

      set_type join ( const set_type & above
                    , const set_type & below
                    , const forbidden_type & forbidden
                    , const state::type & state
                    )
      {
        set_type set (above);

        for ( set_type::const_iterator pos (below.begin())
            ; pos != below.end()
            ; ++pos
            )
          {
            const type::structure_type& strct (pos->second);
            const set_type::const_iterator old (set.find (strct.name()));

            if (old != set.end() && strct != old->second)
              {
                const forbidden_type::const_iterator forbidden_it
                  (forbidden.find (strct.name()));

                if (forbidden_it != forbidden.end())
                  {
                    throw error::forbidden_shadowing ( old->second
                                                     , strct
                                                     , forbidden_it->second
                                                     );
                  }

                state.warn
                  (warning::struct_shadowed ( old->second
                                            , strct
                                            )
                  );
              }

            set.insert (std::make_pair (strct.name(), strct));
          }

        return set;
      }

      set_type join ( const set_type & above
                    , const set_type & below
                    , const state::type & state
                    )
      {
        return join (above, below, forbidden_type(), state);
      }

      bool struct_by_name ( const std::string& name
                          , const type::structure_type& stru
                          )
      {
        return stru.name() == name;
      }
    }
  }
}
