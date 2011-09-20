// mirko.rahn@itwm.fraunhofer.de

#include <pnete/weaver/tv.hpp>
#include <pnete/weaver/weaver.hpp>

#include <QStandardItem>

#include <boost/format.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      static std::string boolString (const bool & v)
      {
        return v ? "true" : "false";
      }

      tv::tv (QStandardItem * root)
        : _stack ()
      {
        _stack.push (root);
      }

      void tv::assert_nonempty () const
      {
        if (_stack.empty())
          {
            throw std::runtime_error ("weaver::tv::assert_nonempty (EMPTY)");
          }
      }

      void tv::assert_empty () const
      {
        if (!_stack.empty())
          {
            throw std::runtime_error ("weaver::tv::assert_empty (NONEMPTY)");
          }
      }

      void tv::push (QStandardItem * x) { _stack.push (x); }
      void tv::pop () { assert_nonempty(); _stack.pop(); }
      QStandardItem * tv::top () { assert_nonempty(); return _stack.top(); }

      void tv::set_text (const QString & str)
      {
        top()->setText (str);
      }
      void tv::set_text (const std::string & str)
      {
        set_text (QString (str.c_str()));
      }
      void tv::add_something (const std::string & sep, const std::string & what)
      {
        set_text ( top()->text()
                 . append(sep.c_str())
                 . append(what.c_str())
                 );
      }

      void tv::add_type (const std::string & type)
      {
        add_something (" :: ", type);
      }
      void tv::add_value(const std::string & value)
      {
        add_something (" = ", value);
      }
      template<typename T>
      void tv::append_key_value ( const std::string & key
                                , const std::string & fmt
                                , const T & val
                                )
      {
        push (append (key));

        add_value ((boost::format (fmt) % val).str());

        pop ();
      }
      template<typename T>
      void tv::append_maybe_key_value ( const std::string & key
                                      , const std::string & fmt
                                      , const fhg::util::maybe<T> & val
                                      )
      {
        if (val)
          {
            append_key_value (key, fmt, *val);
          }
      }
      void tv::append_maybe_bool ( const std::string & key
                                 , const fhg::util::maybe<bool> & val
                                 )
      {
        if (val)
          {
            append_key_value (key, "%s", boolString (*val));
          }
      }

      template<typename T> QStandardItem * tv::append (const T & x)
      {
        QStandardItem* x_item (new QStandardItem (x));
        x_item->setEditable (false);
        top()->appendRow (x_item);
        return x_item;
      }

      template<typename IT>
      void tv::xs
      ( const QString & header
      , IT pos
      , const IT & end
      , void (*fun) ( tv *
                    , const typename std::iterator_traits<IT>::value_type &
                    )
      )
      {
        if (pos != end)
          {
            push (append (header));

            while (pos != end)
              {
                fun (this, *pos);

                ++pos;
              }

            pop ();
          }
      }

      template<typename Coll>
      void tv::xs
      ( const std::string & header
      , const Coll & coll
      , void (*fun) ( tv *
                    , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                    )
      )
      {
        xs (QString (header.c_str()), coll.begin(), coll.end(), fun);
      }

      template<>
      QStandardItem * tv::append<std::string> (const std::string & str)
      {
        return append (QString (str.c_str()));
      }

      template<>
      QStandardItem * tv::append<boost::format> (const boost::format & f)
      {
        return append (f.str());
      }


      WSIG(tv, transition::name, std::string, name)
      {
        push (append (name));
      }
      WSIGE(tv, transition::close) { pop(); }
      WSIG(tv, transition::priority, MAYBE(petri_net::prio_t), prio)
      {
        append_maybe_key_value ("priority", "%i", prio);
      }
      WSIG(tv, transition::internal, MAYBE(bool), internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG(tv, transition::_inline, MAYBE(bool), _inline)
      {
        append_maybe_bool ("inline", _inline);
      }
      WSIG(tv, transition::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(tv, transition::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(tv, transition::function, XMLTYPE(transition_type::f_type), fun)
      {
        boost::apply_visitor
          (FROM(visitor::function_type<tv>) (this), fun);
      }
      WSIG(tv, transition::place_map, XMLTYPE(place_maps_type), pm)
      {
        xs ("place-map", pm, FROM(place_map));
      }
      WSIG(tv, transition::connect_read, XMLTYPE(connections_type), cs)
      {
        xs ("connect-read", cs, FROM(connection));
      }
      WSIG(tv, transition::connect_in, XMLTYPE(connections_type), cs)
      {
        xs ("connect-in", cs, FROM(connection));
      }
      WSIG(tv, transition::connect_out, XMLTYPE(connections_type), cs)
      {
        xs ("connect-out", cs, FROM(connection));
      }
      WSIG(tv, transition::condition, XMLTYPE(conditions_type), cond)
      {
        FROM(conditions) (this, cond);
      }

      WSIG(tv, place::open, ITVAL(XMLTYPE(places_type)), place)
      {
        push (append ("<<place>>"));
      }
      WSIGE(tv, place::close) { pop(); }
      WSIG(tv, place::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(tv, place::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(tv, place::is_virtual, MAYBE(bool), is_virtual)
      {
        append_maybe_bool ("virtual", is_virtual);
      }
      WSIG(tv, place::capacity, MAYBE(petri_net::capacity_t), capacity)
      {
        append_maybe_key_value ("capacity", "%i", capacity);
      }
      WSIG(tv, place::token, ITVAL(XMLTYPE(tokens_type)), token)
      {
        push (append ("token"));
        boost::apply_visitor (FROM(visitor::token<tv>) (this), token);
        pop ();
      }
      WSIG(tv, place::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG(tv, properties::open, WETYPE(property::type), props)
      {
        xs ("property", props.get_map(), FROM(property));
      }

      WSIG(tv, property::open, WETYPE(property::key_type), key)
      {
        push (append (key));
      }
      WSIGE(tv, property::close) { pop(); }
      WSIG(tv, property::value, WETYPE(property::value_type), val)
      {
        add_value (val);
      }

      WSIG(tv, _struct::open, std::string, name)
      {
        push (append (name));
      }
      WSIGE(tv, _struct::close) { pop(); }
      WSIG(tv, _struct::type, ::literal::type_name_t, type)
      {
        add_type (type);
      }

      WSIG(tv, structs::open, XMLTYPE(structs_type), structs)
      {
        xs ("struct", structs, FROM(_struct));
      }

      WSIG(tv, port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        push (append ("<<port>>"));
      }
      WSIGE(tv, port::close) { pop(); }
      WSIG(tv, port::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(tv, port::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(tv, port::place, MAYBE(std::string), place)
      {
        append_maybe_key_value ("place", "%s", place);
      }
      WSIG(tv, port::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG(tv, cinclude::open, ITVAL(XMLTYPE(cincludes_type)), cinclude)
      {
        append (cinclude);
      }

      WSIG(tv, link::open, ITVAL(XMLTYPE(links_type)), link)
      {
        append (link);
      }

      WSIG(tv, expression_sequence::line, std::string, line)
      {
        append (line);
      }

      WSIG(tv, type_get::open, ITVAL(XMLTYPE(type_get_type)), tg)
      {
        append (tg);
      }

      WSIG(tv, type_map::open, ITVAL(XMLTYPE(type_map_type)), tm)
      {
        append_key_value (tm.first, "%s", tm.second);
      }

      WSIG(tv,  specialize::open
          , ITVAL(XMLTYPE(net_type::specializes_type))
          , specialize
          )
      {
        push (append ("<<specialize>>"));
      }
      WSIGE(tv, specialize::close) { pop(); }
      WSIG(tv, specialize::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(tv, specialize::use, std::string, use)
      {
        add_something (" use ", use);
      }
      WSIG(tv, specialize::type_map, XMLTYPE(type_map_type), tm)
      {
        xs ("type_map", tm, FROM(type_map));
      }
      WSIG(tv, specialize::type_get, XMLTYPE(type_get_type), tg)
      {
        xs ("type_get", tg, FROM(type_get));
      }

      WSIG(tv, conditions::open, XMLTYPE(conditions_type), cs)
      {
        xs ("condition", cs, FROM(expression_sequence));
      }

      WSIG( tv
          , function::open
          , ITVAL(XMLTYPE(net_type::functions_type))
          , fun
          )
      {
        push (append ("<<function>>"));
      }
      WSIGE(tv, function::close) { pop(); }
      WSIG(tv, function::name, MAYBE(std::string), name)
      {
        if (name)
          {
            set_text (*name);
          }
      }
      WSIG(tv, function::internal, MAYBE(bool), internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG(tv, function::require, XMLTYPE(requirements_type), reqs)
      {
        xs ("require", reqs, FROM(require));
      }
      WSIG(tv, function::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(tv, function::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(tv, function::in, XMLTYPE(ports_type), ports)
      {
        xs ("in", ports, FROM(port));
      }
      WSIG(tv, function::out, XMLTYPE(ports_type), ports)
      {
        xs ("out", ports, FROM(port));
      }
      WSIG(tv, function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (FROM(visitor::net_type<tv>) (this), fun);
      }
      WSIG(tv, function::conditions, XMLTYPE(conditions_type), cs)
      {
        FROM(conditions) (this, cs);
      }

      WSIG(tv, place_map::open, ITVAL(XMLTYPE(place_maps_type)), pm)
      {
        push (append ("<<place_map>>"));
      }
      WSIGE(tv, place_map::close) { pop(); }
      WSIG(tv, place_map::place_virtual, std::string, name)
      {
        set_text (QString ("virtual: ").append(name.c_str()));
      }
      WSIG(tv, place_map::place_real, std::string, name)
      {
        add_something(" <-> real: ", name);
      }
      WSIG(tv, place_map::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG( tv
          , connection::open
          , ITVAL(XMLTYPE(connections_type))
          , connection
          )
      {
        push (append ("<<connection>>"));
      }
      WSIGE(tv, connection::close) { pop(); }
      WSIG(tv, connection::port, std::string, port)
      {
        set_text (QString ("port: ").append (port.c_str()));
      }
      WSIG(tv, connection::place, std::string, place)
      {
        add_something (" -> place: ", place);
      }

      WSIG(tv, requirement::open, ITVAL(XMLTYPE(requirements_type)), req)
      {
        push (append ("requirement"));
      }
      WSIGE(tv, requirement::close) { pop(); }
      WSIG(tv, requirement::key, std::string, key)
      {
        add_something("", key);
      }
      WSIG(tv, requirement::value, bool, val)
      {
        add_something(": ", boolString(val));
      }

      WSIG(tv, expression::open, XMLTYPE(expression_type), exp)
      {
        xs ("expression", exp.expressions, FROM(expression_sequence));
      }

      WSIG(tv, mod::open, XMLTYPE(mod_type), mod)
      {
        push (append ("module"));
      }
      WSIGE(tv, mod::close) { pop(); pop(); }
      WSIG(tv, mod::name, std::string, name)
      {
        push (append (name));
      }
      WSIG(tv, mod::fun, std::string, fun)
      {
        add_something (" -> ", fun);
      }
      WSIG(tv, mod::cincludes, XMLTYPE(cincludes_type), cincludes)
      {
        xs ("cinclude", cincludes, FROM(cinclude));
      }
      WSIG(tv, mod::links, XMLTYPE(links_type), links)
      {
        xs ("link", links, FROM(link));
      }
      WSIG(tv, mod::code, MAYBE(std::string), code)
      {
        if (code)
          {
            push (append ("code"));
            append (*code);
            pop();
          }
      }

      WSIG(tv, token::literal::name, ::literal::type_name_t, token)
      {
        append (token);
      }

      WSIG(tv, token::structured::field, ::signature::structured_t::const_iterator::value_type, field)
      {
        push (append (field.first));

        boost::apply_visitor
          (FROM(visitor::token<tv>) (this), field.second);

        pop ();
      }

      WSIG(tv, net::open, XMLTYPE(net_type), net)
      {
        push (append ("net"));
      }
      WSIGE(tv, net::close) { pop(); }
      WSIG(tv, net::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(tv, net::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(tv, net::templates, XMLTYPE(net_type::templates_type), templates)
      {
        xs ("template", templates, FROM(function));
      }
      WSIG( tv
          , net::specializes
          , XMLTYPE(net_type::specializes_type)
          , specializes
          )
      {
        xs ("specialize", specializes, FROM(specialize));
      }
      WSIG( tv
          , net::functions
          , XMLTYPE(net_type::functions_type)
          , functions
          )
      {
        xs ("function", functions, FROM(function));
      }
      WSIG(tv, net::places, XMLTYPE(net_type::places_type), places)
      {
        xs ("place", places, FROM(place));
      }
      WSIG( tv
          , net::transitions
          , XMLTYPE(net_type::transitions_type)
          , transitions
          )
      {
        xs ("transition", transitions, FROM(transition));
      }

      WSIG(tv, use::name, std::string, name)
      {
        append (boost::format("use: %s") % name);
      }

      WSIG(tv, context::open, XMLPARSE(state::key_values_t), context)
      {
        xs ("context", context, FROM(context_key_value));
      }
      WSIG(tv, context::key_value, XMLPARSE(state::key_value_t), kv)
      {
        append_key_value (kv.key(), "%s", kv.value());
      }
    }
  }
}
