// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_STRUCT_HPP
#define _XML_PARSE_TYPE_STRUCT_HPP

#include <we/type/signature.hpp>
#include <we/type/literal/valid_name.hpp>

#include <iostream>
#include <list>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct struct_t
      {
        std::string name;
        signature::desc_t sig;
        boost::filesystem::path path;
      };

      inline bool operator == (const struct_t & a, const struct_t & b)
      {
        return (a.name == b.name) && (a.sig == b.sig);
      }

      inline bool operator != (const struct_t & a, const struct_t & b)
      {
        return !(a == b);
      }

      typedef std::list<struct_t> structs_type;

      namespace dump
      {
        inline void dump (xml_util::xmlstream & s, const struct_t & st)
        {
          boost::apply_visitor ( signature::visitor::dump (st.name, s)
                               , st.sig
                               );
        }
      }
    }

    // ********************************************************************* //

    namespace struct_t
    {
      typedef boost::unordered_map< signature::field_name_t
                                  , type::struct_t
                                  > set_type;
      typedef boost::unordered_map< signature::field_name_t
                                  , std::string
                                  > forbidden_type;

      // ******************************************************************* //

      set_type make (const type::structs_type & structs)
      {
        set_type set;

        for ( type::structs_type::const_iterator pos (structs.begin())
            ; pos != structs.end()
            ; ++pos
            )
          {
            const set_type::const_iterator old (set.find (pos->name));

            if (old != set.end())
              {
                throw error::struct_redefined<type::struct_t>
                  (old->second, *pos);
              }

            set[pos->name] = *pos;
          }

        return set;
      }

      // ******************************************************************* //

      inline set_type join ( const set_type & above
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
            const set_type::const_iterator old (set.find (strct.name));

            if (old != set.end())
              {
                const forbidden_type::const_iterator pos
                  (forbidden.find (strct.name));

                if (pos != forbidden.end())
                  {
                    throw error::forbidden_shadowing<type::struct_t>
                      (old->second, strct, pos->second);
                  }

                if (strct != old->second)
                  {
                    state.warn
                      (warning::struct_shadowed<type::struct_t> ( old->second
                                                                , strct
                                                                )
                      );
                  }
              }

            set[strct.name] = strct;
          }

        return set;
      }

      inline set_type join ( const set_type & above
                           , const set_type & below
                           , const state::type & state
                           )
      {
        return join (above, below, forbidden_type(), state);
      }

      // ******************************************************************* //

      class get_literal_type_name
        : public boost::static_visitor<literal::type_name_t>
      {
      public:
        literal::type_name_t operator () (const literal::type_name_t & t) const
        {
          return t;
        }
        literal::type_name_t operator () (const signature::structured_t &) const
        {
          throw error::strange
            ("try to get a literal typename from a structured type");
        }
      };

      // ******************************************************************* //

      class resolve : public boost::static_visitor<bool>
      {
      private:
        const boost::filesystem::path path;
        const set_type & sig_set;

      public:
        resolve ( const set_type & _sig_set
                , const boost::filesystem::path & _path
                )
          : path (_path)
          , sig_set (_sig_set)
        {}

        bool operator () (literal::type_name_t & t) const
        {
          return literal::valid_name (t);
        }

        bool operator () (signature::structured_t & map) const
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

                  pos->second = res->second.sig;

                  boost::apply_visitor (*this, pos->second);
                }
            }
          return true;
        }
      };

      // ******************************************************************* //

      class specialize : public boost::static_visitor<signature::desc_t>
      {
      private:
        const type::type_map_type & map_in;
        const state::type & state;

      public:
        specialize ( const type::type_map_type & _map_in
                   , const state::type & _state
                   )
          : map_in (_map_in)
          , state (_state)
        {}

        signature::desc_t operator () (literal::type_name_t & t) const
        {
          const type::type_map_type::const_iterator mapped (map_in.find (t));

          return (mapped != map_in.end()) ? mapped->second : t;
        }

        signature::desc_t operator () (signature::structured_t & map) const
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
      };
    }
  }
}

#endif
