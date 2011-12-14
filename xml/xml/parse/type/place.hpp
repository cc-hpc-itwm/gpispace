// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <iostream>
#include <string>
#include <list>

#include <xml/parse/type/token.hpp>
#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/error.hpp>

#include <xml/parse/util/weparse.hpp>
#include <xml/parse/util/unique.hpp>

#include <we/type/id.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/literal/default.hpp>

#include <we/type/literal.hpp>
#include <we/type/value.hpp>
#include <we/type/value/require_type.hpp>
#include <we/type/token.hpp>
#include <we/type/error.hpp>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

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
          std::ostringstream s;

          s << "when parsing the value "
            << " of field " << field_name
            << " of place " << place_name
            << " of type " << signature
            << " in " << path
            ;

          const util::we_parser_t parser
            (util::generic_we_parse (value, s.str()));

          try
            {
              state::type::context_t context (state.context());

              const value::type v (parser.eval_all (context));
              const signature::type sig (signature);

              return boost::apply_visitor
                ( value::visitor::require_type (field_name)
                , sig.desc()
                , v
                );
            }
          catch (const expr::exception::eval::divide_by_zero & e)
            {
              throw error::parse_lift (place_name, field_name, path, e.what());
            }
          catch (const expr::exception::eval::type_error & e)
            {
              throw error::parse_lift (place_name, field_name, path, e.what());
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

      typedef std::list<value::type> values_type;
      typedef std::list<token_type> tokens_type;

      struct place_type
      {
      public:
        std::string name;
        std::string type;
        fhg::util::maybe<petri_net::capacity_t> capacity;
        tokens_type tokens;
        values_type values;
        signature::type sig;
        we::type::property::type prop;
        fhg::util::maybe<bool> is_virtual;

        place_type () {}

        place_type ( const std::string & _name
                   , const std::string & _type
                   , const fhg::util::maybe<petri_net::capacity_t> _capacity
                   , const fhg::util::maybe<bool> _is_virtual
                   )
          : name (_name)
          , type (_type)
          , capacity (_capacity)
          , is_virtual (_is_virtual)
        {}

        void push_token (const token_type & t)
        {
          tokens.push_back (t);
        }

        void translate ( const boost::filesystem::path & path
                       , const state::type & state
                       )
        {
          for ( tokens_type::const_iterator tok (tokens.begin())
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

      typedef xml::util::unique<place_type>::elements_type places_type;

      // ******************************************************************* //

      namespace dump
      {
        inline void dump (xml_util::xmlstream & s, const place_type & p)
        {
          s.open ("place");
          s.attr ("name", p.name);
          s.attr ("type", p.type);
          s.attr ("capacity", p.capacity);
          s.attr ("virtual", p.is_virtual);

          ::we::type::property::dump::dump (s, p.prop);

          for ( tokens_type::const_iterator tok (p.tokens.begin())
              ; tok != p.tokens.end()
              ; ++tok
              )
            {
              boost::apply_visitor ( signature::visitor::dump_token ("", s)
                                   , *tok
                                   );
            }

          s.close();
        }
      } // namespace dump
    }
  }
}

#endif
