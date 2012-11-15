// mirko.rahn@itwm.fraunhofer.de

#include <pnete/weaver/tv.hpp>
#include <pnete/weaver/weaver.hpp>

#include <QStandardItem>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
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
        set_text (QString::fromStdString (str));
      }
      void tv::add_something (const std::string & sep, const std::string & what)
      {
        set_text ( top()->text()
                 . append (QString::fromStdString (sep))
                 . append (QString::fromStdString (what))
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
                                      , const boost::optional<T> & val
                                      )
      {
        if (val)
          {
            append_key_value (key, fmt, *val);
          }
      }
      void tv::append_maybe_bool ( const std::string & key
                                 , const boost::optional<bool> & val
                                 )
      {
        if (val)
          {
            append_key_value ( key
                             , "%s"
                             , boost::lexical_cast<std::string> (*val)
                             );
          }
      }

      void tv::append_maybe ( const std::string & key
                            , const boost::optional<std::string> & val
                            )
      {
        if (val)
        {
          append_key_value (key, "%s", *val);
        }
      }

      template<typename IT>
        void tv::append_list (const QString& name, IT pos, const IT& end)
      {
        push (append (name));
        while (pos != end)
        {
          append (*pos);
          ++pos;
        }
        pop();
      }
      template<typename C>
        void tv::append_list (const QString& name, const C& collection)
      {
        append_list (name, collection.begin(), collection.end());
      }

      template<typename T> QStandardItem * tv::append (const T & x)
      {
        QStandardItem* x_item (new QStandardItem (x));
        x_item->setEditable (false);
        top()->appendRow (x_item);
        return x_item;
      }
      template<typename U, typename V>
        QStandardItem* tv::append (const std::pair<U,V>& x)
      {
        return append ( QString::fromStdString
                        ((boost::lexical_cast<std::string> (x.first)))
                      . append (": ")
                      . append ( QString::fromStdString
                                (boost::lexical_cast<std::string> (x.second))
                               )
                      );
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

            from::many (this, pos, end, fun);

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
        xs (QString::fromStdString (header), coll.begin(), coll.end(), fun);
      }

      template<>
      QStandardItem * tv::append<std::string> (const std::string & str)
      {
        return append (QString::fromStdString (str));
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
      WSIG(tv, transition::priority, boost::optional<petri_net::prio_t>, prio)
      {
        append_maybe_key_value ("priority", "%i", prio);
      }
      WSIG(tv, transition::internal, boost::optional<bool>, internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG(tv, transition::_inline, boost::optional<bool>, _inline)
      {
        append_maybe_bool ("inline", _inline);
      }
      WSIG(tv, transition::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
      }
      WSIG(tv, transition::structs, XMLTYPE(structs_type), structs)
      {
        from::structs (this, structs);
      }
      WSIG(tv, transition::function, XMLTYPE(transition_type::function_or_use_type), fun)
      {
        boost::apply_visitor (from::visitor::deref_variant<tv> (this), fun);
      }
      WSIG(tv, transition::place_map, XMLTYPE(transition_type::place_maps_type), pm)
      {
        xs ("place-map", pm.values(), from::place_map);
      }
      WSIG(tv, transition::connect_read, XMLTYPE(transition_type::connections_type), cs)
      {
        xs ("connect-read", cs.values(), from::connection);
      }
      WSIG(tv, transition::connect_in, XMLTYPE(transition_type::connections_type), cs)
      {
        xs ("connect-in", cs.values(), from::connection);
      }
      WSIG(tv, transition::connect_out, XMLTYPE(transition_type::connections_type), cs)
      {
        xs ("connect-out", cs.values(), from::connection);
      }
      WSIG(tv, transition::condition, XMLTYPE(conditions_type), cond)
      {
        from::conditions (this, cond);
      }

      WSIG(tv, place::open, XMLTYPE(place_type), place)
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
      WSIG(tv, place::is_virtual, boost::optional<bool>, is_virtual)
      {
        append_maybe_bool ("virtual", is_virtual);
      }
      WSIG(tv, place::token, XMLTYPE(place_type::token_type), token)
      {
        push (append ("token"));
        boost::apply_visitor (from::visitor::token<tv> (this), token);
        pop ();
      }
      WSIG(tv, place::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
      }

      WSIG(tv, properties::open, WETYPE(property::type), props)
      {
        xs ("property", props.get_map(), from::property);
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
        xs ("struct", structs, from::_struct);
      }

      WSIG(tv, port::open, XMLTYPE(port_type), port)
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
      WSIG(tv, port::place, boost::optional<std::string>, place)
      {
        append_maybe_key_value ("place", "%s", place);
      }
      WSIG(tv, port::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
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

      WSIG(tv, specialize::open, XMLTYPE(specialize_type), specialize)
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
        xs ("type_map", tm, from::type_map);
      }
      WSIG(tv, specialize::type_get, XMLTYPE(type_get_type), tg)
      {
        xs ("type_get", tg, from::type_get);
      }

      WSIG(tv, conditions::open, XMLTYPE(conditions_type), cs)
      {
        xs ("condition", cs, from::expression_sequence);
      }

      WSIG(tv, tmpl::open, XMLTYPE(tmpl_type), t)
      {
        push (append ("<<template>>"));
      }
      WSIG(tv, tmpl::name, boost::optional<std::string>, name)
      {
        if (name)
          {
            set_text (*name);
          }
      }
      WSIG(tv, tmpl::template_parameter, XMLTYPE(tmpl_type::names_type), templates)
      {
        append_list ("template-parameter", templates);
      }
      WSIG(tv, tmpl::function, ::xml::parse::id::ref::function, fun)
      {
        from::function (this, fun);
      }
      WSIGE(tv, tmpl::close) { pop(); }

      WSIG(tv, function::open, XMLTYPE(function_type), fun)
      {
        push (append ("<<function>>"));
      }
      WSIGE(tv, function::close) { pop(); }
      WSIG(tv, function::name, boost::optional<std::string>, name)
      {
        if (name)
          {
            set_text (*name);
          }
      }
      WSIG(tv, function::internal, boost::optional<bool>, internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG(tv, function::require, XMLTYPE(requirements_type), reqs)
      {
        append_list ("require", reqs);
      }
      WSIG(tv, function::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
      }
      WSIG(tv, function::structs, XMLTYPE(structs_type), structs)
      {
        from::structs (this, structs);
      }
      WSIG(tv, function::in, XMLTYPE(function_type::ports_type), ports)
      {
        xs ("in", ports.values(), from::port);
      }
      WSIG(tv, function::out, XMLTYPE(function_type::ports_type), ports)
      {
        xs ("out", ports.values(), from::port);
      }
      WSIG(tv, function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (from::visitor::deref_variant<tv> (this), fun);
      }
      WSIG(tv, function::conditions, XMLTYPE(conditions_type), cs)
      {
        from::conditions (this, cs);
      }

      WSIG(tv, place_map::open, XMLTYPE(place_map_type), pm)
      {
        push (append ("<<place_map>>"));
      }
      WSIGE(tv, place_map::close) { pop(); }
      WSIG(tv, place_map::place_virtual, std::string, name)
      {
        set_text (QString ("virtual: ").append(QString::fromStdString (name)));
      }
      WSIG(tv, place_map::place_real, std::string, name)
      {
        add_something(" <-> real: ", name);
      }
      WSIG(tv, place_map::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
      }

      WSIG(tv, connection::open, XMLTYPE(connect_type), connection)
      {
        push (append ("<<connection>>"));
      }
      WSIGE(tv, connection::close) { pop(); }
      WSIG(tv, connection::port, std::string, port)
      {
        set_text (QString ("port: ").append (QString::fromStdString (port)));
      }
      WSIG(tv, connection::place, std::string, place)
      {
        add_something (" -> place: ", place);
      }

      WSIG(tv, expression::open, XMLTYPE(expression_type), exp)
      {
        xs ("expression", exp.expressions(), from::expression_sequence);
      }

      WSIG(tv, mod::open, XMLTYPE(module_type), mod)
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
      WSIG(tv, mod::cincludes, XMLTYPE(module_type::cincludes_type), cincludes)
      {
        append_list ("cinclude", cincludes);
      }
      WSIG(tv, mod::ldflags, XMLTYPE(module_type::flags_type), ldflags)
      {
        append_list ("ldflag", ldflags);
      }
      WSIG(tv, mod::cxxflags, XMLTYPE(module_type::flags_type), cxxflags)
      {
        append_list ("cxxflag", cxxflags);
      }
      WSIG(tv, mod::links, XMLTYPE(module_type::flags_type), links)
      {
        append_list ("link", links);
      }
      WSIG(tv, mod::code, boost::optional<std::string>, code)
      {
        append_maybe ("code", code);
      }

      WSIG(tv, token::literal::name, ::literal::type_name_t, token)
      {
        append (token);
      }

      WSIG(tv, token::structured::field, ::signature::structured_t::const_iterator::value_type, field)
      {
        push (append (field.first));

        boost::apply_visitor
          (from::visitor::token<tv> (this), field.second);

        pop ();
      }

      WSIG(tv, net::open, XMLTYPE(net_type), net)
      {
        push (append ("net"));
      }
      WSIGE(tv, net::close) { pop(); }
      WSIG(tv, net::properties, WETYPE(property::type), prop)
      {
        from::properties (this, prop);
      }
      WSIG(tv, net::structs, XMLTYPE(structs_type), structs)
      {
        from::structs (this, structs);
      }
      WSIG(tv, net::templates, XMLTYPE(net_type::templates_type), templates)
      {
        xs ("template", templates.ids(), from::tmpl);
      }
      WSIG(tv, net::specializes, XMLTYPE(net_type::specializes_type), specializes)
      {
        xs ("specialize", specializes.ids(), from::specialize);
      }
      WSIG(tv, net::functions, XMLTYPE(net_type::functions_type), functions)
      {
        xs ("function", functions.ids(), from::function);
      }
      WSIG(tv, net::places, XMLTYPE(net_type::places_type), places)
      {
        xs ("place", places.ids(), from::place);
      }
      WSIG( tv
          , net::transitions
          , XMLTYPE(net_type::transitions_type)
          , transitions
          )
      {
        xs ("transition", transitions.values(), from::transition);
      }

      WSIG(tv, use::name, std::string, name)
      {
        append (boost::format("use: %s") % name);
      }

      WSIG(tv, context::open, XMLPARSE(state::key_values_t), context)
      {
        xs ("context", context, from::context_key_value);
      }
      WSIG(tv, context::key_value, XMLPARSE(state::key_value_t), kv)
      {
        append_key_value (kv.key(), "%s", kv.value());
      }
    }
  }
}
