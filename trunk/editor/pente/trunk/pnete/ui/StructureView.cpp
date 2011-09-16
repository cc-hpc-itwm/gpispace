// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/StructureView.hpp>

#include <pnete/traverse/weaver.hpp>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>
#include <QString>

#include <boost/format.hpp>

#include <stack>

#include <stdexcept>

namespace fhg
{
  namespace pnete
  {
    namespace tv
    {
      class weaver
      {
      public:
        explicit weaver (QStandardItem * root)
          : _stack ()
        {
          _stack.push (root);
        }

        template<int Type, typename T> void weave (const T & x) {}
        template<int Type> void weave () {}

      private:
        std::stack<QStandardItem *> _stack;

        inline void assert_nonempty () const
        {
          if (_stack.empty())
            {
              throw std::runtime_error ("tv::weaver::assert_nonempty (EMPTY)");
            }
        }

        inline void assert_empty () const
        {
          if (!_stack.empty())
            {
              throw std::runtime_error ("tv::weaver::assert_empty (NONEMPTY)");
            }
        }

        void push (QStandardItem * x) { _stack.push (x); }
        void pop () { assert_nonempty(); _stack.pop(); }
        QStandardItem * top () { assert_nonempty(); return _stack.top(); }

        void set_text (const QString & str)
        {
          top()->setText (str);
        }
        void set_text (const std::string & str)
        {
          set_text (QString (str.c_str()));
        }
        void add_something (const std::string & sep, const std::string & what)
        {
          set_text ( top()->text()
                   . append(sep.c_str())
                   . append(what.c_str())
                   );
        }

        void add_type (const std::string & type)
        {
          add_something (" :: ", type);
        }
        void add_value(const std::string & value)
        {
          add_something (" = ", value);
        }
        template<typename T>
        void append_key_value ( const std::string & key
                              , const std::string & fmt
                              , const T & val
                              )
        {
          push (append (key));

          add_value ((boost::format (fmt) % val).str());

          pop ();
        }
        template<typename T>
        void append_maybe_key_value ( const std::string & key
                                    , const std::string & fmt
                                    , const fhg::util::maybe<T> & val
                                    )
        {
          if (val)
            {
              append_key_value (key, fmt, *val);
            }
        }
        void append_maybe_bool ( const std::string & key
                               , const fhg::util::maybe<bool> & val
                               )
        {
          if (val)
            {
              append_key_value (key, "%s", boolString (*val));
            }
        }

        static std::string boolString (const bool & v)
        {
          return v ? "true" : "false";
        }

        template<typename T> QStandardItem * append (const T & x)
        {
          QStandardItem* x_item (new QStandardItem (x));
          x_item->setEditable (false);
          top()->appendRow (x_item);
          return x_item;
        }

