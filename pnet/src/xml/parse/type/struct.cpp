// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/struct.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/state.hpp>

#include <we2/type/signature/is_literal.hpp>
#include <we2/type/signature/specialize.hpp>

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
        , const signature::desc_t& sig
        , const pnet::type::signature::structured_type& sig2
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _sig (sig)
        , _sig2 (sig2)
      {
        _id_mapper->put (_id, *this);
      }

      const signature::desc_t& structure_type::signature() const
      {
        return _sig;
      }
      signature::desc_t& structure_type::signature()
      {
        return _sig;
      }
      const signature::desc_t& structure_type::signature (const signature::desc_t& sig)
      {
        return _sig = sig;
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
        return boost::apply_visitor (visitor_get_name(), _sig2);
      }

      namespace
      {
        class visitor_specialize
          : public boost::static_visitor<signature::desc_t>
        {
        private:
          const type::type_map_type& map_in;

        public:
          visitor_specialize (const type::type_map_type& _map_in)
            : map_in (_map_in)
          {}

          signature::desc_t operator() (std::string& t) const
          {
            const type::type_map_type::const_iterator mapped (map_in.find (t));

            return (mapped != map_in.end()) ? mapped->second : t;
          }

          signature::desc_t operator() (signature::structured_t& map) const
          {
            for ( signature::structured_t::map_t::iterator pos (map.begin())
                ; pos != map.end()
                ; ++pos
                )
            {
              const type::type_map_type::const_iterator mapped
                (map_in.find (pos->first));

              pos->second = (mapped != map_in.end())
                          ? mapped->second
                          : boost::apply_visitor (*this, pos->second);
            }

            return map;
          }
        };
      }

      void structure_type::specialize
        (const boost::unordered_map<std::string, std::string>& m)
      {
        _sig = boost::apply_visitor (visitor_specialize (m), _sig);
        pnet::type::signature::specialize (_sig2, m);
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
          , _sig2
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
          boost::apply_visitor ( signature::visitor::dump (st.name(), s)
                               , st.signature()
                               );
        }
      }
    }

    namespace structure_type
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

      // ******************************************************************* //

      std::string get_literal_type_name::operator () (const std::string & t) const
      {
        return t;
      }
      std::string get_literal_type_name::operator () (const signature::structured_t &) const
      {
        throw error::strange
          ("try to get a literal typename from a structured type");
      }

      // ******************************************************************* //

      namespace
      {
        class resolve_with_fun_visitor : public boost::static_visitor<bool>
        {
        public:
          resolve_with_fun_visitor
            ( const resolving_function_type& resolving_function
            , const type::structure_type& strct
            )
              : _struct (strct)
              , _resolving_function (resolving_function)
          {}

          bool operator() (std::string& t) const
          {
            return pnet::type::signature::is_literal (t);
          }

          bool operator() (signature::structured_t& map) const
          {
            BOOST_FOREACH (signature::structured_t::map_t::value_type& sub, map)
            {
              if (!boost::apply_visitor (*this, sub.second))
              {
                const std::string child_name
                  ( boost::apply_visitor
                    (parse::structure_type::get_literal_type_name(), sub.second)
                  );

                const boost::optional<signature::type> res
                  (_resolving_function (child_name));

                if (!res)
                {
                  throw error::cannot_resolve (sub.first, child_name, _struct);
                }

                sub.second = res->desc();

                boost::apply_visitor (*this, sub.second);
              }
            }

            return true;
          }

        private:
          const type::structure_type _struct;
          const resolving_function_type& _resolving_function;
        };
      }

      signature::desc_t resolve_with_fun
        ( const type::structure_type& unresolved_struct
        , resolving_function_type resolving_function
        )
      {
        signature::desc_t sig (unresolved_struct.signature());
        boost::apply_visitor
          ( resolve_with_fun_visitor ( resolving_function
                                     , unresolved_struct
                                     )
          , sig
          );
        return sig;
      }

      bool struct_by_name ( const std::string& name
                          , const type::structure_type& stru
                          )
      {
        return stru.name() == name;
      }

      resolve::resolve ( const set_type & _sig_set
                       , const type::structure_type& strct
                       )
        : _struct (strct)
        , sig_set (_sig_set)
      {}

      bool resolve::operator () (std::string & t) const
      {
        return pnet::type::signature::is_literal (t);
      }

      bool resolve::operator () (signature::structured_t & map) const
      {
        for ( signature::structured_t::map_t::iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
        {
          const bool resolved (boost::apply_visitor (*this, pos->second));

          if (!resolved)
          {
            const std::string child_name
              (boost::apply_visitor ( get_literal_type_name()
                                    , pos->second
                                    )
              );

            set_type::const_iterator res (sig_set.find (child_name));

            if (res == sig_set.end())
            {
              throw error::cannot_resolve (pos->first, child_name, _struct);
            }

            pos->second = res->second.signature();

            boost::apply_visitor (*this, pos->second);
          }
        }
        return true;
      }
    }
  }
}
