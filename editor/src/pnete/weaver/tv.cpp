// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/weaver/tv.hpp>
#include <pnete/weaver/weaver.hpp>

#include <pnete/data/handle/function.hpp>

#include <QStandardItem>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <string>
#include <stack>

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace
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
                                        , const boost::optional<T> & val
                                        );
          void append_maybe_bool ( const std::string & key
                                 , const boost::optional<bool> & val
                                 );
          void append_maybe ( const std::string & key
                            , const boost::optional<std::string> & val
                            );

          template<typename IT>
            void append_list (const QString& name, IT pos, const IT& end);
          template<typename C>
            void append_list (const QString& name, const C& collection);

          template<typename T> QStandardItem * append (const T & x);
          template<typename U, typename V>
            QStandardItem* append (const std::pair<U,V>& x);

          template<typename IT>

            void xs ( const QString & header
                    , IT pos
                    , const IT & end
                    , void (*fun)
                      (tv*, const typename std::iterator_traits<IT>::value_type&)
                    );

          template<typename Coll>
            void xs ( const std::string & header
                    , const Coll & coll
                    , void (*fun)
                      ( tv*
                      , const typename std::iterator_traits
                        <typename Coll::const_iterator>::value_type&
                      )
                    );
        };

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

        template<>
        QStandardItem* tv::append<xml::parse::type::link_type>
          (const xml::parse::type::link_type& l)
        {
          return append ( boost::format ("href %1%, prefix %2%")
                        % l.href()
                        % l.prefix()
                        );
        }

        WSIG(tv, transition::name, std::string, name)
        {
          push (append (name));
        }
        WSIGE(tv, transition::close) { pop(); }
        WSIG(tv, transition::priority, boost::optional<petri_net::priority_type>, prio)
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
        WSIG(tv, transition::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }
        WSIG(tv, transition::structs, ::xml::parse::type::structs_type, structs)
        {
          from::structs (this, structs);
        }
        WSIG(tv, transition::function, ::xml::parse::type::transition_type::function_or_use_type, fun)
        {
          from::variant (this, fun);
        }
        WSIG(tv, transition::place_map, ::xml::parse::type::transition_type::place_maps_type, pm)
        {
          xs ("place-map", pm.ids(), from::place_map);
        }
        WSIG(tv, transition::connection, ::xml::parse::type::transition_type::connections_type, cs)
        {
          xs ("connections", cs.ids(), from::connection);
        }
        WSIG(tv, transition::condition, ::xml::parse::type::conditions_type, cond)
        {
          from::conditions (this, cond);
        }

        WSIG(tv, place::open, ::xml::parse::id::ref::place, place)
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
        WSIG(tv, place::token, ::xml::parse::type::place_type::token_type, token)
        {
          push (append ("token"));
          set_text (token);
          pop ();
        }
        WSIG(tv, place::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }

        WSIG(tv, properties::open, ::we::type::property::type, props)
        {
          xs ("property", props.get_map(), from::property);
        }

        WSIG(tv, property::open, ::we::type::property::key_type, key)
        {
          push (append (key));
        }
        WSIGE(tv, property::close) { pop(); }
        WSIG(tv, property::value, ::we::type::property::value_type, val)
        {
          add_value (val);
        }

        WSIG(tv, structure::open, std::string, name)
        {
          push (append (name));
        }
        WSIGE(tv, structure::close) { pop(); }
        WSIG(tv, structure::type, ::std::string, type)
        {
          add_type (type);
        }

        WSIG(tv, structs::open, ::xml::parse::type::structs_type, structs)
        {
          xs ("struct", structs, from::structure);
        }

        WSIG(tv, port::open, ::xml::parse::id::ref::port, port)
        {
          push (append (we::type::enum_to_string (port.get().direction())));
        }
        WSIGE(tv, port::close) { pop(); }
        WSIG(tv, port::name, std::string, name)
        {
          add_something (": ", name);
        }
        WSIG(tv, port::type, std::string, type)
        {
          add_type (type);
        }
        WSIG(tv, port::place, boost::optional<std::string>, place)
        {
          append_maybe_key_value ("place", "%s", place);
        }
        WSIG(tv, port::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }

        WSIG(tv, expression_sequence::line, std::string, line)
        {
          append (line);
        }

        WSIG(tv, type_get::open, ::xml::parse::type::type_get_type::const_iterator::value_type, tg)
        {
          append (tg);
        }

        WSIG(tv, type_map::open, ::xml::parse::type::type_map_type::const_iterator::value_type, tm)
        {
          append_key_value (tm.first, "%s", tm.second);
        }

        WSIG(tv, specialize::open, ::xml::parse::id::ref::specialize, id)
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
        WSIG(tv, specialize::type_map, ::xml::parse::type::type_map_type, tm)
        {
          xs ("type_map", tm, from::type_map);
        }
        WSIG(tv, specialize::type_get, ::xml::parse::type::type_get_type, tg)
        {
          xs ("type_get", tg, from::type_get);
        }

        WSIG(tv, conditions::open, ::xml::parse::type::conditions_type, cs)
        {
          xs ("condition", cs, from::expression_sequence);
        }

        WSIG(tv, tmpl::open, ::xml::parse::id::ref::tmpl, t)
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
        WSIG(tv, tmpl::template_parameter, ::xml::parse::type::tmpl_type::names_type, templates)
        {
          append_list ("template-parameter", templates);
        }
        WSIG(tv, tmpl::function, ::xml::parse::id::ref::function, fun)
        {
          from::function (this, fun);
        }
        WSIGE(tv, tmpl::close) { pop(); }

        WSIG(tv, function::open, ::xml::parse::id::ref::function, fun)
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
        WSIG(tv, function::require, ::xml::parse::type::requirements_type, reqs)
        {
          append_list ("require", reqs);
        }
        WSIG(tv, function::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }
        WSIG(tv, function::structs, ::xml::parse::type::structs_type, structs)
        {
          from::structs (this, structs);
        }
        WSIG(tv, function::ports, ::xml::parse::type::function_type::ports_type, ports)
        {
          xs ("ports", ports.ids(), from::port);
        }
        WSIG(tv, function::fun, ::xml::parse::type::function_type::content_type, fun)
        {
          from::variant (this, fun);
        }
        WSIG(tv, function::conditions, ::xml::parse::type::conditions_type, cs)
        {
          from::conditions (this, cs);
        }

        WSIG(tv, place_map::open, ::xml::parse::id::ref::place_map, pm)
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
        WSIG(tv, place_map::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }

        WSIG(tv, connection::open, ::xml::parse::id::ref::connect, connection)
        {
          push (append (petri_net::edge::enum_to_string (connection.get().direction())));
        }
        WSIGE(tv, connection::close) { pop(); }
        WSIG(tv, connection::port, std::string, port)
        {
          add_something
            (": ", QString ("port: ").append (QString::fromStdString (port)).toStdString());
        }
        WSIG(tv, connection::place, std::string, place)
        {
          add_something (" -> place: ", place);
        }

        WSIG(tv, expression::open, ::xml::parse::id::ref::expression, id)
        {
          xs ("expression", id.get().expressions(), from::expression_sequence);
        }

        WSIG(tv, mod::open, ::xml::parse::id::ref::module, mod)
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
        WSIG(tv, mod::cincludes, std::list<std::string>, cincludes)
        {
          append_list ("cinclude", cincludes);
        }
        WSIG(tv, mod::ldflags, std::list<std::string>, ldflags)
        {
          append_list ("ldflag", ldflags);
        }
        WSIG(tv, mod::cxxflags, std::list<std::string>, cxxflags)
        {
          append_list ("cxxflag", cxxflags);
        }
        WSIG(tv, mod::links, std::list<xml::parse::type::link_type>, links)
        {
          append_list ("link", links);
        }
        WSIG(tv, mod::code, boost::optional<std::string>, code)
        {
          append_maybe ("code", code);
        }

        WSIG(tv, token::literal::name, ::std::string, token)
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

        WSIG(tv, net::open, ::xml::parse::id::ref::net, net)
        {
          push (append ("net"));
        }
        WSIGE(tv, net::close) { pop(); }
        WSIG(tv, net::properties, ::we::type::property::type, prop)
        {
          from::properties (this, prop);
        }
        WSIG(tv, net::structs, ::xml::parse::type::structs_type, structs)
        {
          from::structs (this, structs);
        }
        WSIG(tv, net::templates, ::xml::parse::type::net_type::templates_type, templates)
        {
          xs ("template", templates.ids(), from::tmpl);
        }
        WSIG(tv, net::specializes, ::xml::parse::type::net_type::specializes_type, specializes)
        {
          xs ("specialize", specializes.ids(), from::specialize);
        }
        WSIG(tv, net::functions, ::xml::parse::type::net_type::functions_type, functions)
        {
          xs ("function", functions.ids(), from::function);
        }
        WSIG(tv, net::places, ::xml::parse::type::net_type::places_type, places)
        {
          xs ("place", places.ids(), from::place);
        }
        WSIG( tv
            , net::transitions
            , ::xml::parse::type::net_type::transitions_type
            , transitions
            )
        {
          xs ("transition", transitions.ids(), from::transition);
        }

        WSIG(tv, use::name, std::string, name)
        {
          append (boost::format("use: %s") % name);
        }
      }

      namespace treeview
      {
        void function ( QStandardItem* tree_root
                      , const data::handle::function& data
                      )
        {
          weaver::tv w (tree_root);
          weaver::from::function (&w, data.id());
        }
      }
    }
  }
}
