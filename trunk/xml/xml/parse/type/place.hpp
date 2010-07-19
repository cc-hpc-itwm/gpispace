// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <xml/parse/type/token.hpp>
#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/error.hpp>

#include <xml/parse/util/maybe.hpp>
#include <xml/parse/util/weparse.hpp>

#include <we/type/id.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <we/type/literal.hpp>
#include <we/type/value.hpp>
#include <we/type/token.hpp>
#include <we/type/error.hpp>

#include <we/expr/parse/position.hpp>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      class default_construct_value : public boost::static_visitor<value::type>
      {
      public:
        value::type
        operator () (const literal::type_name_t & type_name) const
        {
          return literal::of_type (type_name);
        }

        value::type
        operator () (const signature::structured_t & signature) const
        {
          value::structured_t val;

          for ( signature::structured_t::const_iterator sig (signature.begin())
              ; sig != signature.end()
              ; ++sig
              )
            {
              const signature::field_name_t field (sig->first);
              const signature::desc_t desc (sig->second);

              val[field] = boost::apply_visitor (*this, desc);
            }

          return val;
        }
      };

      // ******************************************************************* //

      // binary visiting
      class construct_value : public boost::static_visitor<value::type>
      {
      private:
        const std::string & place_name;
        const boost::filesystem::path & path;
        const signature::field_name_t field_name;
        const state::type & state;
        
      public:
        construct_value ( const std::string & _place_name
                        , const boost::filesystem::path & _path
                        , const signature::field_name_t & _field_name
                        , const state::type & _state
                        )
          : place_name (_place_name)
          , path (_path)
          , field_name (_field_name)
          , state (_state)
        {}
        
        value::type operator () ( const literal::type_name_t & signature
                                , const literal::type_name_t & value
                                ) const
        {
          literal::type val;

          unsigned int k (0);
          std::string::const_iterator pos (value.begin());
          const std::string::const_iterator end (value.end());
          expr::parse::position parse_pos (k, pos, end);

          try
            {
              literal::read (val, parse_pos);
            }
          catch (const expr::exception::parse::exception & e)
            {
              const std::string nice (util::format_parse_error (value, e));

              throw error::parse_lift (place_name, field_name, path, nice);
            }

          if (!parse_pos.end())
            {
              throw error::parse_incomplete
                ( place_name, field_name, signature
                , value, parse_pos.rest(), path
                );
            }

          try
            {
              return literal::require_type (field_name, signature, val);
            }
          catch (const ::type::error & e)
            {
              throw error::parse_lift (place_name, field_name, path, e.what());
            }
        }

        value::type operator () ( const signature::structured_t & signature
                                , const signature::structured_t & value
                                ) const
        {
          value::structured_t val;

          for ( signature::structured_t::const_iterator sig (signature.begin())
              ; sig != signature.end()
              ; ++sig
              )
            {
              const signature::field_name_t field (sig->first);
              const signature::desc_t desc (sig->second);
              const std::string field_deeper
                ((field_name == "") ? field : (field_name + "." + field));

              if (value.has_field (field))
                {
                  val[field] = boost::apply_visitor
                    ( construct_value (place_name, path, field_deeper, state)
                    , desc
                    , value.field(field)
                    );
                }
              else
                {
                  state.warn (warning::default_construction ( place_name
                                                            , field_deeper
                                                            , path
                                                            )
                             );

                  val[field] = boost::apply_visitor
                    (default_construct_value(), desc);
                }
            }

          if (state.Wunused_field())
            {
              for ( signature::structured_t::const_iterator pos (value.begin())
                  ; pos != value.end()
                  ; ++pos
                  )
                {
                  const signature::field_name_t field (pos->first);
                  const std::string field_deeper
                    ((field_name == "") ? field : (field_name + "." + field));

                  if (!signature.has_field (field))
                    {
                      state.warn (warning::unused_field ( place_name
                                                        , field_deeper
                                                        , path
                                                        )
                                 );
                    }
                }
            }

          return val;
        }

        template<typename SIG, typename VAL>
        value::type operator () ( const SIG & signature
                                , const VAL & value
                                ) const
        {
          throw error::parse_type_mismatch 
            (place_name, field_name, signature, value, path);
        }
      };

      // ******************************************************************* //

      typedef std::vector<value::type> value_vec_type;

      struct place_type
      {
      public:
        std::string name;
        std::string type;
        maybe<petri_net::capacity_t> capacity;
        std::vector<token_type> tokens;
        value_vec_type values;
        signature::type sig;
        int level;
        we::type::property::type prop;

        place_type () 
          : name (), type (), capacity (), tokens (), values(), sig(), level ()
        {}

        place_type ( const std::string & _name
                   , const std::string & _type
                   , const maybe<petri_net::capacity_t> _capacity
                   )
          : name (_name)
          , type (_type)
          , capacity (_capacity)
          , tokens ()
          , values ()
          , sig ()
          , level ()
        {}

        void push_token (const token_type & t)
        {
          tokens.push_back (t);
        }

        void translate ( const boost::filesystem::path & path
                       , const state::type & state
                       )
        {
          for ( std::vector<token_type>::const_iterator tok (tokens.begin())
              ; tok != tokens.end()
              ; ++tok
              )
            {
              values.push_back 
                (boost::apply_visitor ( construct_value (name, path, "", state)
                                      , sig.desc()
                                      , *tok
                                      )
                );
           }
        }

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        )
        {
          const type::type_map_type::const_iterator
            mapped (map_in.find (type));

          if (mapped != map_in.end())
            {
              type = mapped->second;
            }
        }
      };

      typedef std::vector<place_type> place_vec_type;

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const place_type & p)
      {
        s << level(p.level)  << "place (" << std::endl;
        s << level(p.level+1) << "name = " << p.name << std::endl;
        s << level(p.level+1) << "type = " << p.type << std::endl;
        s << level(p.level+1) << "sig = " << p.sig << std::endl;
        s << level(p.level+1) << "capacity = " << p.capacity << std::endl;

        for ( std::vector<token_type>::const_iterator tok (p.tokens.begin())
            ; tok != p.tokens.end()
            ; ++tok
            )
          {
            s << level(p.level+1) << "token = " << *tok << std::endl;
          }

        for ( std::vector<value::type>::const_iterator val (p.values.begin())
            ; val != p.values.end()
            ; ++val
            )
          {
            s << level(p.level+1) << "value = " << *val << std::endl;
          }

        s << level (p.level+1) << "properties = " << std::endl;

        p.prop.writeTo (s, p.level+2);

        return s << level(p.level) << ") // place";
      }
    }
  }
}

#endif
