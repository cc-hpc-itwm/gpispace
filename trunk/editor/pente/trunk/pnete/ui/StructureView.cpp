// mirko.rahn@itwm.fraunhofer.de

#include "StructureView.hpp"

#include <pnete/traverse/weaver.hpp>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>
#include <QString>

#include <boost/format.hpp>

#include <stack>

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

        template<int Type, typename T> void weave (const T & x) const {}
        template<int Type> void weave () const {}

      private:
        mutable std::stack<QStandardItem *> _stack;

        static std::string boolString (const bool & v)
        {
          return v ? "true" : "false";
        }

        void add_type (const std::string & type) const
        {
          _stack.top()->setText ( _stack.top()->text()
                                . append (" :: ")
                                . append (type.c_str())
                                );
        }

        template<typename T> QStandardItem * append (const T & x) const
        {
          QStandardItem* x_item (new QStandardItem (x));
          x_item->setEditable (false);
          _stack.top()->appendRow (x_item);
          return x_item;
        }

        void push (QStandardItem * x) const { _stack.push (x); }
        void pop () const { _stack.pop(); }

        template<typename IT>
        void xs
        ( const QString & header
        , IT pos
        , const IT & end
        , void (*fun) ( weaver
                      , const typename std::iterator_traits<IT>::value_type &
                      )
        ) const
        {
          if (pos != end)
            {
              weaver w (append (header));

              while (pos != end)
                {
                  fun (w, *pos);

                  ++pos;
                }
            }
        }

        template<typename Coll>
        void xs
        ( const std::string & header
        , const Coll & coll
        , void (*fun) ( weaver
                      , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                      )
        ) const
        {
          xs (QString (header.c_str()), coll.begin(), coll.end(), fun);
        }
      };

      template<>
      QStandardItem * weaver::append<std::string> (const std::string & str) const
      {
        return append (QString (str.c_str()));
      }

      template<>
      QStandardItem * weaver::append<boost::format> (const boost::format & f) const
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
        if (prio)
          {
            append (boost::format ("priority: %i") % *prio);
          }
      }
      WSIG (transition::internal, MAYBE(bool), internal)
      {
        if (internal)
          {
            append (boost::format ("internal: %s") % boolString (*internal));
          }
      }
      WSIG (transition::_inline, MAYBE(bool), _inline)
      {
        if (_inline)
          {
            append (boost::format ("inline: %s") % boolString (*_inline));
          }
      }
      WSIG(transition::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }
      WSIG(transition::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (*this, structs);
      }
      WSIG(transition::function, XMLTYPE(transition_type::f_type), fun)
      {
        boost::apply_visitor
          (FROM(visitor::function_type<weaver>) (*this), fun);
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
        FROM(conditions) (*this, cond);
      }

      WSIG(place::open, ITVAL(XMLTYPE(places_type)), place)
      {
        push (append ("<<place>>"));
      }
      WSIGE(place::close) { pop(); }
      WSIG(place::name, std::string, name)
      {
        _stack.top()->setText (name.c_str());
      }
      WSIG(place::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(place::is_virtual, MAYBE(bool), is_virtual)
      {
        if (is_virtual)
          {
            append (boost::format ("virtual: %s") % boolString (*is_virtual));
          }
      }
      WSIG(place::capacity, MAYBE(petri_net::capacity_t), capacity)
      {
        if (capacity)
          {
            append (boost::format ("capacity: %i") % *capacity);
          }
      }
      WSIG(place::token, ITVAL(XMLTYPE(tokens_type)), token)
      {
        push (append ("token"));

        boost::apply_visitor (FROM(visitor::token<weaver>) (*this), token);

        pop ();
      }
      WSIG(place::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }

      WSIG(property::open, ITVAL(WETYPE(property::map_type)), prop)
      {
        boost::apply_visitor
          (FROM(visitor::property<weaver>) (*this, prop.first), prop.second);
      }

      WSIG(properties::open, WETYPE(property::type), props)
      {
        xs ("property", props.get_map(), FROM(property));
      }

      WSIG(sig_structured::open, ITVAL(::signature::structured_t), sig)
      {
        boost::apply_visitor
          (FROM(visitor::signature<weaver>) (sig.first, *this), sig.second);
      }

      WSIG(_struct::open, ITVAL(XMLTYPE(structs_type)), s)
      {
        boost::apply_visitor
          (FROM(visitor::signature<weaver>) (s.name, *this), s.sig);
      }

      WSIG(port::open, ITVAL(XMLTYPE(ports_type)), port)
      {
        push (append ("<<port>>"));
      }
      WSIGE(port::close) { pop(); }
      WSIG(port::name, std::string, name)
      {
        _stack.top()->setText (name.c_str());
      }
      WSIG(port::type, std::string, type)
      {
        add_type (type);
      }
      WSIG(port::place, MAYBE(std::string), place)
      {
        if (place)
          {
            append (boost::format("place: %s") % *place);
          }
      }
      WSIG(port::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }

      WSIG(cinclude::open, ITVAL(XMLTYPE(cincludes_type)), cinclude)
      {
        append (cinclude);
      }

      WSIG(link::open, ITVAL(XMLTYPE(links_type)), link)
      {
        append (link);
      }

      WSIG(structs::open, XMLTYPE(structs_type), structs)
      {
        xs ("struct", structs, FROM(_struct));
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
        append (boost::format("%s => %s") % tm.first % tm.second);
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
        _stack.top()->setText (name.c_str());
      }
      WSIG(specialize::use, std::string, use)
      {
        _stack.top()->setText ( _stack.top()->text()
                        . append(" use ")
                        . append(use.c_str())
                        );
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
            _stack.top()->setText ((*name).c_str());
          }
      }
      WSIG(function::internal, MAYBE(bool), internal)
      {
        if (internal)
          {
            append (boost::format ("internal: %s") % boolString (*internal));
          }
      }
      WSIG(function::require, XMLTYPE(requirements_type), reqs)
      {
        xs ("require", reqs, FROM(require));
      }
      WSIG(function::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }
      WSIG(function::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (*this, structs);
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
        boost::apply_visitor (FROM(visitor::net_type<weaver>) (*this), fun);
      }
      WSIG(function::conditions, XMLTYPE(conditions_type), cs)
      {
        FROM(conditions) (*this, cs);
      }

      WSIG(place_map::open, ITVAL(XMLTYPE(place_maps_type)), pm)
      {
        push (append ("<<place_map>>"));
      }
      WSIGE(place_map::close) { pop(); }
      WSIG(place_map::place_virtual, std::string, name)
      {
        _stack.top()->setText ( QString ("virtual: ")
                        . append(name.c_str())
                        );
      }
      WSIG(place_map::place_real, std::string, name)
      {
        _stack.top()->setText ( _stack.top()->text()
                        . append (" <-> real: ")
                        . append (name.c_str())
                        );
      }
      WSIG(place_map::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }

      WSIG(connection::open, ITVAL(XMLTYPE(connections_type)), connection)
      {
        push (append ("<<connection>>"));
      }
      WSIGE(connection::close) { pop(); }
      WSIG(connection::port, std::string, port)
      {
        _stack.top()->setText ( QString ("port: ")
                        . append (port.c_str())
                        );
      }
      WSIG(connection::place, std::string, place)
      {
        _stack.top()->setText ( _stack.top()->text()
                        . append (" -> ")
                        . append (place.c_str())
                        );
      }

      WSIG(requirement::open, ITVAL(XMLTYPE(requirements_type)), req)
      {
        push (append ("requirement"));
      }
      WSIGE(requirement::close) { pop(); }
      WSIG(requirement::key, std::string, key)
      {
        _stack.top()->setText ( _stack.top()->text()
                        . append(key.c_str())
                        );
      }
      WSIG(requirement::value, bool, val)
      {
        _stack.top()->setText ( _stack.top()->text()
                        . append (": ")
                        . append (boolString (val).c_str())
                        );
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
        _stack.top()->setText ( _stack.top()->text()
                        . append (" -> ")
                        . append (fun.c_str())
                        );
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
        append (field.first);

        boost::apply_visitor
          (FROM(visitor::token<weaver>) (*this), field.second);
      }

      WSIG(net::open, XMLTYPE(net_type), net)
      {
        push (append ("net"));
      }
      WSIGE(net::close) { pop(); }
      WSIG(net::properties, WETYPE(property::type), prop)
      {
        FROM(properties) (*this, prop);
      }
      WSIG(net::structs, XMLTYPE(structs_type), structs)
      {
        FROM(structs) (*this, structs);
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

      WSIG(signature::literal::name, std::string, name)
      {
        _stack.top()->setText (name.c_str());
      }
      WSIG(signature::literal::type, ::literal::type_name_t, type)
      {
        add_type (type);
      }
      WSIG( signature::structured::named_map
          , WNAME(signature::structured::named_map_type)
          , nm
          )
      {
        xs (nm.name(), nm.map(), FROM(structured));
      }

      WSIG( property::value::key_value
          , WNAME(property::value::key_value_type)
          , kv
          )
      {
        append (boost::format("%s: %s") % kv.key() % kv.value());
      }

      WSIG( property::type::key_prop
          , WNAME(property::type::key_prop_type)
          , kp
          )
      {
        append (kp.key());

        FROM(properties) (*this, kp.property());
      }

      WSIG(context::open, XMLPARSE(state::type), state)
      {
        push (append ("context"));
      }
      WSIGE(context::close) { pop(); }
      WSIG(context::key_value, XMLPARSE(state::key_value_t), kv)
      {
        append (boost::format("%s => %s") % kv.key() % kv.value());
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

        setFrameShape (QFrame::StyledPanel);
        setFrameShadow (QFrame::Sunken);
        setDragDropMode (QAbstractItemView::NoDragDrop);
        //! \todo As soon as we can actually edit stuff in here, remove.
        setEditTriggers (QAbstractItemView::NoEditTriggers);

        header()->setVisible (false);
      }

      void StructureView::from_file (const QString & input_q)
      {
        const std::string input (input_q.toStdString());
        XMLPARSE(state::type) state;

        state.set_input (input);

        const XMLTYPE(function_type) fun (XMLPARSE(just_parse) (state, input));

        from (fun, state);
      }

      void StructureView::from ( const XMLTYPE(function_type) & fun
                               , const XMLPARSE(state::type) & state
                               )
      {
        FROM( function_state<tv::weaver>
              ( tv::weaver (_root)
              , WNAME(function_state_type) (fun, state)
              )
            );
      }

      void StructureView::from (const XMLTYPE(function_type) & fun)
      {
        FROM(function<tv::weaver>) (tv::weaver (_root), fun);
      }
    }
  }
}