        template<typename IT>
        void xs
        ( const QString & header
        , IT pos
        , const IT & end
        , void (*fun) ( weaver *
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
        void xs
        ( const std::string & header
        , const Coll & coll
        , void (*fun) ( weaver *
                      , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                      )
        )
        {
          xs (QString (header.c_str()), coll.begin(), coll.end(), fun);
        }
      };

      template<>
      QStandardItem * weaver::append<std::string> (const std::string & str)
      {
        return append (QString (str.c_str()));
      }

      template<>
      QStandardItem * weaver::append<boost::format> (const boost::format & f)
      {
        return append (f.str());
      }

      WSIG (transition::name, std::string, name)
      {
        push (append (name));
      }
      WSIGE (transition::close) { pop(); }
      WSIG(transition::priority, MAYBE(petri_net::prio_t), prio)
      {
        append_maybe_key_value ("priority", "%i", prio);
      }
      WSIG (transition::internal, MAYBE(bool), internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG (transition::_inline, MAYBE(bool), _inline)
      {
        append_maybe_bool ("inline", _inline);
      }
      WSIG(transition::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(transition::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(transition::function, XMLTYPE(transition_type::f_type), fun)
      {
        boost::apply_visitor
          (FROM(visitor::function_type<weaver>) (this), fun);
      }
      WSIG(transition::place_map, XMLTYPE(place_maps_type), pm)
      {
        xs ("place-map", pm, FROM(place_map));
      }
      WSIG(transition::connect_read, XMLTYPE(connections_type), cs)
      {
        xs ("connect-read", cs, FROM(connection));
      }
      WSIG(transition::connect_in, XMLTYPE(connections_type), cs)
      {
        xs ("connect-in", cs, FROM(connection));
      }
      WSIG(transition::connect_out, XMLTYPE(connections_type), cs)
      {
        xs ("connect-out", cs, FROM(connection));
      }
      WSIG(transition::condition, XMLTYPE(conditions_type), cond)
      {
        FROM(conditions) (this, cond);
      }

      WSIG(place::open, ITVAL(XMLTYPE(places_type)), place)
      {
        push (append ("<<place>>"));
      }
      WSIGE(place::close) { pop(); }
      WSIG(place::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(place::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(place::is_virtual, MAYBE(bool), is_virtual)
      {
        append_maybe_bool ("virtual", is_virtual);
      }
      WSIG(place::capacity, MAYBE(petri_net::capacity_t), capacity)
      {
        append_maybe_key_value ("capacity", "%i", capacity);
      }
      WSIG(place::token, ITVAL(XMLTYPE(tokens_type)), token)
      {
        push (append ("token"));
        boost::apply_visitor (FROM(visitor::token<weaver>) (this), token);
        pop ();
      }
      WSIG(place::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG(properties::open, WETYPE(property::type), props)
      {
        xs ("property", props.get_map(), FROM(property));
      }

      WSIG(property::open, WETYPE(property::key_type), key)
      {
        push (append (key));
      }
      WSIGE(property::close) { pop(); }
      WSIG(property::value, WETYPE(property::value_type), val)
      {
        add_value (val);
      }

      WSIG (_struct::open, std::string, name)
      {
        push (append (name));
      }
      WSIGE(_struct::close) { pop(); }
      WSIG(_struct::type, ::literal::type_name_t, type)
      {
        add_type (type);
      }

      WSIG(structs::open, XMLTYPE(structs_type), structs)
      {
        xs ("struct", structs, FROM(_struct));
      }

      WSIG(port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        push (append ("<<port>>"));
      }
      WSIGE(port::close) { pop(); }
      WSIG(port::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(port::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(port::place, MAYBE(std::string), place)
      {
        append_maybe_key_value ("place", "%s", place);
      }
      WSIG(port::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG(cinclude::open, ITVAL(XMLTYPE(cincludes_type)), cinclude)
      {
        append (cinclude);
      }

      WSIG(link::open, ITVAL(XMLTYPE(links_type)), link)
      {
        append (link);
      }

      WSIG(expression_sequence::line, std::string, line)
      {
        append (line);
      }

      WSIG(type_get::open, ITVAL(XMLTYPE(type_get_type)), tg)
      {
        append (tg);
      }

      WSIG(type_map::open, ITVAL(XMLTYPE(type_map_type)), tm)
      {
        append_key_value (tm.first, "%s", tm.second);
      }

      WSIG( specialize::open
          , ITVAL(XMLTYPE(net_type::specializes_type))
          , specialize
          )
      {
        push (append ("<<specialize>>"));
      }
      WSIGE(specialize::close) { pop(); }
      WSIG(specialize::name, std::string, name)
      {
        set_text (name);
      }
      WSIG(specialize::use, std::string, use)
      {
        add_something (" use ", use);
      }
      WSIG(specialize::type_map, XMLTYPE(type_map_type), tm)
      {
        xs ("type_map", tm, FROM(type_map));
      }
      WSIG(specialize::type_get, XMLTYPE(type_get_type), tg)
      {
        xs ("type_get", tg, FROM(type_get));
      }

      WSIG(conditions::open, XMLTYPE(conditions_type), cs)
      {
        xs ("condition", cs, FROM(expression_sequence));
      }

      WSIG(function::open, ITVAL(XMLTYPE(net_type::functions_type)), fun)
      {
        push (append ("<<function>>"));
      }
      WSIGE(function::close) { pop(); }
      WSIG(function::name, MAYBE(std::string), name)
      {
        if (name)
          {
            set_text (*name);
          }
      }
      WSIG(function::internal, MAYBE(bool), internal)
      {
        append_maybe_bool ("internal", internal);
      }
      WSIG(function::require, XMLTYPE(requirements_type), reqs)
      {
        xs ("require", reqs, FROM(require));
      }
      WSIG(function::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(function::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(function::in, XMLTYPE(ports_type), ports)
      {
        xs ("in", ports, FROM(port));
      }
      WSIG(function::out, XMLTYPE(ports_type), ports)
      {
        xs ("out", ports, FROM(port));
      }
      WSIG(function::fun, XMLTYPE(function_type::type), fun)
      {
        boost::apply_visitor (FROM(visitor::net_type<weaver>) (this), fun);
      }
      WSIG(function::conditions, XMLTYPE(conditions_type), cs)
      {
        FROM(conditions) (this, cs);
      }

      WSIG(place_map::open, ITVAL(XMLTYPE(place_maps_type)), pm)
      {
        push (append ("<<place_map>>"));
      }
      WSIGE(place_map::close) { pop(); }
      WSIG(place_map::place_virtual, std::string, name)
      {
        set_text (QString ("virtual: ").append(name.c_str()));
      }
      WSIG(place_map::place_real, std::string, name)
      {
        add_something(" <-> real: ", name);
      }
      WSIG(place_map::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }

      WSIG(connection::open, ITVAL(XMLTYPE(connections_type)), connection)
      {
        push (append ("<<connection>>"));
      }
      WSIGE(connection::close) { pop(); }
      WSIG(connection::port, std::string, port)
      {
        set_text (QString ("port: ").append (port.c_str()));
      }
      WSIG(connection::place, std::string, place)
      {
        add_something (" -> place: ", place);
      }

      WSIG(requirement::open, ITVAL(XMLTYPE(requirements_type)), req)
      {
        push (append ("requirement"));
      }
      WSIGE(requirement::close) { pop(); }
      WSIG(requirement::key, std::string, key)
      {
        add_something("", key);
      }
      WSIG(requirement::value, bool, val)
      {
        add_something(": ", boolString(val));
      }

      WSIG(expression::open, XMLTYPE(expression_type), exp)
      {
        xs ("expression", exp.expressions, FROM(expression_sequence));
      }

      WSIG(mod::open, XMLTYPE(mod_type), mod)
      {
        push (append ("module"));
      }
      WSIGE(mod::close) { pop(); pop(); }
      WSIG(mod::name, std::string, name)
      {
        push (append (name));
      }
      WSIG (mod::fun, std::string, fun)
      {
        add_something (" -> ", fun);
      }
      WSIG(mod::cincludes, XMLTYPE(cincludes_type), cincludes)
      {
        xs ("cinclude", cincludes, FROM(cinclude));
      }
      WSIG(mod::links, XMLTYPE(links_type), links)
      {
        xs ("link", links, FROM(link));
      }
      WSIG(mod::code, MAYBE(std::string), code)
      {
        if (code)
          {
            push (append ("code"));
            append (*code);
            pop();
          }
      }

      WSIG(token::literal::name, ::literal::type_name_t, token)
      {
        append (token);
      }

      WSIG(token::structured::field, ::signature::structured_t::const_iterator::value_type, field)
      {
        push (append (field.first));

        boost::apply_visitor
          (FROM(visitor::token<weaver>) (this), field.second);

        pop ();
      }

      WSIG(net::open, XMLTYPE(net_type), net)
      {
        push (append ("net"));
      }
      WSIGE(net::close) { pop(); }
      WSIG(net::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (this, prop);
      }
      WSIG(net::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (this, structs);
      }
      WSIG(net::templates, XMLTYPE(net_type::templates_type), templates)
      {
        xs ("template", templates, FROM(function));
      }
      WSIG(net::specializes, XMLTYPE(net_type::specializes_type), specializes)
      {
        xs ("specialize", specializes, FROM(specialize));
      }
      WSIG(net::functions, XMLTYPE(net_type::functions_type), functions)
      {
        xs ("function", functions, FROM(function));
      }
      WSIG(net::places, XMLTYPE(net_type::places_type), places)
      {
        xs ("place", places, FROM(place));
      }
      WSIG(net::transitions, XMLTYPE(net_type::transitions_type), transitions)
      {
        xs ("transition", transitions, FROM(transition));
      }

      WSIG(use::name, std::string, name)
      {
        append (boost::format("use: %s") % name);
      }

      WSIG(context::open, XMLPARSE(state::key_values_t), context)
      {
        xs ("context", context, FROM(context_key_value));
      }
      WSIG(context::key_value, XMLPARSE(state::key_value_t), kv)
      {
        append_key_value (kv.key(), "%s", kv.value());
      }
    } // namespace tv

    namespace ui
    {
      StructureView::StructureView (QWidget* parent)
      : QTreeView (parent)
      , _model (new QStandardItemModel (this))
      , _root (_model->invisibleRootItem())
      {
        setModel (_model);

        //! \todo As soon as we can actually edit stuff in here, remove.
        setDragDropMode (QAbstractItemView::NoDragDrop);
        setEditTriggers (QAbstractItemView::NoEditTriggers);

        header()->hide();
      }

      void StructureView::append (data::internal::ptr data)
      {
        _datas.push_back (data);

        tv::weaver * w (new tv::weaver (_root));

        FROM( function_context<tv::weaver>
              ( w
              , WNAME(function_context_type) ( data->function()
                                             , data->context()
                                             )
              )
            );
      }

      void StructureView::clear()
      {
        setModel (_model = new QStandardItemModel (this));
        _root = _model->invisibleRootItem();
        _datas.clear();
      }
    }
  }
}
