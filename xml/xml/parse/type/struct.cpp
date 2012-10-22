// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/struct.hpp>

#include <fhg/util/xml.hpp>

#include <boost/unordered_map.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct_t::struct_t ( const id::structure& id
                         , const id::function& parent
                         , const std::string& name
                         , const signature::desc_t& sig
                         , const boost::filesystem::path& path
                         )
        : _id (id)
        , _parent (parent)
        , _name (name)
        , _sig (sig)
        , _path (path)
      { }

      const id::structure& struct_t::id() const
      {
        return _id;
      }
      const id::function& struct_t::parent() const
      {
        return _parent;
      }

      const signature::desc_t& struct_t::signature() const
      {
        return _sig;
      }
      signature::desc_t& struct_t::signature()
      {
        return _sig;
      }
      const signature::desc_t& struct_t::signature (const signature::desc_t& sig)
      {
        return _sig = sig;
      }

      const std::string& struct_t::name() const
      {
        return _name;
      }
      const std::string& struct_t::name (const std::string& name)
      {
        return _name = name;
      }

      const boost::filesystem::path& struct_t::path() const
      {
        return _path;
      }

      bool operator == (const struct_t & a, const struct_t & b)
      {
        return (a.name() == b.name()) && (a.signature() == b.signature());
      }

      bool operator != (const struct_t & a, const struct_t & b)
      {
        return !(a == b);
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const struct_t & st
                  )
        {
          boost::apply_visitor ( signature::visitor::dump (st.name(), s)
                               , st.signature()
                               );
        }
      }
    }

    namespace struct_t
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
                throw error::struct_redefined<type::struct_t>
                  (old->second, *pos);
              }

            set.insert (std::make_pair (pos->name(),  *pos));
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
            const type::struct_t & strct (pos->second);
            const set_type::const_iterator old (set.find (strct.name()));

            if (old != set.end() && strct != old->second)
              {
                const forbidden_type::const_iterator pos
                  (forbidden.find (strct.name()));

                if (pos != forbidden.end())
                  {
                    throw error::forbidden_shadowing<type::struct_t>
                      (old->second, strct, pos->second);
                  }

                state.warn
                  (warning::struct_shadowed<type::struct_t> ( old->second
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

      literal::type_name_t get_literal_type_name::operator () (const literal::type_name_t & t) const
      {
        return t;
      }
      literal::type_name_t get_literal_type_name::operator () (const signature::structured_t &) const
      {
        throw error::strange
          ("try to get a literal typename from a structured type");
      }

      // ******************************************************************* //

      resolve::resolve ( const set_type & _sig_set
                       , const boost::filesystem::path & _path
                       )
        : path (_path)
        , sig_set (_sig_set)
      {}

      bool resolve::operator () (literal::type_name_t & t) const
      {
        return literal::valid_name (t);
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
            const literal::type_name_t child_name
              (boost::apply_visitor ( get_literal_type_name()
                                    , pos->second
                                    )
              );

            set_type::const_iterator res (sig_set.find (child_name));

            if (res == sig_set.end())
            {
              throw error::cannot_resolve
                (pos->first, child_name, path);
            }

            pos->second = res->second.signature();

            boost::apply_visitor (*this, pos->second);
          }
        }
        return true;
      }

      // ******************************************************************* //

      specialize::specialize ( const type::type_map_type & _map_in
                             , const state::type & _state
                             )
        : map_in (_map_in)
        , state (_state)
      {}

      signature::desc_t specialize::operator () (literal::type_name_t & t) const
      {
        const type::type_map_type::const_iterator mapped (map_in.find (t));

        return (mapped != map_in.end()) ? mapped->second : t;
      }

      signature::desc_t specialize::operator () (signature::structured_t & map) const
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
            : boost::apply_visitor (*this, pos->second)
            ;
        }

          return map;
      }
    }
  }
}
