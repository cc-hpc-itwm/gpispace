// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_WEAVER_TV_HPP
#define _FHG_PNETE_WEAVER_TV_HPP 1

class QStandardItem;
class QString;

#include <string>
#include <stack>

#include <fhg/util/maybe.hpp>

#include <pnete/weaver/weaver.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      class tv
      {
      public:
        explicit tv (QStandardItem * root);

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        std::stack<QStandardItem *> _stack;

        inline void assert_nonempty () const;
        inline void assert_empty () const;

        void push (QStandardItem * x);
        void pop ();
        QStandardItem * top ();

        void set_text (const QString & str);
        void set_text (const std::string & str);
        void add_something (const std::string & sep, const std::string & what);
        void add_type (const std::string & type);
        void add_value(const std::string & value);
        template<typename T>
        void append_key_value ( const std::string & key
                              , const std::string & fmt
                              , const T & val
                              );
        template<typename T>
        void append_maybe_key_value ( const std::string & key
                                    , const std::string & fmt
                                    , const fhg::util::maybe<T> & val
                                    );
        void append_maybe_bool ( const std::string & key
                               , const fhg::util::maybe<bool> & val
                               );

        template<typename T> QStandardItem * append (const T & x);

        template<typename IT>
        void xs
        ( const QString & header
        , IT pos
        , const IT & end
        , void (*fun) ( tv *
                      , const typename std::iterator_traits<IT>::value_type &
                      )
        );

        template<typename Coll>
        void xs
        ( const std::string & header
        , const Coll & coll
        , void (*fun) ( tv *
                      , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                      )
        );
      };

      WSIG(tv, transition::name, std::string, name);
      WSIGE(tv, transition::close);
      WSIG(tv, transition::priority, MAYBE(petri_net::prio_t), prio);
      WSIG(tv, transition::internal, MAYBE(bool), internal);
      WSIG(tv, transition::_inline, MAYBE(bool), _inline);
      WSIG(tv, transition::properties, WETYPE(property::type), prop);
      WSIG(tv, transition::structs, XMLTYPE(structs_type), structs);
      WSIG(tv, transition::function, XMLTYPE(transition_type::function_or_use_type), fun);
      WSIG(tv, transition::place_map, XMLTYPE(place_maps_type), pm);
      WSIG(tv, transition::connect_read, XMLTYPE(connections_type), cs);
      WSIG(tv, transition::connect_in, XMLTYPE(connections_type), cs);
      WSIG(tv, transition::connect_out, XMLTYPE(connections_type), cs);
      WSIG(tv, transition::condition, XMLTYPE(conditions_type), cond);
      WSIG(tv, place::open, ITVAL(XMLTYPE(net_type::places_type)), place);
      WSIGE(tv, place::close);
      WSIG(tv, place::name, std::string, name);
      WSIG(tv, place::type, std::string, type);
      WSIG(tv, place::is_virtual, MAYBE(bool), is_virtual);
      WSIG(tv, place::token, ITVAL(XMLTYPE(tokens_type)), token);
      WSIG(tv, place::properties, WETYPE(property::type), prop);
      WSIG(tv, properties::open, WETYPE(property::type), props);
      WSIG(tv, property::open, WETYPE(property::key_type), key);
      WSIGE(tv, property::close);
      WSIG(tv, property::value, WETYPE(property::value_type), val);
      WSIG(tv, _struct::open, std::string, name);
      WSIGE(tv, _struct::close);
      WSIG(tv, _struct::type, ::literal::type_name_t, type);
      WSIG(tv, structs::open, XMLTYPE(structs_type), structs);
      WSIG(tv, port::open, ITVAL(XMLTYPE(ports_type)), port);
      WSIGE(tv, port::close);
      WSIG(tv, port::name, std::string, name);
      WSIG(tv, port::type, std::string, type);
      WSIG(tv, port::place, MAYBE(std::string), place);
      WSIG(tv, port::properties, WETYPE(property::type), prop);
      WSIG(tv, cinclude::open, ITVAL(XMLTYPE(cincludes_type)), cinclude);
      WSIG(tv, ldflag::open, ITVAL(XMLTYPE(flags_type)), flag);
      WSIG(tv, expression_sequence::line, std::string, line);
      WSIG(tv, type_get::open, ITVAL(XMLTYPE(type_get_type)), tg);
      WSIG(tv, type_map::open, ITVAL(XMLTYPE(type_map_type)), tm);
      WSIG(tv, specialize::open, ITVAL(XMLTYPE(specializes_type)), specialize);
      WSIGE(tv, specialize::close);
      WSIG(tv, specialize::name, std::string, name);
      WSIG(tv, specialize::use, std::string, use);
      WSIG(tv, specialize::type_map, XMLTYPE(type_map_type), tm);
      WSIG(tv, specialize::type_get, XMLTYPE(type_get_type), tg);
      WSIG(tv, conditions::open, XMLTYPE(conditions_type), cs);
      WSIG(tv, function::open, ITVAL(XMLTYPE(functions_type)), fun);
      WSIGE(tv, function::close);
      WSIG(tv, function::name, MAYBE(std::string), name);
      WSIG(tv, function::internal, MAYBE(bool), internal);
      WSIG(tv, function::require, XMLTYPE(requirements_type), reqs);
      WSIG(tv, function::properties, WETYPE(property::type), prop);
      WSIG(tv, function::structs, XMLTYPE(structs_type), structs);
      WSIG(tv, function::in, XMLTYPE(ports_type), ports);
      WSIG(tv, function::out, XMLTYPE(ports_type), ports);
      WSIG(tv, function::fun, XMLTYPE(function_type::type), fun);
      WSIG(tv, function::conditions, XMLTYPE(conditions_type), cs);
      WSIG(tv, tmpl::open, ITVAL(XMLTYPE(templates_type)), t);
      WSIG(tv, tmpl::name, MAYBE(std::string), name);
      WSIG(tv, tmpl::template_parameter, XMLTYPE(template_type::names_type), templates);
      WSIG(tv, tmpl::function, XMLTYPE(function_type), fun);
      WSIGE(tv, tmpl::close);
      WSIG(tv, template_parameter::open, ITVAL(XMLTYPE(template_type::names_type)), template_parameter);
      WSIG(tv, place_map::open, ITVAL(XMLTYPE(place_maps_type)), pm);
      WSIGE(tv, place_map::close);
      WSIG(tv, place_map::place_virtual, std::string, name);
      WSIG(tv, place_map::place_real, std::string, name);
      WSIG(tv, place_map::properties, WETYPE(property::type), prop);
      WSIG( tv
          , connection::open
          , ITVAL(XMLTYPE(connections_type))
          , connection
          );
      WSIGE(tv, connection::close);
      WSIG(tv, connection::port, std::string, port);
      WSIG(tv, connection::place, std::string, place);
      WSIG(tv, requirement::open, ITVAL(XMLTYPE(requirements_type)), req);
      WSIGE(tv, requirement::close);
      WSIG(tv, requirement::key, std::string, key);
      WSIG(tv, requirement::value, bool, val);
      WSIG(tv, expression::open, XMLTYPE(expression_type), exp);
      WSIG(tv, mod::open, XMLTYPE(mod_type), mod);
      WSIGE(tv, mod::close);
      WSIG(tv, mod::name, std::string, name);
      WSIG(tv, mod::fun, std::string, fun);
      WSIG(tv, mod::cincludes, XMLTYPE(cincludes_type), cincludes);
      WSIG(tv, mod::ldflags, XMLTYPE(flags_type), ldflags);
      WSIG(tv, mod::code, MAYBE(std::string), code);
      WSIG(tv, token::literal::name, ::literal::type_name_t, token);
      WSIG(tv, token::structured::field, ::signature::structured_t::const_iterator::value_type, field);
      WSIG(tv, net::open, XMLTYPE(net_type), net);
      WSIGE(tv, net::close);
      WSIG(tv, net::properties, WETYPE(property::type), prop);
      WSIG(tv, net::structs, XMLTYPE(structs_type), structs);
      WSIG(tv, net::templates, XMLTYPE(templates_type), templates);
      WSIG(tv, net::specializes, XMLTYPE(specializes_type), specializes);
      WSIG(tv, net::functions, XMLTYPE(functions_type), functions);
      WSIG(tv, net::places, XMLTYPE(net_type::places_type), places);
      WSIG( tv
          , net::transitions
          , XMLTYPE(net_type::transitions_type)
          , transitions
          );
      WSIG(tv, use::name, std::string, name);
      WSIG(tv, context::open, XMLPARSE(state::key_values_t), context);
      WSIG(tv, context::key_value, XMLPARSE(state::key_value_t), kv);
    }
  }
}

#endif
